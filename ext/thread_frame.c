/* 
 *  Access to Ruby's rb_control_frame_t and methods for working with that.
 *  Things like getting a binding for a control frame.
 */

/* What release we got? */
#define THREADFRAME_VERSION "0.2"  

#include <string.h>
#include "vm_core_mini.h"  /* Pulls in ruby.h */

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
    VALUE *signature2[3]; /* proc, method_id, method_class */
} thread_frame_t;

#include "ruby19_externs.h"

VALUE rb_cThreadFrame;       /* ThreadFrame class */
VALUE rb_eThreadFrameError;  /* Error raised on invalid frames. */

static VALUE thread_frame_invalid_internal(thread_frame_t *tf);

/* 
   Allocate a RubyVM::ThreadFrame used by new. Less common than
   thread_frame_t_alloc().
 */
static VALUE
thread_frame_alloc(VALUE klass)
{
    return Data_Wrap_Struct(klass, NULL, xfree, 0);
}

/* 
   Allocate a RubyVM::ThreadFrame and set its threadframe structure.
   This is the more common allocate routine since one normally doesn't
   create a threadframe without <i>first</i> having seomthing to put in it.
 */
static thread_frame_t *
thread_frame_t_alloc(VALUE tfval)
{
    thread_frame_t *tf = ALLOC(thread_frame_t);
    memset(tf, 0, sizeof(thread_frame_t));
    DATA_PTR(tfval) = tf;
    return tf;
}

#define SAVE_FRAME(TF, TH)						\
    tf->th = TH;							\
    tf->cfp = thread_context_frame(tf->th);				\
    memcpy(tf->signature1, &(tf->cfp->iseq), sizeof(tf->signature1));	\
    memcpy(tf->signature2, &(tf->cfp->proc), sizeof(tf->signature2)) 

/*
 *  call-seq:
 *     RubyVM::ThreadFrame.new(thread)          => thread_frame_object
 *
 *  Returns an RubyVM::ThreadFrame object which can contains dynamic frame
 *  information. Don't use this directly. Instead use one of the 
 *  class methods.
 *
 *     ThreadFrame::VERSION               => 0.1 
 *     ThreadFrame::current.flag          => 72
 *     ThreadFrame::current.method_class  => Comparable (?)
 *     ThreadFrame::current.proc          => false
 *     ThreadFrame::current.self          => 'main'
 */
static VALUE
thread_frame_init(VALUE tfval, VALUE thread)
{
    thread_frame_t *tf = thread_frame_t_alloc(tfval);
    rb_thread_t *th;

    GetThreadPtr(thread, th);
    memset(tf, 0, sizeof(thread_frame_t));
    DATA_PTR(tfval) = tf;
    SAVE_FRAME(tf, th) ;
    return tfval;
}

/*
 *  call-seq:
 *     Thread#threadframe  => thread_frame_object
 * 
 *  Returns a RubyVM::ThreadFrame object for the given thread.
 */
static VALUE
thread_frame_threadframe(VALUE thval)
{
    thread_frame_t *tf = ALLOC(thread_frame_t);
    rb_thread_t *th;
    memset(tf, 0, sizeof(thread_frame_t));
    GetThreadPtr(thval, th);
    SAVE_FRAME(tf, th) ;
    return Data_Wrap_Struct(rb_cThreadFrame, NULL, xfree, tf);
}

/*
 *  call-seq:
 *     Thread#stack  => ? 
 * 
 */
static VALUE
thread_stack(VALUE thval)
{
    rb_thread_t *th;
    GetThreadPtr(thval, th);
    return *(th->stack);
}

/*
 *  call-seq:
 *     RubyVM::ThreadFrame::current  => thread_frame_object
 * 
 *  Returns a ThreadFrame object for the currently executing thread.
 *  Same as: RubyVM::ThreadFrame.new(Thread::current)
 */
static VALUE
thread_frame_s_current(VALUE klass)
{
    thread_frame_t *tf = thread_frame_t_alloc(klass);
    SAVE_FRAME(tf, ruby_current_thread) ;
    return Data_Wrap_Struct(klass, NULL, xfree, tf);
}

/*
 *  call-seq:
 *     RubyVM::ThreadFrame#binding   => binding
 *
 *  Returns a binding for a given thread frame.
 */
static VALUE
thread_frame_binding(VALUE klass)
{
    thread_frame_t *tf;
    Data_Get_Struct(klass, thread_frame_t, tf);
    if (Qtrue == thread_frame_invalid_internal(tf))
	rb_raise(rb_eThreadFrameError, "invalid frame");
    else
	return rb_binding_frame_new(tf->th, tf->cfp);
    /* NOTREACHED */
    return Qnil;
}

/*
 *  call-seq:
 *     RubyVM::ThreadFrame#thread   => thread
 *
 *  Returns the thread object for the thread frame.
 */
static VALUE
thread_frame_thread(VALUE klass)
{
    thread_frame_t *tf;
    Data_Get_Struct(klass, thread_frame_t, tf);
    return tf->th->self;
}

#define THREAD_FRAME_FIELD_METHOD(FIELD)	\
static VALUE					\
thread_frame_##FIELD(VALUE klass)		\
{						\
    thread_frame_t *tf;				\
    Data_Get_Struct(klass, thread_frame_t, tf); \
    return tf->cfp->FIELD;			\
}

/*
 *  call-seq:
 *     Thread#pc_offset  => Fixnum
 * 
 * Returns the offset inside the iseq or "program-counter offset" or -1
 * If invalid/unstarted. ThreadFrameError can be raised if the threadframe
 * object is no longer valid.
 */
static VALUE
thread_frame_pc_offset(VALUE klass)
{
    thread_frame_t *tf;
    int pc = -1;
    Data_Get_Struct(klass, thread_frame_t, tf);
    if (Qtrue == thread_frame_invalid_internal(tf))
	rb_raise(rb_eThreadFrameError, "invalid frame");
    if (RUBY_VM_NORMAL_ISEQ_P(tf->cfp->iseq)) {
	pc = tf->cfp->pc - tf->cfp->iseq->iseq_encoded;
    }
    return INT2FIX(pc);
}


#ifdef NO_reg_pc
/*
 *  call-seq:
 *     Thread#pc_offset=
 * 
 * Sets pc to the offset given. 
 * WARNING, this is pretty dangerous. You need to set this to a valid
 # instruction offset since little checking is done.
 */
static VALUE
thread_frame_set_pc_offset(VALUE klass, VALUE offset_val)
{
    thread_frame_t *tf;
    int offset;
    Data_Get_Struct(klass, thread_frame_t, tf);
    if (Qtrue == thread_frame_invalid_internal(tf))
	rb_raise(rb_eThreadFrameError, "invalid frame");
    if (!FIXNUM_P(offset_val))
	return Qfalse;
    offset = FIX2INT(offset_val);
    if (RUBY_VM_NORMAL_ISEQ_P(tf->cfp->iseq)) {
	tf->cfp->pc = tf->cfp->iseq->iseq_encoded + offset;
    }
    return Qtrue;
}
#endif


THREAD_FRAME_FIELD_METHOD(flag)
THREAD_FRAME_FIELD_METHOD(method_class) ;
THREAD_FRAME_FIELD_METHOD(proc) ;
THREAD_FRAME_FIELD_METHOD(self) ;

#define THREAD_FRAME_PTR_FIELD_METHOD(FIELD)	\
static VALUE					\
thread_frame_##FIELD(VALUE klass)		\
{						\
    thread_frame_t *tf;				\
    Data_Get_Struct(klass, thread_frame_t, tf); \
    return *(tf->cfp->FIELD);			\
}

THREAD_FRAME_PTR_FIELD_METHOD(bp) ;
THREAD_FRAME_PTR_FIELD_METHOD(dfp) ;
THREAD_FRAME_PTR_FIELD_METHOD(lfp) ;
/*THREAD_FRAME_PTR_FIELD_METHOD(pc) ;*/
/*THREAD_FRAME_PTR_FIELD_METHOD(sp) ;*/

static VALUE
thread_frame_prev_common(rb_control_frame_t *prev_cfp, rb_thread_t *th)
{
    if (prev_cfp) {
        thread_frame_t *tf;
        VALUE prev = thread_frame_alloc(rb_cThreadFrame);
	thread_frame_t_alloc(prev);
        Data_Get_Struct(prev, thread_frame_t, tf);
        tf->th = th;
        tf->cfp = prev_cfp;
	memcpy(tf->signature1, &(tf->cfp->iseq), sizeof(tf->signature1)); 
	memcpy(tf->signature2, &(tf->cfp->proc), sizeof(tf->signature2));
        return prev;
    } else {
	return Qnil;
    }
}

/*
 *  call-seq:
 *     RubyVM::ThreadFrame#prev           => thread_frame_object
 *
 *  Returns a RubyVM::ThreadFrame object for the frame prior to the
 *  ThreadFrame object or nil if there is none.
 *
 */
static VALUE
thread_frame_prev(VALUE klass)
{
    rb_control_frame_t *prev_cfp;
    thread_frame_t *tf;
    Data_Get_Struct(klass, thread_frame_t, tf);
    prev_cfp = RUBY_VM_PREVIOUS_CONTROL_FRAME(tf->cfp);
    prev_cfp = rb_vm_get_ruby_level_next_cfp(tf->th, prev_cfp);
    return thread_frame_prev_common(prev_cfp, tf->th);
}

/*
 *  call-seq:
 *     RubyVM::ThreadFrame#prev(thread)     => threadframe_object
 *
 *  Returns a RubyVM::ThreadFrame for the frame prior to the
 *  Thread object passed or nil if there is none.
 *
 */
static VALUE
thread_frame_thread_prev(VALUE klass, VALUE thval)
{
    rb_control_frame_t *prev_cfp;
    rb_thread_t *th;
    GetThreadPtr(thval, th);
    prev_cfp = RUBY_VM_PREVIOUS_CONTROL_FRAME(th->cfp);
    return thread_frame_prev_common(prev_cfp, th);
}

/*
 *  call-seq:
 *     RubyVM::ThreadFrame#iseq           => ISeq
 *
 *  Returns an instruction sequence object from the instruction sequence
 *  found inside the ThreadFrame object or nil if there is none.
 *
 */
static VALUE
thread_frame_iseq(VALUE klass)
{
    thread_frame_t *tf;
    rb_iseq_t *iseq;
    VALUE rb_iseq;
    Data_Get_Struct(klass, thread_frame_t, tf);
    iseq = tf->cfp->iseq;
    if (!iseq) return Qnil;
    rb_iseq = iseq_alloc_shared(rb_cISeq);
    RDATA(rb_iseq)->data = iseq;
    return rb_iseq;
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
    thread_frame_t *tf;
    Data_Get_Struct(klass, thread_frame_t, tf);

    if (!tf->cfp->iseq) {
	/* FIXME: try harder... */
	return Qnil;
    } 

    /* FIXME: As Ruby 1.9 improves, so should this. */
    return rb_ary_new3(2, rb_str_new2("file"), tf->cfp->iseq->filename);
}

/*
 * call-seq:
 *    RubyVM::ThreadFrame#source_location() -> Array 
 *
 *  Returns an array of source location positions that match
 * +tf.instruction_offset+. A source location position is left
 * implementation dependent. It could be line number, a line number
 * and start and end column, or a start line number, start column, end
 * line number, end column.
 */
static VALUE
thread_frame_source_location(VALUE klass)
{
    thread_frame_t *tf;
    Data_Get_Struct(klass, thread_frame_t, tf);

    if (!tf->cfp->iseq) {
	/* FIXME: try harder... */
	return Qnil;
    } 
    /* NOTE: for now sourceline returns a single int. In the
       future it might return an array of ints.
     */

    return rb_ary_new3(1, INT2FIX(rb_vm_get_sourceline(tf->cfp)));
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
    thread_frame_t *tf;
    Data_Get_Struct(klass, thread_frame_t, tf);
    return thread_frame_invalid_internal(tf);
}

static VALUE
thread_frame_invalid_internal(thread_frame_t *tf)
{
    int cmp;
    if (RUBY_VM_CONTROL_FRAME_STACK_OVERFLOW_P(tf->th, tf->cfp))
	return Qtrue;
    cmp = (0 == memcmp(tf->signature1, &(tf->cfp->iseq), 
		       sizeof(tf->signature1)) &&
	   0 == memcmp(tf->signature2, &(tf->cfp->proc), 
		       sizeof(tf->signature2)));
    return cmp ? Qfalse : Qtrue;
}
    

#define RB_DEFINE_FIELD_METHOD(FIELD) \
    rb_define_method(rb_cThreadFrame, #FIELD, thread_frame_##FIELD, 0);

extern VALUE rb_cThread;

extern VALUE proc_iseq(VALUE self);

void
Init_thread_frame(void)
{
    /* Additions to RubyVM */
    rb_cThreadFrame = rb_define_class_under(rb_cRubyVM, "ThreadFrame", 
					    rb_cObject);
    rb_define_method(rb_cThread, "threadframe", thread_frame_threadframe, 0);
    rb_define_method(rb_cThread, "stack", thread_stack, 0);

    /* Thread:Frame */
    rb_define_const(rb_cThreadFrame, "VERSION", rb_str_new2(THREADFRAME_VERSION));
    rb_define_alloc_func(rb_cThreadFrame, thread_frame_alloc);

    rb_define_method(rb_cThreadFrame, "initialize", thread_frame_init, 1);
    rb_define_method(rb_cThreadFrame, "invalid?", thread_frame_invalid, 0);
    rb_define_method(rb_cThreadFrame, "iseq", thread_frame_iseq, 0);
#ifdef NO_reg_pc
    rb_define_method(rb_cThreadFrame, "pc_offset=", 
		     thread_frame_set_pc_offset, 1);
#endif
    rb_define_method(rb_cThreadFrame, "prev", thread_frame_prev, 1);
    rb_define_method(rb_cThreadFrame, "source_container", 
		     thread_frame_source_container, 0);
    rb_define_method(rb_cThreadFrame, "source_location", 
		     thread_frame_source_location, 0);
    rb_define_method(rb_cThreadFrame, "thread", thread_frame_thread, 0);

    rb_eThreadFrameError = rb_define_class("ThreadFrameError", 
					   rb_eStandardError);

    rb_define_singleton_method(rb_cThreadFrame, "prev", 
			       thread_frame_thread_prev, 1);
    rb_define_singleton_method(rb_cThreadFrame, "current", 
			       thread_frame_s_current,   0);

    RB_DEFINE_FIELD_METHOD(binding);
    RB_DEFINE_FIELD_METHOD(bp);
    RB_DEFINE_FIELD_METHOD(dfp);
    RB_DEFINE_FIELD_METHOD(flag);
    RB_DEFINE_FIELD_METHOD(lfp);
    RB_DEFINE_FIELD_METHOD(method_class);
    RB_DEFINE_FIELD_METHOD(pc_offset);
    RB_DEFINE_FIELD_METHOD(prev);
    RB_DEFINE_FIELD_METHOD(proc);
    RB_DEFINE_FIELD_METHOD(self);
    /*RB_DEFINE_FIELD_METHOD(sp);*/
    
    /* Just a test to pull in a second C source file. */
    rb_define_method(rb_cProc, "iseq", proc_iseq, 0);
}
