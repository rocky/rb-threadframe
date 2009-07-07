#include "thread_extra.h"  /* Pulls in ruby.h */
#include "ruby19_externs.h"

/* 
   Proc#iseq - access instruction sequence of a Proc 
 */
VALUE
proc_iseq(VALUE self)
{
    rb_proc_t *proc;
    rb_iseq_t *iseq;
    VALUE rb_iseq;
    GetProcPtr(self, proc);
    iseq = proc->block.iseq;
    if (!iseq) return Qnil;
    rb_iseq = iseq_alloc_shared(rb_cISeq);
    RDATA(rb_iseq)->data = iseq;
    return rb_iseq;
}

