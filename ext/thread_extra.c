#if 0  /* The following is to fake out rdoc, until I find a better fix. */
/* 
 *  Additions to the RubyVM::Method class
 */
VALUE rb_cThread = rb_define_class("Method", ...)
#endif

/* 
 *  Additions to Thread class.
 */

#include "vm_core_mini.h"  /* Pulls in ruby.h */
#include "thread_extra.h"

/* 
 *  call-seq:
 *  Thread#tracing - value of the thread event-hook tracing.
 */
VALUE
thread_exec_event_tracing(VALUE self)
{
    rb_thread_t *th;
    GetThreadPtr(self, th);
    return th->exec_event_tracing ? Qtrue : Qfalse;
}

/* 
 *  call-seq:
 *  Thread#tracing_set - set_value of the thread event-hook tracing.
 */
VALUE
thread_exec_event_tracing_set(VALUE self, VALUE new_value)
{
    rb_thread_t *th;
    GetThreadPtr(self, th);

    th->exec_event_tracing = RTEST(new_value) ? Qtrue : Qfalse;
    return th->exec_event_tracing;
}

/* 
 *  call-seq:
 *  Thread#tracing - value of the thread event-hook tracing.
 */
VALUE
thread_tracing(VALUE self)
{
    rb_thread_t *th;
    GetThreadPtr(self, th);
    return th->tracing ? Qtrue : Qfalse;
}

/* 
 *  call-seq:
 *  Thread#tracing_set - set_value of the thread event-hook tracing.
 */
VALUE
thread_tracing_set(VALUE self, VALUE new_value)
{
    rb_thread_t *th;
    GetThreadPtr(self, th);

    th->tracing = RTEST(new_value) ? Qtrue : Qfalse;
    return th->tracing;
}

void
Init_thread_extra(void)
{
    /* Additions to Thread class */
    rb_define_method(rb_cThread, "exec_event_tracing=",  
		     thread_exec_event_tracing_set, 1);
    rb_define_method(rb_cThread, "exec_event_tracing",   
		     thread_exec_event_tracing, 0);
    rb_define_method(rb_cThread, "tracing=",  thread_tracing_set, 1);
    rb_define_method(rb_cThread, "tracing",   thread_tracing, 0);
}
