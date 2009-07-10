/* Headers Exposing a little more of the 1.9 runtime and some 
   method prototypes for extensions to the Thread class.
*/
#include <ruby.h>
#include "vm_core_mini.h"

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

