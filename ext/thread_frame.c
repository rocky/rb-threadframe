/* 
 *  Access to Ruby's rb_control_frame_t and methods for working with that.
 *  Things like getting a binding for a control frame.
 */

/* What release we got? */
#define THREADFRAME_VERSION "0.2"  

#include <string.h>
#include "thread_extra.h"  /* Pulls in ruby.h */

/* Frames can't be detached from the control frame they live in.
   So we create a structure to contain the pair. */
typedef struct 
{
    rb_thread_t *th;
    rb_control_frame_t *cfp;
} thread_frame_t;
  
/* From vm_core.h: */
#define RUBY_VM_PREVIOUS_CONTROL_FRAME(cfp) (cfp+1)

extern rb_control_frame_t * thread_context_frame(void *);
extern rb_thread_t *ruby_current_thread;
extern VALUE rb_iseq_disasm_internal(rb_iseq_t *iseqdat);

VALUE rb_cThreadFrame;  /* ThreadFrame class */

/* 
   Allocate a Thread::Frame used by new. Less common than
   thread_frame_t_alloc().
 */
static VALUE
thread_frame_alloc(VALUE klass)
{
    return Data_Wrap_Struct(klass, NULL, xfree, 0);
}

/* 
   Allocate a Thread::Frame and set its threadframe structure.
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

/*
 *  call-seq:
 *     Thread::Frame.new()           => ThreadFrame
 *
 *  Returns an Thread::Frame object which can contains dynamic frame
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
thread_frame_init(VALUE tfval)
{
    thread_frame_t_alloc(tfval);
    return tfval;
}

VALUE
rb_thread_frame_new()
{
    return thread_frame_init(thread_frame_alloc(rb_cThreadFrame));
}

/*
 *  call-seq:
 *     Thread::Frame::current  => thread_frame_object
 * 
 *  Returns a Thread::Frame object for the currently executing thread.
 */
static VALUE
thread_frame_s_current(VALUE klass)
{
    thread_frame_t *tf = thread_frame_t_alloc(klass);
    tf->th = ruby_current_thread;
    tf->cfp = thread_context_frame(tf->th);
    return Data_Wrap_Struct(klass, NULL, xfree, tf);
}

/*
 *  call-seq:
 *     Thread::Frame#binding   => binding
 *
 *  Returns a binding for a given thread frame.
 */
static VALUE
thread_frame_binding(VALUE klass)
{
    thread_frame_t *tf;
    Data_Get_Struct(klass, thread_frame_t, tf);
    return rb_binding_frame_new(tf->th, tf->cfp);
}

#define THREAD_FRAME_FIELD_METHOD(FIELD)	\
static VALUE					\
thread_frame_##FIELD(VALUE klass)		\
{						\
    thread_frame_t *tf;				\
    Data_Get_Struct(klass, thread_frame_t, tf); \
    return tf->cfp->FIELD;			\
}

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

/*THREAD_FRAME_PTR_FIELD_METHOD(bp) ;*/
THREAD_FRAME_PTR_FIELD_METHOD(dfp) ;
THREAD_FRAME_PTR_FIELD_METHOD(lfp) ;
/*THREAD_FRAME_PTR_FIELD_METHOD(pc) ;*/
/*THREAD_FRAME_PTR_FIELD_METHOD(sp) ;*/

static VALUE
thread_frame_prev_common(rb_control_frame_t *prev_cfp, rb_thread_t *th)
{
    if (prev_cfp->iseq) {
        VALUE prev = rb_thread_frame_new();
        thread_frame_t *tf_prev;
        Data_Get_Struct(prev, thread_frame_t, tf_prev);
        tf_prev->cfp = prev_cfp;
        tf_prev->th = th;
        return prev;
    } else return Qnil;
}

/*
 *  call-seq:
 *     Thread::Frame#prev           => threadframe_object
 *
 *  Returns a ThreadFrame for the frame prior to the
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
    return thread_frame_prev_common(prev_cfp, tf->th);
}

/*
 *  call-seq:
 *     Thread::Frame#prev(thread)     => threadframe_object
 *
 *  Returns a Thread::Frame for the frame prior to the
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

extern VALUE iseq_alloc_shared(VALUE klass);
extern VALUE rb_cISeq;

/*
 *  call-seq:
 *     tf.iseq           => ISeq
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

#define RB_DEFINE_FIELD_METHOD(FIELD) \
    rb_define_method(rb_cThreadFrame, #FIELD, thread_frame_##FIELD, 0);

extern VALUE rb_cThread;

void
Init_thread_frame(void)
{
    rb_cThreadFrame = rb_define_class_under(rb_cThread, "Frame", rb_cObject);
    rb_define_const(rb_cThreadFrame, "VERSION", rb_str_new2(THREADFRAME_VERSION));
    rb_define_alloc_func(rb_cThreadFrame, thread_frame_alloc);
    rb_define_method(rb_cThreadFrame, "initialize", thread_frame_init, 0);
    rb_define_singleton_method(rb_cThreadFrame, "current", thread_frame_s_current,
			       0);
    rb_define_method(rb_cThreadFrame, "iseq", thread_frame_iseq, 0);
    rb_define_method(rb_cThreadFrame, "prev", thread_frame_prev, 1);
    rb_define_singleton_method(rb_cThreadFrame, "prev", 
			       thread_frame_thread_prev, 1);
    RB_DEFINE_FIELD_METHOD(binding);
    /*RB_DEFINE_FIELD_METHOD(bp);*/
    RB_DEFINE_FIELD_METHOD(dfp);
    RB_DEFINE_FIELD_METHOD(flag);
    RB_DEFINE_FIELD_METHOD(lfp);
    RB_DEFINE_FIELD_METHOD(method_class);
    /*RB_DEFINE_FIELD_METHOD(pc);*/
    RB_DEFINE_FIELD_METHOD(prev);
    RB_DEFINE_FIELD_METHOD(proc);
    RB_DEFINE_FIELD_METHOD(self);
    /*RB_DEFINE_FIELD_METHOD(sp);*/
    
    /* Just a test to pull in a second C source file. */
    rb_define_singleton_method(rb_cThread, "ni", 
			       thread_extra_ni, 0);

}
