/* ----- vm_method.c ----- */
static rb_method_entry_t*
search_method(VALUE klass, ID id)
{
    st_data_t body;
    if (!klass) {
	return 0;
    }

    while (!st_lookup(RCLASS_M_TBL(klass), id, &body)) {
	klass = RCLASS_SUPER(klass);
	if (!klass) {
	    return 0;
	}
    }

    return (rb_method_entry_t *)body;
}

