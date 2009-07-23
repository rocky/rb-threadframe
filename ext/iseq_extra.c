#include "vm_core_mini.h"  /* Pulls in ruby.h */
#include "iseq_extra.h"


/* 
   RubyVM::InstructionSequence#arity - arity of an instruction sequence
 */
VALUE
iseq_arity(rb_iseq_t *iseq)
{
    return (iseq) ? INT2FIX(get_iseq_arity(iseq)) : Qnil;
}

