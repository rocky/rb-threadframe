#include "vm_core_mini.h"  /* Pulls in ruby.h */
#include "ruby19_externs.h"
#include <string.h>       /* For strlen() */

struct iseq_insn_info_entry {
    unsigned short position;
    unsigned short line_no;
    unsigned short sp;
};

/* 
 * call-seq:
 *     RubyVM::InstructionSequence#arity => Fixnum
 *  sequence or nil is we can't get one.
 *
 *  Returns the number of arguments that would not be ignored.
 *  See Ruby 1.9 proc_arity of proc.c
 */
VALUE
iseq_arity(VALUE iseqval)
{
    rb_iseq_t *iseq;
    if (Qnil == iseqval) return Qnil;
    GetISeqPtr(iseqval, iseq);
    return (iseq) ? INT2FIX(get_iseq_arity(iseq)) : Qnil;
}

/* 
 * call-seq:
 *     RubyVM::InstructionSequence#equal?(iseq2) => bool
 *
 *  Returns true if the instruction sequences are equal.
 */
VALUE 
iseq_equal(VALUE iseqval1, VALUE iseqval2)
{
    rb_iseq_t *iseq1, *iseq2;

    if (Qnil == iseqval2) return Qfalse;
    if (!rb_obj_is_kind_of(iseqval2, rb_cISeq)) {
	rb_raise(rb_eTypeError, 
		 "comparison argument must be an instance of %s or nil (is %s)",
		 rb_obj_classname(iseqval1), rb_obj_classname(iseqval2));
    }
    
    if (iseqval1 == iseqval2) return Qtrue;
    GetISeqPtr(iseqval1, iseq1);
    GetISeqPtr(iseqval2, iseq2);

    /* FIXME: the count 24 below  is bogus. I think this should be the fields
       from "type" to  "mark_ary". 
     */
    if (0 == memcmp(iseq1, iseq2, 28))
	return Qtrue;
    else
	return Qfalse;
}


/* 
 * call-seq:
 *     RubyVM::InstructionSequence#local_name(i) - String
 * 
 *  Returns the string name of local variable in i'th position
 *  of the instruction sequence local table, or nil if i is
 * out of range.
 */
VALUE
iseq_local_name(VALUE iseqval, VALUE val)
{
    rb_iseq_t *iseq;
    if (FIXNUM_P(val)) {
      int i = FIX2INT(val);
      GetISeqPtr(iseqval, iseq);
      return (i < iseq->local_table_size && i >= 0) 
	? rb_str_new2(rb_id2name(iseq->local_table[i]))
	: Qnil;
    } else {
      rb_raise(rb_eTypeError, "type mismatch: %s given, int >= 0 expected", 
	       rb_class2name(CLASS_OF(val)));
    }
}

/* 
 * call-seq:
 *     RubyVM::InstructionSequence#offsetlines -> Hash[Fixnum] -> [Fixnum]
 * 
 * Returns an hash. The keys in the hash form the VM offsets of the
 * instructions.  The value of the hash for a given offset is a list
 * of line numbers associated with that offset.
 */
VALUE iseq_offsetlines(VALUE iseqval)
{
    rb_iseq_t *iseq;
    VALUE offsetlines = rb_hash_new();
    unsigned long i, size;
    struct iseq_insn_info_entry *table;
   
    GetISeqPtr(iseqval, iseq);
    
    size = iseq->insn_info_size;
    table = iseq->insn_info_table;

    for (i = 0; i < size; i++) {
	VALUE ary = rb_ary_new2(1);
	rb_ary_push(ary, INT2FIX(table[i].line_no));
	rb_hash_aset(offsetlines, INT2FIX(table[i].position), ary);
    }
    return offsetlines;
}

/* 
 * call-seq:
 *     RubyVM::InstructionSequence#offset2lines(offset) -> [Fixnum]
 * 
 * Returns an Array or nil. If offset is found then return the list of
 * lines associated with that offset. If the offset isn't found return nil.
 */
VALUE iseq_offset2lines(VALUE iseqval, VALUE offsetval)
{
    rb_iseq_t *iseq;
   
    GetISeqPtr(iseqval, iseq);
    
    if (FIXNUM_P(offsetval)) {
	unsigned long i, size;
	int offset = FIX2INT(offsetval);
	struct iseq_insn_info_entry *table;

	size = iseq->insn_info_size;
	table = iseq->insn_info_table;

	for (i = 0; i < size; i++) {
	    if (table[i].position == offset) {
		VALUE ary = rb_ary_new2(1);
		rb_ary_push(ary, INT2FIX(table[i].line_no));
		return ary;
	    }
	}
    }
    return Qnil;
}


VALUE
iseq_source_container_internal(rb_iseq_t *iseq)
{
    VALUE fileval = iseq->filename;
    const char *file = RSTRING_PTR(fileval);
    const char *contain_type;
    size_t len = strlen(file);
    
    /* FIXME: Looking for (...) is a hack that I would love to know how
       to remove. Probably Ruby has to be changed to record this kind
       of information.
     */
    if (len > 0 && file[0] == '(' && file[len-1] == ')')
	contain_type = "string";
    else
	contain_type = "file";
    return rb_ary_new3(2, rb_str_new2(contain_type), fileval);
}

/*
 * call-seq:
 *    RubyVM::InstructionSequence#source_container() -> [Type, String]
 *
 * Returns a tuple representing kind of container, e.g. file
 * eval'd string object, and the name of the container. If file,
 * it would be a file name. If an eval'd string it might be the string.
 */
static VALUE
iseq_source_container(VALUE iseqval)
{
    rb_iseq_t *iseq;
   
    if (Qnil == iseqval) return Qnil;
    GetISeqPtr(iseqval, iseq);
    return iseq_source_container_internal(iseq);
}



#define ISEQ_FIELD_METHOD(FIELD)		\
static VALUE					\
iseq_##FIELD(VALUE iseqval)			\
{						\
  rb_iseq_t *iseq;				\
  GetISeqPtr(iseqval, iseq);			\
  return iseq->FIELD;				\
}

ISEQ_FIELD_METHOD(self) ;
ISEQ_FIELD_METHOD(orig) ;

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
ISEQ_INT_FIELD_METHOD(klass) ;
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
    RB_DEFINE_ISEQ_METHOD(klass, 0) ;
    RB_DEFINE_ISEQ_METHOD(local_name, 1) ;
    RB_DEFINE_ISEQ_METHOD(local_size, 0) ;
    RB_DEFINE_ISEQ_METHOD(local_table_size, 0) ;
    RB_DEFINE_ISEQ_METHOD(offset2lines, 1) ;
    RB_DEFINE_ISEQ_METHOD(offsetlines, 0) ;
    RB_DEFINE_ISEQ_METHOD(orig, 0) ;
    RB_DEFINE_ISEQ_METHOD(self, 0) ;
    RB_DEFINE_ISEQ_METHOD(source_container, 0) ;

    rb_define_method(rb_cISeq, "equal?", iseq_equal, 1) ;

}

