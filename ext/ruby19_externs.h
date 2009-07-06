/* Ruby 1.9 externs that we need. 

   We assume all structure typedefs needed below have been defined
   previously 
*/


extern rb_control_frame_t * thread_context_frame(void *);
extern rb_thread_t *ruby_current_thread;
extern rb_control_frame_t * rb_vm_get_ruby_level_next_cfp(rb_thread_t *th, rb_control_frame_t *cfp);
extern int rb_vm_get_sourceline(const rb_control_frame_t *cfp);
extern VALUE rb_iseq_disasm_internal(rb_iseq_t *iseqdat);
extern VALUE rb_cRubyVM;  /* RubyVM class */
