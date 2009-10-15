#if 0  /* The following is to fake out rdoc, until I find a better fix. */
/* 
 *  Additions to the RubyVM::InstructionSequence class
 */
VALUE rb_cIseq = rb_define_class_under(rb_cRubyVM, "InstructionSequence", ...)
#endif

#include "vm_core_mini.h"  /* Pulls in ruby.h */
#ifdef HAVE_COMPILE_OPTIONS
#endif
#include "iseq_mini.h"  /* Pulls in ruby.h */
#include "ruby19_externs.h"
#include <string.h>       /* For strlen() */

struct iseq_insn_info_entry {
    unsigned short position;
    unsigned short line_no;
    unsigned short sp;
};


/* 
 * Document-method: RubyVM::InstructionSequence::arity?
 *
 * call-seq:
 *     RubyVM::InstructionSequence#arity -> Fixnum
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
 * Document-method: RubyVM::InstructionSequence::brkpt_alloc
 *
 * call-seq:
 *     RubyVM::InstructionSequence#brkpt_alloc -> bool
 *
 *  Allocates a breakpoint byte vector of zeros for each 
 *  instruction in the instruction sequence. True is returned if 
 *  a vector was allocated, false if there already was one allocated,
 *  and nil if there was some problem.
 */
VALUE
iseq_brkpt_alloc(VALUE iseqval)
{
    rb_iseq_t *iseq;
    if (Qnil == iseqval) return Qnil;
    else {
	GetISeqPtr(iseqval, iseq);
	if (iseq->breakpoints) {
	    return Qfalse;
	}
	iseq->breakpoints = calloc( iseq->iseq_size, sizeof(unsigned char));
	return Qtrue;
    }
}

/* 
 * Document-method: RubyVM::InstructionSequence::brkpt_clear
 *
 * call-seq:
 *     RubyVM::InstructionSequence#brkpt_clear(offset) -> bool
 *
 *  Clears breakpoint of byte vector at +offset+.
 *  True is returned if there was a breakpoint previously set,
 *  false if not, and nil if there was some problem.
 */
VALUE
iseq_brkpt_clear(VALUE iseqval, VALUE offsetval)
{
    rb_iseq_t *iseq;
    if (Qnil != iseqval) {
	GetISeqPtr(iseqval, iseq);
	if (!iseq->breakpoints) 
	    return Qtrue;
	if (FIXNUM_P(offsetval)) {
	    int offset = FIX2INT(offsetval);
	    /* FIXME: check that offset is at a valid instruction offset */
	    if (offset >= 0 && offset < iseq->iseq_size) {
		iseq->breakpoints[offset] = '\000';
		return Qtrue;
	    } else
		rb_raise(rb_eTypeError, "given offset %d is not less than max size %lu", 
			 offset, iseq->iseq_size);
	} else {
	    rb_raise(rb_eTypeError, "type mismatch: %s given, int >= 0 expected", 
		     rb_class2name(CLASS_OF(offsetval)));
	}
    }
    return Qnil;
}

/* 
 * Document-method: RubyVM::InstructionSequence::brkpt_dealloc
 *
 * call-seq:
 *     RubyVM::InstructionSequence#dealloc -> bool
 *
 *  DeAllocates a breakpoint byte vector of zeros for each 
 *  instruction in the instruction sequence. True is returned if 
 *  a vector was allocated, false if there already was one allocated,
 *  and nil if there was some problem.
 */
VALUE
iseq_brkpt_dealloc(VALUE iseqval)
{
    rb_iseq_t *iseq;
    if (Qnil == iseqval) return Qnil;
    else {
	GetISeqPtr(iseqval, iseq);
	if (!iseq->breakpoints) {
	    return Qfalse;
	}
	free(iseq->breakpoints);
	iseq->breakpoints = NULL;
	return Qtrue;
    }
}

/* 
 * Document-method: RubyVM::InstructionSequence::brkpt_get(offset)
 *
 * call-seq:
 *     RubyVM::InstructionSequence#brkpt_get(offset) -> bool
 *
 *  Get a value of breakpoint of byte vector at +offset+.
 *  True is returned if there was a breakpoint previously set,
 *  false if not, and nil if there was some problem.
 */
VALUE
iseq_brkpt_get(VALUE iseqval, VALUE offsetval)
{
    rb_iseq_t *iseq;
    if (Qnil != iseqval) {
	if (FIXNUM_P(offsetval)) {
	    int offset = FIX2INT(offsetval);

	    GetISeqPtr(iseqval, iseq);
	    if (!iseq->breakpoints) {
		return Qfalse;
	    }
	    /* FIXME: check that offset is at a valid instruction offset */
	    if (offset >= 0 && offset < iseq->iseq_size) {
		return (0 != iseq->breakpoints[offset]) ? Qtrue : Qfalse;
	    } else
		rb_raise(rb_eTypeError, "given offset %d is not less than max size %lu", 
			 offset, iseq->iseq_size);
	} else {
	    rb_raise(rb_eTypeError, "type mismatch: %s given, int >= 0 expected", 
		     rb_class2name(CLASS_OF(offsetval)));
	}
    }
    return Qnil;
}

/* 
 * Document-method: RubyVM::InstructionSequence::brkpt_set
 *
 * call-seq:
 *     RubyVM::InstructionSequence#brkpt_set(offset) -> bool
 *
 *  Set a breakpoint of byte vector at +offset+.
 *  True is returned if there was a breakpoint previously set,
 *  false if not, and nil if there was some problem.
 */
VALUE
iseq_brkpt_set(VALUE iseqval, VALUE offsetval)
{
    rb_iseq_t *iseq;
    if (Qnil != iseqval) {
	GetISeqPtr(iseqval, iseq);
	if (!iseq->breakpoints) {
	    VALUE alloc_ret = iseq_brkpt_alloc(iseqval);
	    if (!iseq->breakpoints) return alloc_ret;
	}
	if (FIXNUM_P(offsetval)) {
	    int offset = FIX2INT(offsetval);
	    /* FIXME: check that offset is at a valid instruction offset */
	    if (offset >= 0 && offset < iseq->iseq_size) {
		iseq->breakpoints[offset] = '\001';
		return Qtrue;
	    } else
		rb_raise(rb_eTypeError, "given offset %d is not less than max size %lu", 
			 offset, iseq->iseq_size);
	} else {
	    rb_raise(rb_eTypeError, "type mismatch: %s given, int >= 0 expected", 
		     rb_class2name(CLASS_OF(offsetval)));
	}
    }
    return Qnil;
}

#ifdef HAVE_COMPILE_OPTIONS
/* 
 * Document-method: RubyVM::InstructionSequence::compile_options
 *
 * call-seq:
 *     RubyVM::InstructionSequence#compile_options -> Hash
 *
 *  Returns a hash of the compiler options used to create the 
 *  instruction sequence.
 */
VALUE
iseq_compile_options(VALUE iseqval)
{
    rb_iseq_t *iseq;
    if (Qnil == iseqval) return Qnil;
    else {
	VALUE hash_opts = rb_hash_new();
	rb_compile_option_t *compile_opts;
	GetISeqPtr(iseqval, iseq);
	if (!iseq->compile_data) return Qnil;
	compile_opts = iseq->compile_data->option;
	rb_hash_aset(hash_opts, rb_str_new2("inline_const_cache"), 
		     (compile_opts->inline_const_cache) ? Qtrue : Qfalse);
	return hash_opts;
    }
    
}
#endif

/* 
 * Document-method: RubyVM::InstructionSequence::equal?
 * 
 * call-seq:
 *     RubyVM::InstructionSequence#equal?(iseq2) -> bool
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
 *     RubyVM::InstructionSequence#iseq_size - Fixnum
 * 
 *  Returns the size of the instruction sequences
 */
VALUE
iseq_iseq_size(VALUE iseqval)
{
    rb_iseq_t *iseq;
    if (Qnil == iseqval) return Qnil;
    GetISeqPtr(iseqval, iseq);
    return INT2FIX(iseq->iseq_size);
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

/* FIXME: should return array of destroyed entries */
VALUE iseq_killcache(VALUE iseqval)
{
    rb_iseq_t *iseqdat;
    int *iseq ;
    unsigned long i, size, count = 0;
    struct iseq_insn_info_entry *table;

    GetISeqPtr(iseqval, iseqdat);
    iseq = (int *) iseqdat->iseq;
    size = iseqdat->insn_info_size;
    table = iseqdat->insn_info_table;
    for (i = 0; i < size; i++) {
      const unsigned long pos = table[i].position;
      const int insn = iseq[pos];
      if (insn == 52) /* getinlinecache */
      {
	/* printf("pos: %lu\n", pos); */
	count ++;
	iseq[pos] = 0;
	iseq[pos+1] = 0;
	iseq[pos+2] = 0;
      }
    }
    return INT2FIX(count);
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


/* RDoc can't find methods when we use a definition like this: */
#define RB_DEFINE_ISEQ_METHOD(FIELD, ARGC) \
    rb_define_method(rb_cISeq, #FIELD, iseq_##FIELD, ARGC);

void
Init_iseq_extra(void)
{
    rb_define_method(rb_cISeq, "arity",            iseq_arity, 0);
    rb_define_method(rb_cISeq, "arg_block",        iseq_arg_block, 0) ;
    rb_define_method(rb_cISeq, "arg_opts",         iseq_arg_opts, 0) ;
    rb_define_method(rb_cISeq, "arg_post_len",     iseq_arg_post_len, 0) ;
    rb_define_method(rb_cISeq, "arg_rest",         iseq_arg_rest, 0) ;
    rb_define_method(rb_cISeq, "arg_simple",       iseq_arg_simple, 0) ;
    rb_define_method(rb_cISeq, "argc",             iseq_argc, 0) ;
#ifdef HAVE_COMPILE_OPTIONS
    rb_define_method(rb_cISeq, "compile_options",  iseq_compile_options, 0) ;
#endif
    rb_define_method(rb_cISeq, "brkpt_alloc",      iseq_brkpt_alloc, 0) ;
    rb_define_method(rb_cISeq, "brkpt_clear",      iseq_brkpt_clear, 1) ;
    rb_define_method(rb_cISeq, "brkpt_dealloc",    iseq_brkpt_dealloc, 0) ;
    rb_define_method(rb_cISeq, "brkpt_get",        iseq_brkpt_get, 1) ;
    rb_define_method(rb_cISeq, "brkpt_set",        iseq_brkpt_set, 1) ;
    rb_define_method(rb_cISeq, "equal?",           iseq_equal, 1) ;
    rb_define_method(rb_cISeq, "iseq_size",        iseq_iseq_size, 0) ;
    rb_define_method(rb_cISeq, "killcache",        iseq_killcache, 0) ;
    rb_define_method(rb_cISeq, "klass",            iseq_klass, 0) ;
    rb_define_method(rb_cISeq, "local_name",       iseq_local_name, 1) ;
    rb_define_method(rb_cISeq, "local_size",       iseq_local_size, 0) ;
    rb_define_method(rb_cISeq, "local_table_size", iseq_local_table_size, 0) ;
    rb_define_method(rb_cISeq, "offset2lines",     iseq_offset2lines, 1) ;
    rb_define_method(rb_cISeq, "offsetlines",      iseq_offsetlines, 0) ;
    rb_define_method(rb_cISeq, "orig",             iseq_orig, 0) ;
    rb_define_method(rb_cISeq, "self",             iseq_self, 0) ;
    rb_define_method(rb_cISeq, "source_container", iseq_source_container, 0) ;
}
