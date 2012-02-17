static int pline_callback_info(VALUE *p_klass, ID *p_id, const char **p_srcfile, long *p_line)
{
  VALUE klass = *p_klass;
  ID id = *p_id;
  const char *srcfile;
  long line;

  if (klass == 0) {
    rb_frame_method_id_and_class(&id, &klass);
  }
  if (klass) {
    if (TYPE(klass) == T_ICLASS) {
      klass = RBASIC(klass)->klass;
    } else if (FL_TEST(klass, FL_SINGLETON)) {
      klass = rb_iv_get(klass, "__attached__");
    }
  }
  if (!klass || !id || id == ID_ALLOCATOR) {
    return 0;
  }

  srcfile = rb_sourcefile();
  if (!srcfile) {
    return 0;
  }

  line = rb_sourceline();
  if (line < 0) {
    return 0;
  }

  *p_klass = klass;
  *p_id = id;
  *p_srcfile = srcfile;
  *p_line = line;

  return 1;
}

static void pline_callback(rb_event_flag_t event, VALUE arg, VALUE self, ID id, VALUE klass)
{
  const char *srcfile;
  long line;
  int success = pline_callback_info(&klass, &id, &srcfile, &line);
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

static VALUE pline_m_profile(VALUE self, VALUE klass, VALUE mid, VALUE singleton_p)
{
  rb_iseq_t *iseq;
  VALUE minfo;

  if (rb_obj_class(klass) != rb_cClass) {
    rb_raise(rb_eArgError, "first argument should be class");
  }

  if (rb_obj_class(mid) != rb_cSymbol) {
    rb_raise(rb_eArgError, "second argument should be symbol");
  }

  iseq = pline_find_iseq(klass, mid, singleton_p);
  minfo = rb_funcall(cMethodInfo, rb_intern("new"), 4, iseq->self, klass, mid, singleton_p);
  pline_inject(iseq);
  pline_hook_line(0);
  rb_funcall(cMethodInfo, rb_intern("register"), 1, minfo);

  return Qnil;
}

static void pline_profile_init(void)
{
  rb_define_singleton_method(mPLine, "profile", pline_m_profile, 3);
  pline_table = st_init_strtable();
}

