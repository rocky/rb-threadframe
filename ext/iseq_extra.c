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
 *  Does checks that offsetval is in range and converts it to a
 *  positive integer. An exception is raised if offsetval is
 *  invalid.
 */
static long unsigned int
iseq_get_offset_internal(rb_iseq_t *iseq, VALUE offsetval)
{
    if (FIXNUM_P(offsetval)) {
	long int offset = FIX2INT(offsetval);
	unsigned long int uoffset;
	unsigned long int size = iseq->iseq_size;

	uoffset = (offset < 0) ? 
	    (unsigned long int) size + offset :
	    (unsigned long int) offset;
	
	/* FIXME: check that offset is at a valid instruction offset */
	if (uoffset >= size) {
	    rb_raise(rb_eTypeError, 
		     "instruction sequence offset %lu should be in the range -%lu .. %lu",
		     uoffset, size, size-1);
	}
	return uoffset;
    } else {
	rb_raise(rb_eTypeError, "type mismatch: %s given, Fixnum expected", 
		 rb_class2name(CLASS_OF(offsetval)));
	return 0; /* NOT REACHED */
    }
    
}

/* 
 * Document-method: RubyVM::InstructionSequence::brkpt_get(offset)
 *
 * call-seq:
 *     RubyVM::InstructionSequence#brkpt_get(offset) -> bool
 *
 *  Get a value of breakpoint of byte vector at +offset+.
 *
 *  True is returned if there is a breakpoint previously set, false
 *  if not, and nil if there was some problem. Negative values of
 *  <i>offset</i> count from the end of the instruction sequence.
 */
VALUE
iseq_brkpt_get(VALUE iseqval, VALUE offsetval)
{
    if (Qnil != iseqval) {
	rb_iseq_t *iseq;
	unsigned long int uoffset;
	
	GetISeqPtr(iseqval, iseq);
	if (!iseq->breakpoints) return Qfalse;

	uoffset = iseq_get_offset_internal(iseq, offsetval);
	      
	/* FIXME: check that offset is at a valid instruction offset */
	return (0 != iseq->breakpoints[uoffset]) ? Qtrue : Qfalse;
    }
    return Qnil;
}

static VALUE
iseq_brkpt_set_unset(VALUE iseqval, VALUE offsetval, char val)
{
    if (Qnil != iseqval) {
	rb_iseq_t *iseq;
	unsigned long int uoffset;

	GetISeqPtr(iseqval, iseq);
	if (!iseq->breakpoints) {
	    if (val) {
		/* Set breakpoint requested */
		VALUE alloc_ret = iseq_brkpt_alloc(iseqval);
		if (!iseq->breakpoints) return alloc_ret;
	    } else 
		/* Unset breakpoint requested. */
		return Qtrue;
	}
	uoffset = iseq_get_offset_internal(iseq, offsetval);

	/* FIXME: check that offset is at a valid instruction offset */
	iseq->breakpoints[uoffset] = val;
	return Qtrue;
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
 *
 * True is returned if the breakpoint is now set. An IndexError can or a
 * TypeError can be raised if values are invalid.  Negative values of
 * <i>offset</i> count from the end of the instruction sequence.
 */
VALUE
iseq_brkpt_set(VALUE iseqval, VALUE offsetval)
{
    return iseq_brkpt_set_unset(iseqval, offsetval, '\001');
}

/* 
 * Document-method: RubyVM::InstructionSequence::brkpt_unset
 *
 * call-seq:
 *     RubyVM::InstructionSequence#brkpt_unset(offset) -> bool
 *
 *  Unsets breakpoint of byte vector at +offset+.
 * 
 *  True is returned if the breakpoint is now unset. An IndexError can or
 *  a TypeError can be raised if values are invalid.  However if the
 *  instruction sequence does not have any breakpoints allocated, we
 *  will not check whether the offset is valid in any way.
 * 
 *  Negative values of <i>offset</i> count from the end of the
 *  instruction sequence. 
 */
VALUE
iseq_brkpt_unset(VALUE iseqval, VALUE offsetval)
{
    return iseq_brkpt_set_unset(iseqval, offsetval, '\000');
}

/* 
 * Document-method: RubyVM::InstructionSequence::brkpts
 *
 * call-seq:
 *     RubyVM::InstructionSequence#brkpts -> Array
 *
 *  Returns a list of breakpoints in effect for this instruction sequence.
 *  If no breakpoints have been allocated nil is returned. If breakpoints
 *  were allocated but none are set then the empty array is returned.
 */
VALUE
iseq_brkpts(VALUE iseqval)
{
    rb_iseq_t *iseq;
    if (Qnil == iseqval) return Qnil;
    else {
	GetISeqPtr(iseqval, iseq);
	if (iseq->breakpoints) {
	    unsigned int offset;
	    VALUE ary = rb_ary_new();
	    for (offset = 0; offset < iseq->iseq_size; offset++) {
		if (iseq->breakpoints[offset])
		    rb_ary_push(ary, INT2FIX(offset));
	    }
	    return ary;
	} else {
	    return Qnil;
	}
    }
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
      long int i = FIX2INT(val);
      long int size;
      
      GetISeqPtr(iseqval, iseq);
      
      size = iseq->local_table_size;
      
      if (i < 0) i = size + i;
      
      if (i >= size)
	rb_raise(rb_eIndexError, 
		 "local table index %ld should be in the range -%ld .. %ld",
		 i, size, size-1);
      
      rb_str_new2(rb_id2name(iseq->local_table[i]));
    } else {
      rb_raise(rb_eTypeError, "type mismatch: %s given, Fixnum expected", 
	       rb_class2name(CLASS_OF(val)));
    }
}

/* 
 * call-seq:
 *     RubyVM::InstructionSequence#name -> String
 * 
 *  Returns the name if the instruction sequence.
 */
VALUE
iseq_name(VALUE iseqval)
{
    rb_iseq_t *iseq;
    GetISeqPtr(iseqval, iseq);
    return(iseq->name);
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
    VALUE *iseq ;
    unsigned long i, size, count = 0;
    struct iseq_insn_info_entry *table;

    GetISeqPtr(iseqval, iseqdat);
    iseq = iseqdat->iseq;
    size = iseqdat->insn_info_size;
    table = iseqdat->insn_info_table;
    for (i = 0; i < size; i++) {
      const unsigned long pos = table[i].position;
      const VALUE insn = iseq[pos];
      if (0 == strncmp(insn_name(insn), "getinlinecache", 
		       sizeof("getinlinecache")))
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
    if (len > 0 && 
	((file[0] == '(' && file[len-1] == ')')
	 || 0 == strncmp(file, "<compiled>", 
			 sizeof("<compiled>"))))
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


#if 0
/*
 * call-seq:
 *    RubyVM::InstructionSequence#type() -> Fixnum
 *
 * Returns instruction-sequence type.
 */
static VALUE
iseq_type(VALUE iseqval)
#endif

#define ISEQ_FIELD_METHOD(FIELD)		\
static VALUE					\
iseq_##FIELD(VALUE iseqval)			\
{						\
  rb_iseq_t *iseq;				\
  if (Qnil == iseqval) return Qnil;		\
  GetISeqPtr(iseqval, iseq);			\
  return iseq->FIELD;				\
}

ISEQ_FIELD_METHOD(orig) ;
ISEQ_FIELD_METHOD(self) ;
ISEQ_FIELD_METHOD(type) ;

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
ISEQ_INT_FIELD_METHOD(iseq_size) ;
ISEQ_INT_FIELD_METHOD(klass) ;
ISEQ_INT_FIELD_METHOD(line_no) ;
ISEQ_INT_FIELD_METHOD(local_size) ;
ISEQ_INT_FIELD_METHOD(local_table_size) ;


/* RDoc can't find methods when we use a definition like this: */
#define RB_DEFINE_ISEQ_METHOD(FIELD, ARGC) \
    rb_define_method(rb_cISeq, #FIELD, iseq_##FIELD, ARGC);

void
Init_iseq_extra(void)
{
    rb_define_method(rb_cISeq, "arg_block",        iseq_arg_block, 0) ;
    rb_define_method(rb_cISeq, "arg_opts",         iseq_arg_opts, 0) ;
    rb_define_method(rb_cISeq, "arg_post_len",     iseq_arg_post_len, 0) ;
    rb_define_method(rb_cISeq, "arg_rest",         iseq_arg_rest, 0) ;
    rb_define_method(rb_cISeq, "arg_simple",       iseq_arg_simple, 0) ;
    rb_define_method(rb_cISeq, "argc",             iseq_argc, 0) ;
    rb_define_method(rb_cISeq, "arity",            iseq_arity, 0);
#ifdef HAVE_COMPILE_OPTIONS
    rb_define_method(rb_cISeq, "compile_options",  iseq_compile_options, 0) ;
#endif
    rb_define_method(rb_cISeq, "brkpt_alloc",      iseq_brkpt_alloc, 0) ;
    rb_define_method(rb_cISeq, "brkpt_dealloc",    iseq_brkpt_dealloc, 0) ;
    rb_define_method(rb_cISeq, "brkpt_get",        iseq_brkpt_get, 1) ;
    rb_define_method(rb_cISeq, "brkpt_set",        iseq_brkpt_set, 1) ;
    rb_define_method(rb_cISeq, "brkpt_unset",      iseq_brkpt_unset, 1) ;
    rb_define_method(rb_cISeq, "brkpts",           iseq_brkpts, 0) ;
    rb_define_method(rb_cISeq, "equal?",           iseq_equal, 1) ;
    rb_define_method(rb_cISeq, "iseq_size",        iseq_iseq_size, 0) ;
    rb_define_method(rb_cISeq, "killcache",        iseq_killcache, 0) ;
    rb_define_method(rb_cISeq, "klass",            iseq_klass, 0) ;
    rb_define_method(rb_cISeq, "line_no",          iseq_line_no, 0) ;
    rb_define_method(rb_cISeq, "local_name",       iseq_local_name, 1) ;
    rb_define_method(rb_cISeq, "local_size",       iseq_local_size, 0) ;
    rb_define_method(rb_cISeq, "local_table_size", iseq_local_table_size, 0) ;
    rb_define_method(rb_cISeq, "offset2lines",     iseq_offset2lines, 1) ;
    rb_define_method(rb_cISeq, "offsetlines",      iseq_offsetlines, 0) ;
    rb_define_method(rb_cISeq, "orig",             iseq_orig, 0) ;
    rb_define_method(rb_cISeq, "name",             iseq_name, 0) ;
    rb_define_method(rb_cISeq, "self",             iseq_self, 0) ;
    rb_define_method(rb_cISeq, "source_container", iseq_source_container, 0) ;
    rb_define_method(rb_cISeq, "type",             iseq_type, 0) ;

}
