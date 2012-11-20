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
 *  Additions to the RubyVM::Method and RubyVM::UnboundMethod class
 */
VALUE rb_cIseq = rb_define_class("Method", ...)
VALUE rb_cIseq = rb_define_class("UnboundMethod", ...)
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

static void null_gc_proc(void *ptr) { }

static size_t null_gc_memsize(const void *ptr) { }


static const rb_data_type_t method_data_type = {
    "method",
    null_gc_proc,
    null_gc_proc,
    null_gc_memsize,
};

static inline rb_method_definition_t *
method_get_def(VALUE method)
{
    /* FIXME: use
       struct METHOD *data;
       TypedData_Get_Struct(method, struct METHOD, &method_data_type, data);
    */
    struct METHOD *data = (struct METHOD *)DATA_PTR(method);
    return data->me.def;
}

/* 
 *  call-seq:
 *  Method#type -> String
 * 
 *  Returns the Method object.
 */
VALUE
method_type(VALUE self)
{
    rb_method_definition_t *def = method_get_def(self);
    const char *type_str;
    switch (def->type) {
      case VM_METHOD_TYPE_ISEQ: 
	type_str = "instruction sequence";
	break;
      case VM_METHOD_TYPE_CFUNC: 
	type_str = "C function";
	break;
      case VM_METHOD_TYPE_ATTRSET:
	type_str = "attrset";
	break;
      case VM_METHOD_TYPE_IVAR:
	type_str = "ivar";
	break;
      case VM_METHOD_TYPE_BMETHOD:
	type_str = "bmethod";
	break;
      case VM_METHOD_TYPE_ZSUPER:
	type_str = "zsuper";
	break;
      case VM_METHOD_TYPE_UNDEF:
	type_str = "undefined";
	break;
      case VM_METHOD_TYPE_NOTIMPLEMENTED:
	type_str = "not implemented";
	break;
      case VM_METHOD_TYPE_OPTIMIZED: /* Kernel#send, Proc#call, etc */
	type_str = "optimized";
	break;
      case VM_METHOD_TYPE_MISSING: /* wrapper for method_missing(id) */
	type_str = "type missing";
	break;
      default:
	type_str = "unknown";
	break;
    }
    return rb_str_new2(type_str);
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
    rb_define_method(rb_cMethod, "alias_count",  method_alias_count, 0);
    rb_define_method(rb_cMethod, "iseq",         method_iseq, 0);
    rb_define_method(rb_cMethod, "original_id",  method_original_id, 0);
    rb_define_method(rb_cMethod, "type",         method_type, 0);

    rb_define_method(rb_cUnboundMethod, "alias_count",  method_alias_count, 0);
    rb_define_method(rb_cUnboundMethod, "original_id",  method_original_id, 0);
    rb_define_method(rb_cUnboundMethod, "type",         method_type, 0);
}
