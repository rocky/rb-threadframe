#include "vm_core_mini.h"  /* Pulls in ruby.h */

extern VALUE rb_cISeq;
extern int get_iseq_arity(rb_iseq_t *iseq);

/* 
   RubyVM::InstructionSequence#arity - arity of an instruction sequence
 */
VALUE
iseq_arity(VALUE iseqval)
{
    rb_iseq_t *iseq;
    GetISeqPtr(iseqval, iseq);
    return (iseq) ? INT2FIX(get_iseq_arity(iseq)) : Qnil;
}

/* 
   RubyVM::InstructionSequence#arity - arity of an instruction sequence
 */
VALUE
iseq_local_name(VALUE iseqval, VALUE val)
{
    rb_iseq_t *iseq;
    int i = FIX2INT(val);
    GetISeqPtr(iseqval, iseq);
    return (i < iseq->local_table_size) 
	? rb_str_new2(rb_id2name(iseq->local_table[i]))
	: Qnil;
}

#define ISEQ_INT_FIELD_METHOD(FIELD)		\
static VALUE					\
iseq_##FIELD(VALUE iseqval)			\
{						\
  rb_iseq_t *iseq;				\
  GetISeqPtr(iseqval, iseq);			\
  return INT2FIX(iseq->FIELD);			\
}

ISEQ_INT_FIELD_METHOD(arg_block) ;
ISEQ_INT_FIELD_METHOD(arg_opts) ;
ISEQ_INT_FIELD_METHOD(arg_post_len) ;
ISEQ_INT_FIELD_METHOD(arg_rest) ;
ISEQ_INT_FIELD_METHOD(arg_simple) ;
ISEQ_INT_FIELD_METHOD(argc) ;
ISEQ_INT_FIELD_METHOD(local_size) ;
ISEQ_INT_FIELD_METHOD(local_table_size) ;

#define RB_DEFINE_ISEQ_METHOD(FIELD, ARGC) \
    rb_define_method(rb_cISeq, #FIELD, iseq_##FIELD, ARGC);

void
Init_iseq_extra(void)
{
    RB_DEFINE_ISEQ_METHOD(arity, 0);
    RB_DEFINE_ISEQ_METHOD(arg_block, 0) ;
    RB_DEFINE_ISEQ_METHOD(arg_opts, 0) ;
    RB_DEFINE_ISEQ_METHOD(arg_post_len, 0) ;
    RB_DEFINE_ISEQ_METHOD(arg_rest, 0) ;
    RB_DEFINE_ISEQ_METHOD(arg_simple, 0) ;
    RB_DEFINE_ISEQ_METHOD(argc, 0) ;
    RB_DEFINE_ISEQ_METHOD(local_name, 1) ;
    RB_DEFINE_ISEQ_METHOD(local_size, 0) ;
    RB_DEFINE_ISEQ_METHOD(local_table_size, 0) ;
}

