static int pline_summarize_i(st_data_t key, st_data_t value, st_data_t arg)
{
  VALUE result = (VALUE)arg;
  VALUE iv = (VALUE)value;
  const char *srcfile = (const char *)key;
  pline_src_info_t *info = DATA_PTR(iv);
  VALUE src = rb_funcall(mPLine, rb_intern("source"), 1, rb_str_new2(srcfile));
  long i;

  rb_ary_push(result, rb_str_new2(srcfile));
  for(i = 0; i < RARRAY_LEN(src); i++) {
    VALUE line = LONG2NUM(idx2line(i));
    VALUE time;
    if (i < info->size) {
      time = LL2NUM(nano2micro(info->vals[i]));
    } else {
      time = LONG2NUM(0);
    }
    rb_ary_push(result, rb_funcall(mPLine, rb_intern("summarize_line"), 3, src, line, time));
  }
  rb_ary_push(result, rb_str_new2(""));

  return ST_CONTINUE;
}

static VALUE pline_m_summarize(VALUE self)
{
  VALUE result = rb_ary_new();
  st_foreach(pline_table, pline_summarize_i, (st_data_t)result);
  return result;
}

static void pline_summarize_init(void)
{
  rb_define_singleton_method(mPLine, "summarize", pline_m_summarize, 0);
}

