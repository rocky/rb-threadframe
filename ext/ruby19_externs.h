/* Ruby 1.9 externs that we need. 

   We assume all structure typedefs needed below have been defined
   previously. 

   Routines marked "new" are not found in an unmodified Ruby 1.9.
   Routines marked remove "static" are static routines that need to be
   made extern.
*/


/* From iseq */
extern VALUE iseq_alloc_shared(VALUE klass); /* new */
extern VALUE rb_cISeq;
extern const char * ruby_node_name(int node);

/* From proc.c */
extern int   get_iseq_arity(rb_iseq_t *iseq);  /* new */
extern int   method_arity(VALUE method);       /* removed "static" */
extern VALUE rb_binding_frame_new(void *vth, void *vcfp);  /* new */


/* From thread.c */
extern rb_control_frame_t * thread_context_frame(void *); /* new */
extern VALUE rb_cThread;  /* Thread class */


extern VALUE rb_iseq_disasm_internal(rb_iseq_t *iseqdat); /* new */
extern VALUE rb_cRubyVM;  /* RubyVM class */

/* From vm.c */
extern int rb_vm_get_sourceline(const rb_control_frame_t *cfp);
extern rb_control_frame_t * rb_vm_get_ruby_level_next_cfp(rb_thread_t *th, rb_control_frame_t *cfp); 
 
/* From node.c */
extern VALUE rb_parser_dump_tree(NODE *node, int comment);
