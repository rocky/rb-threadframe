#include "vm_core_mini.h"  /* Pulls in ruby.h */
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
    if (!iseq 
#if 0
	/* Our iseq struct isn't complete enough to contain self. */
	|| !RUBY_VM_NORMAL_ISEQ_P(iseq->self)
#endif
	)
	return Qnil;
    rb_iseq = iseq_alloc_shared(rb_cISeq);
    RDATA(rb_iseq)->data = iseq;
    return rb_iseq;
}

void
Init_proc_extra(void)
{
    rb_define_method(rb_cProc, "iseq",  proc_iseq, 0);
}
