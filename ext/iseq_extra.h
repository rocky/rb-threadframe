/* in iseq_extra.c */
extern VALUE iseq_argc(VALUE);
extern VALUE iseq_arity(VALUE);
extern void  Init_iseq_extra(void);
extern VALUE iseq_source_container_internal(rb_iseq_t *iseq);
const  char *source_container_type(VALUE fileval);


