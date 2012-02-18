static int pline_callback_info(const char **p_srcfile, long *p_line)
{
  const char *srcfile;
  long line;

  srcfile = rb_sourcefile();
  if (!srcfile) {
    return 0;
  }

  line = rb_sourceline();
  if (line < 0) {
    return 0;
  }

  *p_srcfile = srcfile;
  *p_line = line;

  return 1;
}

static void pline_callback(rb_event_flag_t event, VALUE arg, VALUE self, ID id, VALUE klass)
{
  const char *srcfile;
  long line;
  int success = pline_callback_info(&srcfile, &line);
  VALUE sinfo;

  if (!success) return;

  sinfo = sinfo_find_force(srcfile);
  sinfo_measure(sinfo, line);

  return;
}

static void pline_hook_line(int remove)
{
  static int hooked = 0;

  if (remove) {
    if (!hooked) return;
    hooked = 0;
    rb_remove_event_hook(&pline_callback);
    return;
  }

  if (hooked) {
    return;
  }

  rb_add_event_hook(&pline_callback, RUBY_EVENT_END, Qnil);
  hooked = 1;

  return;
}

static VALUE pline_m_profile(VALUE self, VALUE obj, VALUE mid, VALUE singleton_p)
{
  rb_iseq_t *iseq;
  VALUE minfo;

  if (rb_obj_class(mid) != rb_cSymbol) {
    rb_raise(rb_eArgError, "second argument should be symbol");
  }

  iseq = pline_find_iseq(obj, mid, singleton_p);
  minfo = rb_funcall(cMethodInfo, rb_intern("new"), 4, iseq->self, obj, mid, singleton_p);
  pline_inject(iseq);
  pline_hook_line(0);
  rb_funcall(cMethodInfo, rb_intern("register"), 1, minfo);

  return Qnil;
}

static void pline_profile_init(void)
{
  rb_define_singleton_method(mPLine, "profile", pline_m_profile, 3);
}

