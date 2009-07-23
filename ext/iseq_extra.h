/* From Ruby 1.9 */
extern VALUE rb_cISeq;
extern int get_iseq_arity(rb_iseq_t *iseq);

/* in iseq_extra.h */
extern VALUE iseq_arity(rb_iseq_t *iseq);
