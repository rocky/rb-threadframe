/* Headers Exposing a little more of the 1.9 runtime and some 
   method prototypes for extensions to the Thread class.
*/
#include <ruby.h>
#include <signal.h>
#include "thread_pthread.h"
#include "node.h"

/* From vm_core.h: */

/* Frame information: */
#define VM_FRAME_MAGIC_METHOD 0x11
#define VM_FRAME_MAGIC_BLOCK  0x21
#define VM_FRAME_MAGIC_CLASS  0x31
#define VM_FRAME_MAGIC_TOP    0x41
#define VM_FRAME_MAGIC_FINISH 0x51
#define VM_FRAME_MAGIC_CFUNC  0x61
#define VM_FRAME_MAGIC_PROC   0x71
#define VM_FRAME_MAGIC_IFUNC  0x81
#define VM_FRAME_MAGIC_EVAL   0x91
#define VM_FRAME_MAGIC_LAMBDA 0xa1
#define VM_FRAME_MAGIC_MASK_BITS   8
#define VM_FRAME_MAGIC_MASK   (~(~0<<VM_FRAME_MAGIC_MASK_BITS))

#define VM_FRAME_TYPE(cfp) ((cfp)->flag & VM_FRAME_MAGIC_MASK)

#define VM_FRAME_TRACE_RETURN 0x01  /* Call trace hook on return. */
#define VM_FRAME_TRACE_OFF    0x02  /* Turn of event hook tracing in this frame
				       and any frames created from this one. */

/* other frame flag */
#define VM_FRAME_FLAG_PASSED 0x0100

#define RUBY_VM_PREVIOUS_CONTROL_FRAME(cfp) (cfp+1)
#define RUBYVM_CFUNC_FRAME_P(cfp) \
  (VM_FRAME_TYPE(cfp) == VM_FRAME_MAGIC_CFUNC)
#define RUBY_VM_NEXT_CONTROL_FRAME(cfp) (cfp-1)
#define RUBY_VM_END_CONTROL_FRAME(th) \
  ((rb_control_frame_t *)((th)->stack + (th)->stack_size))
#define RUBY_VM_VALID_CONTROL_FRAME_P(cfp, ecfp) \
  ((void *)(ecfp) > (void *)(cfp))
#define RUBY_VM_CONTROL_FRAME_STACK_OVERFLOW_P(th, cfp) \
  (!RUBY_VM_VALID_CONTROL_FRAME_P((cfp), RUBY_VM_END_CONTROL_FRAME(th)))

#define RUBY_VM_IFUNC_P(ptr)        (BUILTIN_TYPE(ptr) == T_NODE)
#define RUBY_VM_NORMAL_ISEQ_P(ptr) \
  (ptr && !RUBY_VM_IFUNC_P(ptr))


#if 1
#define GetCoreDataFromValue(obj, type, ptr) do { \
    ptr = (type*)DATA_PTR(obj); \
} while (0)
#else
#define GetCoreDataFromValue(obj, type, ptr) Data_Get_Struct(obj, type, ptr)
#endif
#if 1
#define GetCoreDataFromValue(obj, type, ptr) do { \
    ptr = (type*)DATA_PTR(obj); \
} while (0)
#else
#define GetCoreDataFromValue(obj, type, ptr) Data_Get_Struct(obj, type, ptr)
#endif
#define GetISeqPtr(obj, ptr) \
  GetCoreDataFromValue(obj, rb_iseq_t, ptr)

/* Opaque types (for now at least) */
typedef struct iseq_catch_table_entry iseq_catch_table_entry_t;

#ifndef NSIG
# define NSIG (_SIGMAX + 1)      /* For QNX */
#endif

#define RUBY_NSIG NSIG

typedef struct rb_compile_option_struct {
    int inline_const_cache;
    int peephole_optimization;
    int tailcall_optimization;
    int specialized_instruction;
    int operands_unification;
    int instructions_unification;
    int stack_caching;
    int trace_instruction;
    int debug_level;
} rb_compile_option_t;

/* Instruction sequence */
typedef struct rb_iseq_struct {
    /***************/
    /* static data */
    /***************/

    VALUE type;          /* instruction sequence type */
    VALUE name;          /* String: iseq name */
    VALUE filename;      /* file information where this sequence from */
    VALUE filepath;      /* real file path or nil */
    VALUE *iseq;         /* iseq (insn number and operands) */
    VALUE *iseq_encoded; /* encoded iseq */
    unsigned long iseq_size;
    VALUE mark_ary;     /* Array: includes operands which should be GC marked */
    VALUE coverage;     /* coverage array */
    unsigned short line_no;

    /* insn info, must be freed */
    struct iseq_insn_info_entry *insn_info_table;
    size_t insn_info_size;

    ID *local_table;            /* must free */
    int local_table_size;

    /* method, class frame: sizeof(vars) + 1, block frame: sizeof(vars) */
    int local_size;

    struct iseq_inline_cache_entry *ic_entries;
    int ic_size;

    /**
     * argument information
     *
     *  def m(a1, a2, ..., aM,                    # mandatory
     *        b1=(...), b2=(...), ..., bN=(...),  # optional
     *        *c,                                 # rest
     *        d1, d2, ..., dO,                    # post
     *        &e)                                 # block
     * =>
     *
     *  argc           = M
     *  arg_rest       = M+N+1 // or -1 if no rest arg
     *  arg_opts       = N
     *  arg_opts_tbl   = [ (N entries) ]
     *  arg_post_len   = O // 0 if no post arguments
     *  arg_post_start = M+N+2
     *  arg_block      = M+N + 1 + O + 1 // -1 if no block arg
     *  arg_simple     = 0 if not simple arguments.
     *                 = 1 if no opt, rest, post, block.
     *                 = 2 if ambiguous block parameter ({|a|}).
     *  arg_size       = argument size.
     */

    int argc;
    int arg_simple;
    int arg_rest;
    int arg_block;
    int arg_opts;
    int arg_post_len;
    int arg_post_start;
    int arg_size;
    VALUE *arg_opt_table;

    size_t stack_max; /* for stack overflow check */

    /* catch table */
    iseq_catch_table_entry_t *catch_table;
    int catch_table_size;

    /* for child iseq */
    struct rb_iseq_struct *parent_iseq;
    struct rb_iseq_struct *local_iseq;

    /****************/
    /* dynamic data */
    /****************/

    VALUE self;
    VALUE orig;                 /* non-NULL if its data have origin */

    /* block inlining */
    /*
     * NODE *node;
     * void *special_block_builder;
     * void *cached_special_block_builder;
     * VALUE cached_special_block;
     */

    /* klass/module nest information stack (cref) */
    NODE *cref_stack;
    VALUE klass;

    /* misc */
    ID defined_method_id;       /* for define_method */

    /* used at compile time */
    struct iseq_compile_data *compile_data;
    /* Used to set a breakpoint at a VM instruction */
    unsigned char *breakpoints; 
} rb_iseq_t;

enum ruby_special_exceptions {
    ruby_error_reenter,
    ruby_error_nomemory,
    ruby_error_sysstack,
    ruby_special_error_count
};

typedef struct rb_vm_struct {
    VALUE self;

    rb_thread_lock_t global_vm_lock;

    struct rb_thread_struct *main_thread;
    struct rb_thread_struct *running_thread;

    st_table *living_threads;
    VALUE thgroup_default;

    int running;
    int thread_abort_on_exception;
    unsigned long trace_flag;
    volatile int sleeper;

    /* object management */
    VALUE mark_object_ary;

    VALUE special_exceptions[ruby_special_error_count];

    /* load */
    VALUE top_self;
    VALUE load_path;
    VALUE loaded_features;
    struct st_table *loading_table;

    /* signal */
    struct {
        VALUE cmd;
        int safe;
    } trap_list[RUBY_NSIG];

    /* hook */
    rb_event_hook_t *event_hooks;

    int src_encoding_index;

    VALUE verbose, debug, progname;
    VALUE coverages;

    struct unlinked_method_entry_list_entry *unlinked_method_entry_list;

#if defined(ENABLE_VM_OBJSPACE) && ENABLE_VM_OBJSPACE
    struct rb_objspace *objspace;
#endif
} rb_vm_t;

#include "method_mini.h"

typedef struct {
    VALUE *pc;                  /* cfp[0] */
    VALUE *sp;                  /* cfp[1] */
    VALUE *bp;                  /* cfp[2] */
    rb_iseq_t *iseq;            /* cfp[3] */
    VALUE flag;                 /* cfp[4] */
    VALUE self;                 /* cfp[5] / block[0] */
    VALUE *lfp;                 /* cfp[6] / block[1] */
    VALUE *dfp;                 /* cfp[7] / block[2] */
    rb_iseq_t *block_iseq;      /* cfp[8] / block[3] */
    VALUE proc;                 /* cfp[9] / block[4] */
    const rb_method_entry_t *me;/* cfp[10] */
    short int tracing;          /* Bits to control per-frame event tracing. 
				   See VM_FRAME_TRACE_xxx defines.
				 */
} rb_control_frame_t;

typedef struct rb_block_struct {
    VALUE self;                 /* share with method frame if it's only block */
    VALUE *lfp;                 /* share with method frame if it's only block */
    VALUE *dfp;                 /* share with method frame if it's only block */
    rb_iseq_t *iseq;
    VALUE proc;
} rb_block_t;

#define GetThreadPtr(obj, ptr) \
  GetCoreDataFromValue(obj, rb_thread_t, ptr)

#define GetProcPtr(obj, ptr) \
  GetCoreDataFromValue(obj, rb_proc_t, ptr)

typedef struct rb_thread_struct
{
    VALUE self;
    rb_vm_t *vm;

    /* execution information */
    VALUE *stack;               /* must free, must mark. rb: seems to be nil. */
    unsigned long stack_size;   /* Number of stack (or rb_control_frame_t) entries */
    rb_control_frame_t *cfp;

    int safe_level;
    int raised_flag;
    VALUE last_status; /* $? */

    /* passing state */
    int state;

    /* tracer */
    rb_event_hook_t *event_hooks;
    rb_event_flag_t event_flags;
    int tracing;  /* 0 if not tracing. If less than 0, skip that many
                     C call/return pairs */
    int exec_event_tracing;  /* 0 if not in rb_threadptr_evec_event_hooks. */
    int trace_skip_insn_count; /* # of VM instructions to skip */

    /* misc */
    int method_missing_reason;
    int abort_on_exception;

    /* for rb_iterate */
    const rb_block_t *passed_block;

    /* for bmethod */
    const rb_method_entry_t *passed_me;

    /* for load(true) */
    VALUE top_self;
    VALUE top_wrapper;

    /* eval env */
    rb_block_t *base_block;

    VALUE *local_lfp;
    VALUE local_svar;

    /* Lot's of other stuff ... 
       thread control ... */
} rb_thread_t;

typedef struct {
    rb_block_t block;

    VALUE envval;               /* for GC mark */
    VALUE blockprocval;
    int safe_level;
    int is_from_method;
    int is_lambda;
} rb_proc_t;

#define GetEnvPtr(obj, ptr) \
  GetCoreDataFromValue(obj, rb_env_t, ptr)

typedef struct {
    VALUE *env;
    int env_size;
    int local_size;
    VALUE prev_envval;		/* for GC mark */
    rb_block_t block;
} rb_env_t;

#define GetBindingPtr(obj, ptr) \
  GetCoreDataFromValue(obj, rb_binding_t, ptr)

typedef struct {
    VALUE env;
    VALUE filename;
    unsigned short line_no;
} rb_binding_t;

#define GET_THREAD() ruby_current_thread
extern rb_thread_t *ruby_current_thread;
