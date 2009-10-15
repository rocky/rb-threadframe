struct iseq_compile_data {
    /* GC is needed */
    VALUE err_info;
    VALUE mark_ary;
    VALUE catch_table_ary;	/* Array */

    /* GC is not needed */
    struct iseq_label_data *start_label;
    struct iseq_label_data *end_label;
    struct iseq_label_data *redo_label;
    VALUE current_block;
    VALUE ensure_node;
    VALUE for_iseq;
    struct iseq_compile_data_ensure_node_stack *ensure_node_stack;
    int loopval_popped;	/* used by NODE_BREAK */
    int cached_const;
    struct iseq_compile_data_storage *storage_head;
    struct iseq_compile_data_storage *storage_current;
    int last_line;
    int last_coverable_line;
    int flip_cnt;
    int label_no;
    int node_level;
    /*const*/ rb_compile_option_t *option;  // "const" removed 
};


