/* Headers Exposing a little more of the 1.9 runtime and some 
   method prototypes for extensions to the Thread class.
*/
#include <ruby.h>
#include "vm_core_mini.h"

/* This is copied verbatim from vm_core.h */
typedef struct {
    VALUE *pc;			/* cfp[0] */
    VALUE *sp;			/* cfp[1] */
    VALUE *bp;			/* cfp[2] */
    rb_iseq_t *iseq;		/* cfp[3] */
    VALUE flag;			/* cfp[4] */
    VALUE self;			/* cfp[5] / block[0] */
    VALUE *lfp;			/* cfp[6] / block[1] */
    VALUE *dfp;			/* cfp[7] / block[2] */
    rb_iseq_t *block_iseq;	/* cfp[8] / block[3] */
    VALUE proc;			/* cfp[9] / block[4] */
    ID method_id;               /* cfp[10] saved in special case */
    VALUE method_class;         /* cfp[11] saved in special case */
} rb_control_frame_t;

typedef void rb_vm_t;

typedef struct rb_thread_struct
{
    VALUE self;
    rb_vm_t *vm;

    /* execution information */
    VALUE *stack;		/* must free, must mark. rb: seems to be nil. */
    unsigned long stack_size;   /* Number of stack (or rb_control_frame_t) entries */
    rb_control_frame_t *cfp;

    int safe_level;
    int raised_flag;
    VALUE last_status; /* $? */

    /* passing state */
    int state;

    /* Lot's of other stuff ... */
} rb_thread_t;


#define GetThreadPtr(obj, ptr) \
  GetCoreDataFromValue(obj, rb_thread_t, ptr)

#if 1
#define GetCoreDataFromValue(obj, type, ptr) do { \
    ptr = (type*)DATA_PTR(obj); \
} while (0)
#else
#define GetCoreDataFromValue(obj, type, ptr) Data_Get_Struct(obj, type, ptr)
#endif


RUBY_EXTERN VALUE thread_extra_ni(VALUE klass);

