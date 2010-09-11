/* 
 * Copyright (C) 2010 Rocky Bernstein
 */
#if 0  /* The following is to fake out rdoc, until I find a better fix. */
/* 
 *  Additions to the RubyVM::Proc classes
 */
VALUE rb_cProc = rb_define_class("Proc", ...)
#endif

#include "../include/vm_core_mini.h"   /* Pulls in ruby.h and node.h */
#include "../include/ruby19_externs.h"

struct METHOD {
    VALUE recv;
    VALUE rclass;
    ID id;
    rb_method_entry_t me;
};

/* 
 *  call-seq:
 *  Proc#iseq -> RubyVM::InstructionSequence
 *
 *  Returns the instruction sequence for a Proc object.
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

/* Defined in Ruby 1.9 proc.c */
extern rb_iseq_t *rb_method_get_iseq(VALUE method);

#if 0  /* The following is to fake out rdoc, until I find a better fix. */
/* 
 *  Additions to the RubyVM::Method class
 */
VALUE rb_cIseq = rb_define_class("Method", ...)
#endif
/* 
 *  call-seq:
 *  Method#iseq -> RubyVM::InstructionSequence
 * 
 *  Returns the instruction sequence of a Method object.
 */
VALUE
method_iseq(VALUE self)
{
    VALUE rb_iseq;
    rb_iseq_t *iseq = rb_method_get_iseq(self);
    if (!iseq) return Qnil;
    rb_iseq = iseq_alloc_shared(rb_cISeq);
    RDATA(rb_iseq)->data = iseq;
    return rb_iseq;
}

/* 
 *  call-seq:
 *  Method#alias_count -> Fixnum
 *
 *  number of aliases a method has
 */
VALUE
method_alias_count(VALUE self)
{
  struct METHOD *m1 = (struct METHOD *)DATA_PTR(self);
  return INT2FIX(m1->me.def->alias_count);
}

/* 
 *  call-seq:
 *  Method#original_id - Original name of method
 */
VALUE
method_original_id(VALUE self)
{
  struct METHOD *m1 = (struct METHOD *)DATA_PTR(self);
  return ID2SYM(m1->me.def->original_id);
}


void
Init_proc_extra(void)
{
    /* Additions to Proc */
    rb_define_method(rb_cProc,   "iseq",  proc_iseq, 0);

    /* Additions to Method */
    rb_define_method(rb_cMethod, "iseq",  method_iseq, 0);
    rb_define_method(rb_cMethod, "alias_count",  method_alias_count, 0);
    rb_define_method(rb_cMethod, "original_id",  method_original_id, 0);
}
