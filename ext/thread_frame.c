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

#define RUBY_VM_PREVIOUS_CONTROL_FRAME(cfp) (cfp+1)

VALUE rb_cThreadFrame;

static VALUE
thread_frame_s_new(VALUE klass)
{
    rb_control_frame_t *data;
    VALUE thread_frame = Data_Make_Struct(klass, rb_control_frame_t, 0, -1, 
					  data);
    return thread_frame;
}

extern rb_control_frame_t * thread_context_frame(void *);

static VALUE
thread_frame_s_current(VALUE klass)
{
    rb_control_frame_t *cfp = thread_context_frame(NULL);
    VALUE thread_frame = Data_Wrap_Struct(klass, NULL, NULL, cfp);
    return thread_frame;
}

extern rb_thread_t *ruby_current_thread;

static VALUE
thread_frame_binding(VALUE klass)
{
    rb_control_frame_t *cfp;
    rb_thread_t * th = ruby_current_thread;
    Data_Get_Struct(klass, rb_control_frame_t, cfp);
    return rb_binding_frame_new(th, cfp);
}

static VALUE
thread_frame_bp(VALUE klass)
{
    rb_control_frame_t *cfp;
    Data_Get_Struct(klass, rb_control_frame_t, cfp);
    return *(cfp->bp);
}

static VALUE
thread_frame_pc(VALUE klass)
{
    rb_control_frame_t *cfp;
    Data_Get_Struct(klass, rb_control_frame_t, cfp);
    return *(cfp->pc);
}

static VALUE
thread_frame_prev(VALUE klass)
{
    rb_control_frame_t *cfp;
    Data_Get_Struct(klass, rb_control_frame_t, cfp);
    cfp = RUBY_VM_PREVIOUS_CONTROL_FRAME(cfp);
    if (cfp->iseq) {
      VALUE thread_frame = Data_Wrap_Struct(rb_cThreadFrame, NULL, NULL, cfp);
      return thread_frame;
    } else return Qnil;
}

static VALUE
thread_frame_proc(VALUE klass)
{
    rb_control_frame_t *cfp;
    Data_Get_Struct(klass, rb_control_frame_t, cfp);
    return cfp->proc;
}

static VALUE
thread_frame_method_class(VALUE klass)
{
    rb_control_frame_t *cfp;
    Data_Get_Struct(klass, rb_control_frame_t, cfp);
    return cfp->method_class;
}

static VALUE
thread_frame_self(VALUE klass)
{
    rb_control_frame_t *cfp;
    Data_Get_Struct(klass, rb_control_frame_t, cfp);
    return cfp->self;
}

static VALUE
thread_frame_sp(VALUE klass)
{
    rb_control_frame_t *cfp;
    Data_Get_Struct(klass, rb_control_frame_t, cfp);
    return *(cfp->sp);
}

void
Init_thread_frame(void)
{
  rb_cThreadFrame = rb_define_class("ThreadFrame", rb_cObject);
  rb_define_singleton_method(rb_cThreadFrame, "new", 
			     thread_frame_s_new, 0);
  rb_define_singleton_method(rb_cThreadFrame, "current", 
			     thread_frame_s_current, 0);
  rb_define_method(rb_cThreadFrame, "bp", thread_frame_bp, 0);
  rb_define_method(rb_cThreadFrame, "method_class", thread_frame_method_class,
		   0);
  rb_define_method(rb_cThreadFrame, "pc", thread_frame_pc, 0);
  rb_define_method(rb_cThreadFrame, "prev", thread_frame_prev, 0);
  rb_define_method(rb_cThreadFrame, "proc", thread_frame_proc, 0);
  rb_define_method(rb_cThreadFrame, "self", thread_frame_self, 0);
  rb_define_method(rb_cThreadFrame, "sp", thread_frame_sp, 0);
  rb_define_method(rb_cThreadFrame, "binding", thread_frame_binding, 0);
}

