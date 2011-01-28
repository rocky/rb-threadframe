/* 
 * Copyright (C) 2010, 2011 Rocky Bernstein
 *
 *  Access to Ruby's rb_control_frame_t and methods for working with that.
 *  Things like getting a binding for a control frame.
 */

/* What release we got? */
#define THREADFRAME_VERSION "0.37.dev"

#include <string.h>
#include "../include/vm_core_mini.h"   /* Pulls in ruby.h and node.h */
#include "proc_extra.h"
#include "iseq_extra.h"
#include "thread_extra.h"

/* for GC debug */

#ifndef RUBY_MARK_FREE_DEBUG
#define RUBY_MARK_FREE_DEBUG 0
#endif

#if RUBY_MARK_FREE_DEBUG
extern int ruby_gc_debug_indent;

static void
rb_gc_debug_indent(void)
{
    printf("%*s", ruby_gc_debug_indent, "");
}

static void
rb_gc_debug_body(const char *mode, const char *msg, int st, void *ptr)
{
    if (st == 0) {
	ruby_gc_debug_indent--;
    }
    rb_gc_debug_indent();
    printf("%s: %s %s (%p)\n", mode, st ? "->" : "<-", msg, ptr);

    if (st) {
	ruby_gc_debug_indent++;
    }

    fflush(stdout);
}

#define RUBY_MARK_ENTER(msg) rb_gc_debug_body("mark", msg, 1, ptr)
#define RUBY_MARK_LEAVE(msg) rb_gc_debug_body("mark", msg, 0, ptr)
#define RUBY_FREE_ENTER(msg) rb_gc_debug_body("free", msg, 1, ptr)
#define RUBY_FREE_LEAVE(msg) rb_gc_debug_body("free", msg, 0, ptr)
#define RUBY_GC_INFO         rb_gc_debug_indent(); printf

#else
#define RUBY_MARK_ENTER(msg)
#define RUBY_MARK_LEAVE(msg)
#define RUBY_FREE_ENTER(msg)
#define RUBY_FREE_LEAVE(msg)
#define RUBY_GC_INFO if(0)printf
#endif

/* Frames can't be detached from the control frame they live in.
   So we create a structure to contain the pair. 

   The signature fields are used to weakly verify the validity of cfp.
   it stores to contents of fields in cfp on allocation.  This, the
   validity of "th" pointing to a valid thread, and cfp pointing to valid
   location inside the frame area we use to check that this structure
   is valid. */

typedef struct 
{
    rb_thread_t *th;
    rb_control_frame_t *cfp;
    VALUE *signature1[3]; /* iseq, flag, self */
    VALUE *signature2[1]; /* proc */
} thread_frame_t;

#include "../include/ruby19_externs.h"

VALUE rb_cThreadFrame;       /* ThreadFrame class */
VALUE rb_eThreadFrameError;  /* Error raised on invalid frames. */

/* Static forward declarations */
static VALUE thread_frame_iseq(VALUE klass);
static VALUE thread_frame_prev_internal(rb_control_frame_t *prev_cfp, 
					rb_thread_t *th, int n);
static int   thread_frame_stack_size_internal(rb_control_frame_t *cfp, 
					      rb_thread_t *th);
static VALUE thread_frame_type(VALUE klass);


extern void iseq_mark(void *ptr); /* in iseq.c */

/* 
  FIXME: I've never seen the following routine get called.
  Why? 
 */
static void
thread_frame_mark(void *ptr)
{
    RUBY_MARK_ENTER("thread_frame");
    if (ptr) {
	thread_frame_t *tf = ptr;
	if (tf && tf->cfp && RUBY_VM_NORMAL_ISEQ_P(tf->cfp->iseq)) {
	    iseq_mark(tf->cfp->iseq);
	}
    }
}

/* Just to have a handle  on the free routine... */
static inline void
tf_free(void *ptr) 
{
    xfree(ptr);
}


/* 
   Allocate a RubyVM::ThreadFrame used by new. Less common than
   thread_frame_t_alloc(). The caller is responsible for filling in
   the C struct data. Below we wrap NULL.
 */
static VALUE
thread_frame_alloc(VALUE klass)
{
    return Data_Wrap_Struct(klass, thread_frame_mark, tf_free, NULL);
}

/* 
   Allocate a RubyVM::ThreadFrame and set its threadframe structure.
   This is the more common allocate routine since one normally doesn't
   create a threadframe without <i>first</i> having something to put in it.
 */
static thread_frame_t *
thread_frame_t_alloc(VALUE tfval)
{
    thread_frame_t *tf = ALLOC(thread_frame_t);
    memset(tf, 0, sizeof(thread_frame_t));
    DATA_PTR(tfval) = tf;
    return tf;
}

/* 
   Check to see if tf is valid. +true+ is returned if we can't prove
   the frame is invalide. +nil+ or +false+ is returned if something is not
   right. In those cases where we don't know that we have a valid frame,
   we also NULL out the cfp if that hasn't been done already. This will
   keep garbage collection from marking bad data.
 */
static VALUE
thread_frame_invalid_internal(thread_frame_t *tf)
{
    int cmp;

    /* All valid frame types have 0x1 set so we will use this.
       Warning: this is an undocumented assumption which may someday
       be wrong. */
    if (!tf->cfp) return Qtrue;
    if ((tf->cfp->flag & 0x1) == 0) {
	tf->cfp = NULL;
	return Qtrue;
    }

    if (RUBY_VM_CONTROL_FRAME_STACK_OVERFLOW_P(tf->th, tf->cfp)) {
	tf->cfp = NULL;
	return Qtrue;
    }
    if (RUBY_VM_NORMAL_ISEQ_P(tf->cfp->iseq)) {
	cmp = (0 == memcmp(tf->signature1, &(tf->cfp->iseq), 
			   sizeof(tf->signature1)) &&
	       0 == memcmp(tf->signature2, &(tf->cfp->proc), 
			   sizeof(tf->signature2)));
	if (cmp) return Qfalse;
	tf->cfp = NULL;
	return Qtrue;
    } else {
	/* FIXME: figure out what to do here. Probably more work is
	 * needed in thread_frame_prev_internal.
	 */
	return Qnil;
    }
}

/* 
   COPY_SIGNATURE saves some invariant data from the frame for
   comparison later when the frame is used again.

   Even though fields like iseq may not be valid for things C function
   frames, nevertheless all we care about is whether they could change
   or not over the course of evaluation. Hving more data to compare
   against to verify whether a frame is valid is helpful. If the data
   is random unitialized data, that's even better. Again just so long as
   that random data doesn't change in the course of normal use.

   FIXME: There are probably more fields which could be saved.  */
#define COPY_SIGNATURE(tf, cfp)					  \
    memcpy(tf->signature1, &(cfp->iseq), sizeof(tf->signature1)); \
    memcpy(tf->signature2, &(cfp->proc), sizeof(tf->signature2)) 
    
#define SAVE_FRAME(TF, TH)						\
    tf->th = TH;							\
    tf->cfp = thread_control_frame(tf->th);				\
    COPY_SIGNATURE(tf, tf->cfp);					\

#define GET_THREAD_PTR \
    rb_thread_t *th; \
    GetThreadPtr(thval, th)


/*
 *  call-seq:
 *     RubyVM::ThreadFrame#threadframe  -> thread_frame_object
 * 
 *  Returns a RubyVM::ThreadFrame object for the given thread.
 */
static VALUE
thread_frame_threadframe(VALUE thval)
{
    thread_frame_t *tf = ALLOC(thread_frame_t);
    GET_THREAD_PTR;
    memset(tf, 0, sizeof(thread_frame_t));
    SAVE_FRAME(tf, th) ;
    return Data_Wrap_Struct(rb_cThreadFrame, thread_frame_mark, tf_free, tf);
}

#define THREAD_FRAME_SETUP \
    thread_frame_t *tf; \
    Data_Get_Struct(klass, thread_frame_t, tf)

#define THREAD_FRAME_SETUP_WITH_ERROR		    \
    THREAD_FRAME_SETUP;				    \
    if (Qtrue == thread_frame_invalid_internal(tf)) \
	rb_raise(rb_eThreadFrameError, "invalid frame")

#define THREAD_FRAME_FIELD_METHOD(FIELD)	\
static VALUE					\
thread_frame_##FIELD(VALUE klass)		\
{						\
    THREAD_FRAME_SETUP ;			\
    return tf->cfp->FIELD;			\
}

#define THREAD_FRAME_FP_METHOD(REG)				\
VALUE						                \
thread_frame_##REG(VALUE klass, VALUE index)			\
{								\
    if (!FIXNUM_P(index)) {					\
	rb_raise(rb_eTypeError, "integer argument expected");	\
    } else {							\
        long int i = FIX2INT(index);				\
	THREAD_FRAME_SETUP_WITH_ERROR ;					\
	/* FIXME: check index is within range. */		\
	return tf->cfp->REG[-i]; /* stack  grows "down" */	\
    }								\
}

#if 0
/*
 *  call-seq:
 *     RubyVM::ThreadFrame#dfp(n)  -> object
 * 
 * Returns a RubyVM object stored at dfp position <i>i</i>. The top object
 * is position 0. 
 */
static VALUE 
thread_frame_dfp(VALUE klass, VALUE index) 
{
    /* handled by THREAD_FRAME_FP_METHOD macro;  */
}
#endif
/* The above declaration is to make RDOC happy. */
THREAD_FRAME_FP_METHOD(dfp)

/*
 *  call-seq:
 *     RubyVM::ThreadFrame#lfp(i)  -> object
 * 
 * Returns a RubyVM object stored at lfp position <i>i</i>. The top object
 * is position 0. Negative values of <i>i</i> count from the end.
 */
static VALUE 
thread_frame_lfp(VALUE klass, VALUE index) 
{
  if (!FIXNUM_P(index)) {
    rb_raise(rb_eTypeError, "integer argument expected");
  } else {
    long int i = FIX2INT(index);
    long int size;
    
    THREAD_FRAME_SETUP_WITH_ERROR ;

    size = tf->cfp->iseq->local_size;
    if (i < 0) i = size - i;
    
    if (i > size)
      rb_raise(rb_eIndexError, 
	       "local frame index %ld should be in the range %ld .. %ld",
	       i, -size, size-1);

    return tf->cfp->lfp[-i]; /* stack  grows "down" */
  }
}

#if 0
/*
 *  call-seq:
 *     RubyVM::ThreadFrame#sp(n)  -> object
 * 
 * Returns a RubyVM object stored at stack position <i>i</i>. The top object
 * is position 0. 1 is the next object.
 */
VALUE 
thread_frame_sp(VALUE klass, VALUE index) 
{
    /* handled by THREAD_FRAME_FP_METHOD macro;  */
}
#endif
/* The above declaration is to make RDOC happy. 
   FIXME: Figure out a way to check if "index" is valid!
*/
THREAD_FRAME_FP_METHOD(sp)

static long int
thread_frame_sp_size_internal(thread_frame_t *tf) 
{
    rb_control_frame_t *prev_cfp;
    long int ret_val;
    prev_cfp = RUBY_VM_PREVIOUS_CONTROL_FRAME(tf->cfp);
    if (RUBY_VM_CONTROL_FRAME_STACK_OVERFLOW_P(tf->th, prev_cfp))
	return Qnil;
    ret_val = tf->cfp->sp - prev_cfp->sp - 1;
    /* FIXME: Why For C Functions we tack on 3? Possibly we're doing
       the previous frame and possibly that's where the action is that
       we should be looking at.? */
    if (RUBYVM_CFUNC_FRAME_P(tf->cfp)) ret_val += 3;
    return ret_val;
}

/*
 *  call-seq:
 *     RubyVM::ThreadFrame#sp_size  -> FixNum
 * 
 * Returns the number of stack or sp entries in the current
 * frame. That is the, number values that have been pushed onto the
 * stack since the current call.  This is different than
 * RubyVM::ThreadFrame#stack_size which counts the number of frames in
 * the call stack. +nil+ is returned if there is an error.
 */
VALUE 
thread_frame_sp_size(VALUE klass) 
{
    THREAD_FRAME_SETUP_WITH_ERROR ;
    return INT2FIX(thread_frame_sp_size_internal(tf));
}

/*
 *  call-seq:
 *     RubyVM::ThreadFrame#sp_set(n, newvalue)  -> object
 * 
 * Sets VM stack position <i>n</i> to <i>newvalue</i>. The top object
 * is position 0. 1 is the next object.
 */
static VALUE 
thread_frame_sp_set(VALUE klass, VALUE index, VALUE newvalue)
{
    if (!FIXNUM_P(index)) {
	rb_raise(rb_eTypeError, "integer argument expected");
    } else {
        long int i = FIX2INT(index);
	THREAD_FRAME_SETUP_WITH_ERROR ;
	if (i <= thread_frame_sp_size_internal(tf)) {
	    /* stack  grows "down" */
	    tf->cfp->sp[-i] = newvalue;
	} else {
	    rb_raise(rb_eArgError, "argument too big");
	}
	return newvalue;
    }
}

#ifndef NO_reg_pc
/*
 *  call-seq:
 *     RubyVM::ThreadFrame#pc_offset=
 * 
 * Sets pc to the offset given. 
 * WARNING, this is pretty dangerous. You need to set this to a valid
 * instruction offset since little checking is done.
 */
static VALUE
thread_frame_set_pc_offset(VALUE klass, VALUE offset_val)
{
    int offset;
    THREAD_FRAME_SETUP_WITH_ERROR ;

    if (!FIXNUM_P(offset_val)) {
	rb_raise(rb_eTypeError, "integer argument expected");
    } else {
        offset = FIX2INT(offset_val);
	if (RUBY_VM_NORMAL_ISEQ_P(tf->cfp->iseq) && 
	    (tf->cfp->pc != 0 && tf->cfp->iseq != 0)) {
            tf->cfp->pc = tf->cfp->iseq->iseq_encoded + offset;
	}
    }
    return Qtrue;
}
#endif

#if 0
/*
 *  call-seq:
 *     RubyVM::ThreadFrame#flag -> Fixnum
 *
 *  Returns the frame flags, a FIXNUM which should be interpreted as a
 *  bitmask.
 *
 */
static VALUE thread_frame_flag(VALUE klass) 
{ 
    /* handled by THREAD_FRAME_FIELD_METHOD macro;  */
}
/* The above declaration is to make RDOC happy. */
#endif
THREAD_FRAME_FIELD_METHOD(flag) ;

/*
 *  call-seq:
 *     RubyVM::ThreadFrame#argc -> Fixnum
 *
 *  Returns the number of arguments that were actually passed 
 *  in the call to this frame. This differs from arity when
 *  arity can take optional or "splat"ted parameters.
 *
 */
static VALUE
thread_frame_argc(VALUE klass)
{
    THREAD_FRAME_SETUP_WITH_ERROR;

    if (RUBYVM_CFUNC_FRAME_P(tf->cfp)) {
	return INT2FIX(tf->cfp->me->def->body.cfunc.actual_argc);
    } else if (RUBY_VM_NORMAL_ISEQ_P(tf->cfp->iseq)) {
	return iseq_argc(thread_frame_iseq(klass));
    } else
	return Qnil;
}

/*
 *  call-seq:
 *     RubyVM::ThreadFrame#arity -> Fixnum
 *
 *  Returns the number of arguments that would not be ignored.
 *  See Ruby 1.9 proc_arity of proc.c
 *
 */
static VALUE
thread_frame_arity(VALUE klass)
{
    THREAD_FRAME_SETUP_WITH_ERROR ;

    if (RUBY_VM_NORMAL_ISEQ_P(tf->cfp->iseq)) {
	return rb_iseq_arity(thread_frame_iseq(klass));
    } else if (RUBYVM_CFUNC_FRAME_P(tf->cfp)) {
	return INT2FIX(tf->cfp->me->def->body.cfunc.argc);
    } else
	return Qnil;
}

/*
 *  call-seq:
 *     RubyVM::ThreadFrame#binding   -> binding
 *
 *  Returns a binding for a given thread frame.
 */
static VALUE
thread_frame_binding(VALUE klass)
{
    THREAD_FRAME_SETUP_WITH_ERROR ;

    {
	rb_binding_t *bind = 0;
	VALUE bindval = rb_binding_frame_new(tf->th, tf->cfp);
	GetBindingPtr(bindval, bind);
	bind->line_no = rb_vm_get_sourceline(tf->cfp);
	if (tf->cfp->iseq) {
	    bind->filename = tf->cfp->iseq->filename;
	} else {
	    bind->filename = tf->th->vm->progname;
	}
	return bindval;
    }    
    /* NOTREACHED */
    return Qnil;
}

/*
 *  call-seq:
 *     RubyVM::ThreadFrame#equal(tf)   -> bool
 *
 *  Returns true if two thread frames are equal.
 */
static VALUE
thread_frame_equal(VALUE klass, VALUE tfval2)
{
    THREAD_FRAME_SETUP_WITH_ERROR ;

    {
	thread_frame_t *tf2;
	if (!rb_obj_is_kind_of(tfval2, rb_cThreadFrame)) {
	rb_raise(rb_eTypeError, 
		 "comparison argument must be an instance of %s (is %s)",
		 rb_obj_classname(klass), rb_obj_classname(tfval2));
	}
	Data_Get_Struct(tfval2, thread_frame_t, tf2);
	if (Qtrue == thread_frame_invalid_internal(tf2))
	    rb_raise(rb_eThreadFrameError, "invalid frame");

	/* And just when you thought I'd never get around to the
	   actual comparison... 

	   Comparing cfp's should be enough, but we'll throw in the thread
	   for good measure.
	*/
	return (tf->th == tf2->th && tf->cfp == tf2->cfp) 
	    ? Qtrue : Qfalse;
    }
    /* NOTREACHED */
    return Qnil;
}

/*
 *  call-seq:
 *     RubyVM::ThreadFrame.new(thread)          -> thread_frame_object
 *
 *  Returns an RubyVM::ThreadFrame object which can contains dynamic frame
 *  information. Don't use this directly. Instead use one of the 
 *  class methods.
 *
 *    RubyVM::ThreadFrame::VERSION               => 0.1 
 *    RubyVM::ThreadFrame::current.flag          => 72
 *    RubyVM::ThreadFrame::current.proc          => false
 *    RubyVM::ThreadFrame::current.self          => 'main'
 */
static VALUE
thread_frame_initialize(VALUE tfval, VALUE thval)
{
    thread_frame_t *tf = thread_frame_t_alloc(tfval);
    GET_THREAD_PTR ;
    memset(tf, 0, sizeof(thread_frame_t));
    DATA_PTR(tfval) = tf;
    SAVE_FRAME(tf, th) ;
    return tfval;
}

/*
 * call-seq:
 *    RubyVM::ThreadFrame#invalid? -> Boolean
 *
 * Returns true if the frame is no longer valid. On the other hand,
 * since the test we use is weak, returning false might not mean the
 * frame is valid, just that we can't disprove that it is not invalid.
 * 
 * It is suggested that frames are used in a way that ensures they will
 * be valid. In particular frames should have local scope and frames to 
 * threads other than the running one should be stopped while the frame 
 * variable is active.
 */
static VALUE
thread_frame_invalid(VALUE klass)
{
    THREAD_FRAME_SETUP ;
    return thread_frame_invalid_internal(tf);
}

static VALUE
thread_frame_is_return_stop(VALUE klass)
{
    THREAD_FRAME_SETUP ;
    return (tf->cfp->tracing & VM_FRAME_TRACE_RETURN) ? Qtrue : Qfalse;
}

static VALUE
thread_frame_is_trace_off(VALUE klass)
{
    THREAD_FRAME_SETUP ;
    return (tf->cfp->tracing & VM_FRAME_TRACE_OFF) ? Qtrue : Qfalse;
}

/*
 *  call-seq:
 *     ThreadFrame#method  -> String or nil
 * 
 * Returns the method associated with the frame or nil of none.
 * ThreadFrameError can be raised if the threadframe
 * object is no longer valid.
 */
static VALUE
thread_frame_method(VALUE klass)
{
    THREAD_FRAME_SETUP_WITH_ERROR ;			\

    switch (VM_FRAME_TYPE(tf->cfp)) {
      case VM_FRAME_MAGIC_BLOCK:
      case VM_FRAME_MAGIC_EVAL:
      case VM_FRAME_MAGIC_LAMBDA:
      case VM_FRAME_MAGIC_METHOD:
      case VM_FRAME_MAGIC_TOP:
	if (RUBY_VM_NORMAL_ISEQ_P(tf->cfp->iseq)) 
	    return tf->cfp->iseq->name;
	else
	    return Qnil; /* unknown */
    case VM_FRAME_MAGIC_CFUNC: {
          const char *method_name = rb_id2name(tf->cfp->me->def->original_id);
	  return rb_str_new2((NULL == method_name) ? "" : method_name );
        }
      default:
	/* FIXME */
	return thread_frame_type(klass);
    }
    /* NOTREACHED */
    return Qnil;
}

/*
 *  call-seq:
 *     ThreadFrame#pc_offset  -> Fixnum
 * 
 * Returns the offset inside the iseq or "program-counter offset" or -1
 * If invalid/unstarted. ThreadFrameError can be raised if the threadframe
 * object is no longer valid.
 */
static VALUE
thread_frame_pc_offset(VALUE klass)
{
    unsigned long pc;
    THREAD_FRAME_SETUP_WITH_ERROR ;

    if (RUBY_VM_NORMAL_ISEQ_P(tf->cfp->iseq) && 
	(tf->cfp->pc != 0 && tf->cfp->iseq != 0)) {
	pc = tf->cfp->pc - tf->cfp->iseq->iseq_encoded;
	return INT2FIX(pc);
    } else {
        return INT2FIX(-1);
    }
}


/*
 *  call-seq:
 *     ThreadFrame#iseq           -> ISeq
 *
 *  Returns an instruction sequence object from the instruction sequence
 *  found inside the ThreadFrame object or nil if there is none.
 *
 */
static VALUE
thread_frame_iseq(VALUE klass)
{
    rb_iseq_t *iseq;
    VALUE rb_iseq;
    THREAD_FRAME_SETUP_WITH_ERROR ;
    iseq = tf->cfp->iseq;
    if (!iseq) return Qnil;
    rb_iseq = iseq_alloc_shared(rb_cISeq);
    RDATA(rb_iseq)->data = iseq;
    return rb_iseq;
}

/* 
   See the above thread_frame_prev comment for what's going on here.
*/
static VALUE
thread_frame_next(VALUE klass)
{
    rb_control_frame_t *cfp = NULL;
    THREAD_FRAME_SETUP_WITH_ERROR ;
    cfp = RUBY_VM_NEXT_CONTROL_FRAME(tf->cfp);

    if ((void *)(cfp) <= (void *)(tf->th->stack))
        return Qnil;
    else {
        thread_frame_t *next_tf;
        VALUE next = thread_frame_alloc(rb_cThreadFrame);
	thread_frame_t_alloc(next);
	Data_Get_Struct(next, thread_frame_t, next_tf);
	next_tf->th  = tf->th;
	next_tf->cfp = cfp;
	COPY_SIGNATURE(tf, cfp);
	return next;
    }
}

/*
 *  call-seq:
 *     RubyVM::ThreadFrame#prev(n=1) -> thread_frame_object
 *
 *  Returns a RubyVM::ThreadFrame object for the frame prior to the
 *  ThreadFrame object or +nil+ if there is none. Setting n=0 just
 *  returns the object passed.  A negative starts from the end. So
 *  prev(-1) is the top frame.  Counts outside of the range
 *  -stack_size .. stack_size-1 exceed the the range of the stack and
 *  return nil.
 *
 */
VALUE
thread_frame_prev(int argc, VALUE *argv, VALUE klass)
{
    VALUE nv;
    int n;

    THREAD_FRAME_SETUP_WITH_ERROR ;

    rb_scan_args(argc, argv, "01", &nv);

    if (Qnil == nv)
	n = 1;
    else if (!FIXNUM_P(nv)) {
	rb_raise(rb_eTypeError, "integer argument expected");
    } else
	n = FIX2INT(nv);
    
    if (n < 0) {
      int stack_size = thread_frame_stack_size_internal(tf->cfp, tf->th);
      if (-n > stack_size) return Qnil;
      n = stack_size + n;
    }
    if (n == 0) return klass;
    return thread_frame_prev_internal(tf->cfp, tf->th, n);
}

/* 
   See the above thread_frame_prev comment for what's going on here.
*/
static VALUE
thread_frame_prev_internal(rb_control_frame_t *prev_cfp, rb_thread_t *th, 
			   int n)
{
  thread_frame_t *tf;
  VALUE prev;
  rb_control_frame_t *cfp = NULL;

  for (; n > 0; n--) {
    cfp = prev_cfp;
    prev_cfp = RUBY_VM_PREVIOUS_CONTROL_FRAME(cfp);
    if (VM_FRAME_TYPE(prev_cfp) == VM_FRAME_MAGIC_FINISH) {
	prev_cfp = RUBY_VM_PREVIOUS_CONTROL_FRAME(prev_cfp);
    }
    if (RUBY_VM_CONTROL_FRAME_STACK_OVERFLOW_P(th, prev_cfp))
	return Qnil;
  }
  if (!cfp) return Qnil;

  prev = thread_frame_alloc(rb_cThreadFrame);
  thread_frame_t_alloc(prev);
  Data_Get_Struct(prev, thread_frame_t, tf);
  tf->th  = th;
  tf->cfp = prev_cfp;
  COPY_SIGNATURE(tf, tf->cfp);
  return prev;
}

THREAD_FRAME_FIELD_METHOD(proc) ;
THREAD_FRAME_FIELD_METHOD(self) ;

static VALUE
thread_frame_return_stop_set(VALUE klass, VALUE boolval)
{
    short int boolmask = !(NIL_P(boolval) || Qfalse == boolval);
    THREAD_FRAME_SETUP ;
    
    if (boolmask)
	tf->cfp->tracing |=  VM_FRAME_TRACE_RETURN;
    else
	tf->cfp->tracing &= ~VM_FRAME_TRACE_RETURN;
    return boolval;
}

static VALUE
thread_frame_trace_off_set(VALUE klass, VALUE boolval)
{
    short int boolmask = !(NIL_P(boolval) || Qfalse == boolval);
    THREAD_FRAME_SETUP ;
    
    if (boolmask)
	tf->cfp->tracing |=  VM_FRAME_TRACE_OFF;
    else
	tf->cfp->tracing &= ~VM_FRAME_TRACE_OFF;
    return boolval;
}

/*
 *  call-seq:
 *     RubyVM::ThreadFrame::current  -> thread_frame_object
 * 
 *  Returns a ThreadFrame object for the currently executing thread.
 *  Same as: RubyVM::ThreadFrame.new(Thread::current)
 */
static VALUE
thread_frame_s_current(VALUE klass)
{
    thread_frame_t *tf = thread_frame_t_alloc(klass);
    SAVE_FRAME(tf, ruby_current_thread) ;
    return Data_Wrap_Struct(klass, thread_frame_mark, tf_free, tf);
}

/*
 *  call-seq:
 *     RubyVM::ThreadFrame::prev(thread)     -> threadframe_object
 *     RubyVM::ThreadFrame::prev(thread, n)   -> threadframe_object
 *
 *  Returns a RubyVM::ThreadFrame for the frame prior to the
 *  Thread object passed or nil if there is none. The default value for n
 *  is 1. 0 just returns the object passed.
 *  Negative counts or counts exceeding the stack will return nil.
 */
static VALUE
thread_frame_s_prev(int argc, VALUE *argv, VALUE klass)
{
    VALUE thval;
    VALUE nv;
    int n;
  
    rb_scan_args(argc, argv, "11", &thval, &nv);

    if (Qfalse == rb_obj_is_kind_of(thval, rb_cThread))
	rb_raise(rb_eTypeError, 
		 "ThreadFrame object needed for first argument");

    GET_THREAD_PTR ;
    
    if (Qnil == nv)
	n = 1;
    else if (!FIXNUM_P(nv)) {
	rb_raise(rb_eTypeError, "Fixnum needed for second argument");
    } else
	n = FIX2INT(nv);

    if (n == 0) { 
	return thread_frame_s_current(klass);
    } else if (n < 0) {
      int stack_size = thread_frame_stack_size_internal(th->cfp, th);
      if (-n > stack_size) return Qnil;
      n = stack_size + n;
    }

    return thread_frame_prev_internal(th->cfp, th, n);
}

/*
 * call-seq:
 *    RubyVM::ThreadFrame#source_container() -> [Type, String]
 *
 * Returns a tuple representing kind of container, e.g. file
 * eval'd string object, and the name of the container. If file,
 * it would be a file name. If an eval'd string it might be the string.
 */
static VALUE
thread_frame_source_container(VALUE klass)
{
    VALUE filename = Qnil;
    const char *contain_type;
    rb_control_frame_t *cfp;
    int is_eval = 0;

    THREAD_FRAME_SETUP ;

    for ( cfp = tf->cfp; cfp && !cfp->iseq && RUBYVM_CFUNC_FRAME_P(cfp); 
	  cfp = RUBY_VM_PREVIOUS_CONTROL_FRAME(cfp) ) ;


    if (cfp->iseq) 
	filename = cfp->iseq->filename;
    else {
	if (tf->th->vm->progname) 
	    filename = tf->th->vm->progname;
	else 
	    return Qnil;
    }
    
    contain_type = source_container_type(filename);

    is_eval = ( 0 == strcmp("string", contain_type)
		&& VM_FRAME_MAGIC_EVAL == VM_FRAME_TYPE(tf->cfp) );

    if ( is_eval ) {
	/* Try to pick up string from stack. */
	VALUE prev = thread_frame_prev_internal(tf->cfp, tf->th, 1);
	thread_frame_t *prev_tf;
	Data_Get_Struct(prev, thread_frame_t, prev_tf);
	
	if (RUBYVM_CFUNC_FRAME_P(prev_tf->cfp) && 
	    thread_frame_stack_size_internal(prev_tf->cfp, prev_tf->th) >= 3)
	    filename = thread_frame_sp(prev, INT2FIX(3));
    }

    return rb_ary_new3(2, rb_str_new2(contain_type), filename);
}

/*
 * call-seq:
 *    RubyVM::ThreadFrame#source_location() -> Array 
 *
 * Returns an array of source location positions that match
 * +tf.instruction_offset+. A source location position is left
 * implementation dependent. It could be line number, a line number
 * and start and end column, or a start line number, start column, end
 * line number, end column.
 */
static VALUE
thread_frame_source_location(VALUE klass)
{
    rb_control_frame_t *cfp;
    THREAD_FRAME_SETUP ;

    /* For now, it appears like we have line numbers only when there
       is an instruction sequence. The heuristic that is used by
       vm_backtrace_each of vm.c seems to be to use the line number of
       the closest control frame that has an instruction sequence.
       FIXME: investigate whether this is always the most accurate location. If
       not, improve.
    */
    for ( cfp = tf->cfp; cfp && !cfp->iseq && RUBYVM_CFUNC_FRAME_P(cfp); 
	  cfp = RUBY_VM_PREVIOUS_CONTROL_FRAME(cfp) ) ;
    
    return (cfp->iseq)
	/* NOTE: for now sourceline returns a single int. In the
	   future it might return an array of ints.
	*/
	? rb_ary_new3(1, INT2FIX(rb_vm_get_sourceline(cfp)))
	: Qnil;
}

/*
 *  call-seq:
 *     RubyVM::ThreadFrame#stack_size  -> Fixnum;
 *
 *  Returns a count of the number of frames including the current one. 
 *  ThreadFrame#prev(ThreadFrame#stack_size) = nil
 *  ThreadFrame#prev(ThreadFrame#stack_size-1) = top frame
 *  
 * 
 */
static VALUE
thread_frame_stack_size(VALUE klass)
{
    THREAD_FRAME_SETUP ;
    return INT2FIX(thread_frame_stack_size_internal(tf->cfp, tf->th));
}

/* 
   See the above thread_frame_stack_size comment for what's going on here.
*/
static int
thread_frame_stack_size_internal(rb_control_frame_t *cfp, rb_thread_t *th)
{
    int n;
    for ( n = 0; 
	  !RUBY_VM_CONTROL_FRAME_STACK_OVERFLOW_P(th, cfp);
	  cfp = RUBY_VM_PREVIOUS_CONTROL_FRAME(cfp)) {
	n++;
	if (VM_FRAME_TYPE(cfp) == VM_FRAME_MAGIC_FINISH) {
	    cfp = RUBY_VM_PREVIOUS_CONTROL_FRAME(cfp);
	    if (RUBY_VM_CONTROL_FRAME_STACK_OVERFLOW_P(th, cfp))
		break;
	}
    }
    return n;
}

/*
 *  call-seq:
 *     RubyVM::ThreadFrame#thread   -> thread
 *
 *  Returns the thread object for the thread frame.
 */
static VALUE
thread_frame_thread(VALUE klass)
{
    THREAD_FRAME_SETUP ;
    return tf->th->self;
}

/* Extracted from vm_dump.c. Would be nice to have this routine put there
   and used in both places. */
static const char *
frame_magic2str(rb_control_frame_t *cfp) 
{
    switch (VM_FRAME_TYPE(cfp)) {
      case VM_FRAME_MAGIC_TOP:
	return "TOP";
      case VM_FRAME_MAGIC_METHOD:
	return "METHOD";
      case VM_FRAME_MAGIC_CLASS:
	return "CLASS";
      case VM_FRAME_MAGIC_BLOCK:
	return "BLOCK";
      case VM_FRAME_MAGIC_FINISH:
	return "FINISH";
      case VM_FRAME_MAGIC_CFUNC:
	return "CFUNC";
      case VM_FRAME_MAGIC_PROC:
	return "PROC";
      case VM_FRAME_MAGIC_LAMBDA:
	return "LAMBDA";
      case VM_FRAME_MAGIC_IFUNC:
	return "IFUNC";
      case VM_FRAME_MAGIC_EVAL:
	return "EVAL";
      case 0:
	return "------";
      default:
	return "(none)";
    }
    /* NOTREACHED */
    return "?";
}

/*
 *  call-seq:
 *     ThreadFrame#type  -> String 
 * 
 * Returns the kind of frame. Basically interprets VM_FRAME_MAGIC for
 * tf->cfp->flag
 */
static VALUE
thread_frame_type(VALUE klass)
{
    THREAD_FRAME_SETUP ;			
    return rb_str_new2(frame_magic2str(tf->cfp));
}

void
Init_thread_frame(void)
{
    /* Additions to RubyVM */
    rb_cThreadFrame = rb_define_class_under(rb_cRubyVM, "ThreadFrame", 
					    rb_cObject);
    rb_define_method(rb_cThread, "threadframe", thread_frame_threadframe, 0);

    /* Thread::Frame */
    rb_define_const(rb_cThreadFrame, "VERSION", 
		    rb_str_new2(THREADFRAME_VERSION));
    rb_define_alloc_func(rb_cThreadFrame, thread_frame_alloc);

    rb_define_method(rb_cThreadFrame, "invalid?", thread_frame_invalid, 0);

    /* RubyVM::ThreadFrame */
    rb_define_method(rb_cThreadFrame, "argc", thread_frame_argc, 0);
    rb_define_method(rb_cThreadFrame, "arity", thread_frame_arity, 0);
    rb_define_method(rb_cThreadFrame, "binding", thread_frame_binding, 0);
    rb_define_method(rb_cThreadFrame, "dfp", thread_frame_dfp, 1);
    rb_define_method(rb_cThreadFrame, "flag", thread_frame_flag, 0);
    rb_define_method(rb_cThreadFrame, "initialize", thread_frame_initialize, 1);
    rb_define_method(rb_cThreadFrame, "iseq", thread_frame_iseq, 0);
    rb_define_method(rb_cThreadFrame, "lfp", thread_frame_lfp, 1);
    rb_define_method(rb_cThreadFrame, "method", thread_frame_method, 0);
    rb_define_method(rb_cThreadFrame, "next", thread_frame_next, 0);
    rb_define_method(rb_cThreadFrame, "pc_offset", thread_frame_pc_offset, 0);
    rb_define_method(rb_cThreadFrame, "prev", thread_frame_prev, -1);
    rb_define_method(rb_cThreadFrame, "proc", thread_frame_proc, 0);
    rb_define_method(rb_cThreadFrame, "return_stop=", thread_frame_return_stop_set, 1);
    rb_define_method(rb_cThreadFrame, "return_stop?", thread_frame_is_return_stop, 0);
    rb_define_method(rb_cThreadFrame, "self", thread_frame_self, 0);
    rb_define_method(rb_cThreadFrame, "source_container", 
		     thread_frame_source_container, 0);
    rb_define_method(rb_cThreadFrame, "source_location", 
		     thread_frame_source_location, 0);

    /* sp[] and sp[]= would be neater, but that would require making sp an
       object which I am not sure I want to do.
     */
    rb_define_method(rb_cThreadFrame, "sp", thread_frame_sp, 1);
    rb_define_method(rb_cThreadFrame, "sp_set", thread_frame_sp_set, 2);
    rb_define_method(rb_cThreadFrame, "sp_size", thread_frame_sp_size, 0);

    /* I think I like the more explicit stack_size over size or length. */
    rb_define_method(rb_cThreadFrame, "stack_size", 
		     thread_frame_stack_size, 0);

    rb_define_method(rb_cThreadFrame, "thread", thread_frame_thread, 0);
    rb_define_method(rb_cThreadFrame, "trace_off?", thread_frame_is_trace_off, 0);
    rb_define_method(rb_cThreadFrame, "trace_off=", thread_frame_trace_off_set, 1);
    rb_define_method(rb_cThreadFrame, "type", thread_frame_type, 0);

    rb_define_method(rb_cThreadFrame, "equal?", 
		     thread_frame_equal, 1);

#ifndef NO_reg_pc
    rb_define_method(rb_cThreadFrame, "pc_offset=", 
		     thread_frame_set_pc_offset, 1);
#endif


    rb_eThreadFrameError = rb_define_class("ThreadFrameError", 
					   rb_eStandardError);

    rb_define_singleton_method(rb_cThreadFrame, "prev", 
			       thread_frame_s_prev, -1);
    rb_define_singleton_method(rb_cThreadFrame, "current", 
			       thread_frame_s_current,   0);

    
    /* Perform the other C extension initializations. */
    Init_iseq_extra();
    Init_proc_extra();
    Init_thread_extra();
    Init_thread_extra();
}
