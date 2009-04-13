/* 
   Access to Ruby's rb_control_frame_t and methods for working with that.
   Things like getting a binding for a control frame.
 */
#include <string.h>
#include "thread_extra.h"  /* Pulls in ruby.h */

typedef struct 
{
    rb_thread_t *th;
    rb_control_frame_t *cfp;
} thread_frame_t;
  

#define RUBY_VM_PREVIOUS_CONTROL_FRAME(cfp) (cfp+1)

extern rb_control_frame_t * thread_context_frame(void *);
extern rb_thread_t *ruby_current_thread;

VALUE rb_cThreadFrame;

static VALUE
thread_frame_alloc(VALUE klass)
{
    return Data_Wrap_Struct(klass, NULL, xfree, 0);
}

static thread_frame_t *
thread_frame_t_alloc(VALUE tfval)
{
    thread_frame_t *tf = ALLOC(thread_frame_t);
    memset(tf, 0, sizeof(thread_frame_t));
    DATA_PTR(tfval) = tf;
    return tf;
}

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
 *     tf.binding   => binding
 *
 *  Returns a binding for a given thread frame.
 *
 *     ThreadFrame::current.binding   #=> #<Binding:0x0000000223a648>
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

/*
 *  call-seq:
 *     tf.flag           => Fixnum
 *     tf.method_class   => Module
 *     tf.method_class   => false | proc ?
 *
 *  Returns an binding for a given thread frame.
 *
 *     ThreadFrame::current.flag          => 72
 *     ThreadFrame::current.method_class  => Comparable (?)
 *     ThreadFrame::current.proc          => false
 *     ThreadFrame::current.self          => 'main'
 */

THREAD_FRAME_FIELD_METHOD(flag) ;
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

/*
 *  call-seq:
 *     tf.prev           => ThreadFrame
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
    if (prev_cfp->iseq) {
        VALUE prev = rb_thread_frame_new();
        thread_frame_t *tf_prev;
        Data_Get_Struct(prev, thread_frame_t, tf_prev);
        tf_prev->cfp = prev_cfp;
        tf_prev->th = tf->th;
        return prev;
    } else return Qnil;
}

static VALUE
thread_frame_prev1(VALUE klass, VALUE thval)
{
    rb_control_frame_t *prev_cfp;
    rb_thread_t *th;
    GetThreadPtr(thval, th);
    prev_cfp = RUBY_VM_PREVIOUS_CONTROL_FRAME(th->cfp);
    if (prev_cfp->iseq) {
        VALUE prev = rb_thread_frame_new();
        thread_frame_t *tf_prev;
        Data_Get_Struct(prev, thread_frame_t, tf_prev);
        tf_prev->cfp = prev_cfp;
        tf_prev->th = th;
        return prev;
    } else return Qnil;
}

#define RB_DEFINE_FIELD_METHOD(FIELD) \
    rb_define_method(rb_cThreadFrame, #FIELD, thread_frame_##FIELD, 0);

void
Init_thread_frame(void)
{
  rb_cThreadFrame = rb_define_class("ThreadFrame", rb_cObject);
  rb_define_alloc_func(rb_cThreadFrame, thread_frame_alloc);
  rb_define_method(rb_cThreadFrame, "initialize", thread_frame_init, 0);
  rb_define_singleton_method(rb_cThreadFrame, "current", thread_frame_s_current,
			     0);
  rb_define_singleton_method(rb_cThreadFrame, "prev1", thread_frame_prev1, 1);
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

  rb_define_singleton_method(rb_cThread, "ni", 
			     thread_extra_ni, 0);

}

