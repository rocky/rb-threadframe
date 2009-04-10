#include <string.h>
#include <ruby.h>

typedef struct rb_iseq_struct rb_iseq_t;

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

typedef void rb_thread_t;

typedef struct 
{
    rb_thread_t *th;
    rb_control_frame_t *cfp;
} thread_frame_t;
  

#define RUBY_VM_PREVIOUS_CONTROL_FRAME(cfp) (cfp+1)

extern rb_control_frame_t * thread_context_frame(void *);
extern rb_thread_t *ruby_current_thread;

VALUE rb_cThreadFrame;

static VALUE
thread_frame_alloc(VALUE klass)
{
    return Data_Wrap_Struct(klass, NULL, xfree, 0);
}

static thread_frame_t *
thread_frame_t_alloc(VALUE tfval)
{
    thread_frame_t *tf = ALLOC(thread_frame_t);
    memset(tf, 0, sizeof(thread_frame_t));
    DATA_PTR(tfval) = tf;
    return tf;
}

static VALUE
thread_frame_init(VALUE tfval)
{
    thread_frame_t_alloc(tfval);
    return tfval;
}

VALUE
rb_thread_frame_new()
{
    return thread_frame_init(thread_frame_alloc(rb_cThreadFrame));
}

static VALUE
thread_frame_s_current(VALUE klass)
{
    thread_frame_t *tf = thread_frame_t_alloc(klass);
    tf->th = ruby_current_thread;
    tf->cfp = thread_context_frame(tf->th);
    return Data_Wrap_Struct(klass, NULL, xfree, tf);
}

static VALUE
thread_frame_binding(VALUE klass)
{
    thread_frame_t *tf;
    Data_Get_Struct(klass, thread_frame_t, tf);
    return rb_binding_frame_new(tf->th, tf->cfp);
}

#define THREAD_FRAME_FIELD_METHOD(FIELD)	\
static VALUE					\
thread_frame_##FIELD(VALUE klass)		\
{						\
    thread_frame_t *tf;				\
    Data_Get_Struct(klass, thread_frame_t, tf); \
    return tf->cfp->FIELD;			\
}

THREAD_FRAME_FIELD_METHOD(flag) ;
THREAD_FRAME_FIELD_METHOD(method_class) ;
THREAD_FRAME_FIELD_METHOD(proc) ;
THREAD_FRAME_FIELD_METHOD(self) ;

#define THREAD_FRAME_PTR_FIELD_METHOD(FIELD)	\
static VALUE					\
thread_frame_##FIELD(VALUE klass)		\
{						\
    thread_frame_t *tf;				\
    Data_Get_Struct(klass, thread_frame_t, tf); \
    return *(tf->cfp->FIELD);			\
}

/*THREAD_FRAME_PTR_FIELD_METHOD(bp) ;*/
THREAD_FRAME_PTR_FIELD_METHOD(dfp) ;
THREAD_FRAME_PTR_FIELD_METHOD(lfp) ;
/*THREAD_FRAME_PTR_FIELD_METHOD(pc) ;*/
/*THREAD_FRAME_PTR_FIELD_METHOD(sp) ;*/

static VALUE
thread_frame_prev(VALUE klass)
{
    rb_control_frame_t *prev_cfp;
    thread_frame_t *tf;
    Data_Get_Struct(klass, thread_frame_t, tf);
    prev_cfp = RUBY_VM_PREVIOUS_CONTROL_FRAME(tf->cfp);
    if (prev_cfp->iseq) {
        VALUE prev = rb_thread_frame_new();
        thread_frame_t *tf_prev;
        Data_Get_Struct(prev, thread_frame_t, tf_prev);
        tf_prev->cfp = prev_cfp;
        tf_prev->th = tf->th;
        return prev;
    } else return Qnil;
}

#define RB_DEFINE_FIELD_METHOD(FIELD) \
    rb_define_method(rb_cThreadFrame, #FIELD, thread_frame_##FIELD, 0);

void
Init_thread_frame(void)
{
  rb_cThreadFrame = rb_define_class("ThreadFrame", rb_cObject);
  rb_define_alloc_func(rb_cThreadFrame, thread_frame_alloc);
  rb_define_method(rb_cThreadFrame, "initialize", thread_frame_init, 0);
  rb_define_singleton_method(rb_cThreadFrame, "current", 
			     thread_frame_s_current, 0);
  RB_DEFINE_FIELD_METHOD(binding);
  /*RB_DEFINE_FIELD_METHOD(bp);*/
  RB_DEFINE_FIELD_METHOD(dfp);
  RB_DEFINE_FIELD_METHOD(flag);
  RB_DEFINE_FIELD_METHOD(lfp);
  RB_DEFINE_FIELD_METHOD(method_class);
  /*RB_DEFINE_FIELD_METHOD(pc);*/
  RB_DEFINE_FIELD_METHOD(prev);
  RB_DEFINE_FIELD_METHOD(proc);
  RB_DEFINE_FIELD_METHOD(self);
  /*RB_DEFINE_FIELD_METHOD(sp);*/
}

