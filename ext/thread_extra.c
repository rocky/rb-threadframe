/* 
 * Copyright (C) 2010, 2012 Rocky Bernstein
 */
#if 0  /* The following is to fake out rdoc, until I find a better fix. */
/* 
 *  Additions to the Thread class
 */
VALUE rb_cThread = rb_define_class("Method", ...)
#endif

#include "../include/vm_core_mini.h"   /* Pulls in ruby.h and node.h */
#include "thread_extra.h"

/* 
 *  call-seq:
 *  Thread#exec_event_tracing -> bool
 *
 *  Returns the value of the thread event-hook tracing.
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
 *  Thread#exec_event_tracing=(new_value)
 * 
 *  Sets the value of thread event-hook tracing.
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
 *  Thread#tracing -> bool
 * 
 *  Returns the value of the thread event-hook tracing.
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
 *  Thread#tracing= bool
 * 
 *  Sets the value of thread event-hook tracing.
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
