static int profiler_source_location(const char **p_srcfile, long *p_line)
{
#if 0
  VALUE thval = rb_thread_current();
  rb_thread_t *th = DATA_PTR(thval);
  rb_control_frame_t *cfp = th->cfp;
  rb_iseq_t *iseq;
  const char *srcfile;
  long line = -1, i, pc;

  /* find cfp */
  while (!RUBY_VM_CONTROL_FRAME_STACK_OVERFLOW_P(th, cfp)) {
    if (RUBY_VM_NORMAL_ISEQ_P(cfp->iseq)) {
        break;
    }
    cfp = RUBY_VM_PREVIOUS_CONTROL_FRAME(cfp);
  }
  if (RUBY_VM_CONTROL_FRAME_STACK_OVERFLOW_P(th, cfp)) {
    return 0;
  }
  iseq = cfp->iseq;

  /* find sourcefile */
  srcfile = RSTRING_PTR(iseq->filename);
  if (!srcfile) {
    return 0;
  }

  /* find line */
  if (iseq->insn_info_size <= 0) {
    return 0;
  }
  pc = cfp->pc - iseq->iseq_encoded;
  for (i = 0; i < iseq->insn_info_size; i++) {
    if (iseq->insn_info_table[i].position == pc) {
      line = iseq->insn_info_table[i - 1].line_no;
      break;
    }
  }
  if (line < 0) {
    line = iseq->insn_info_table[i - 1].line_no;
  }
  if (line < 0) {
    rb_bug("pline_callback_info: should not be reached");
  }
#else
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
#endif

  *p_srcfile = srcfile;
  *p_line = line;

  return 1;
}

static void profiler_linetrace_callback(rb_event_flag_t event, VALUE arg, VALUE self, ID id, VALUE klass)
{
  const char *srcfile;
  long line;
  int success = profiler_source_location(&srcfile, &line);
  VALUE sinfo;

  if (!success) return;
  sinfo_measure(srcfile, line);

  return;
}

static void profiler_linetrace(int remove)
{
  static int hooked = 0;

  if (remove) {
    if (!hooked) return;
    hooked = 0;
    rb_remove_event_hook(&profiler_linetrace_callback);
    return;
  }

  if (hooked) {
    return;
  }

  rb_add_event_hook(&profiler_linetrace_callback, RUBY_EVENT_END, Qnil);
  hooked = 1;

  return;
}

static VALUE profiler_m_profile(int argc, VALUE *argv, VALUE self)
{
  rb_iseq_t *iseq;
  VALUE minfo, obj, mid, singleton_p = Qfalse;

  rb_scan_args(argc, argv, "21", &obj, &mid, &singleton_p);

  if (rb_obj_class(mid) != rb_cSymbol) {
    rb_raise(rb_eArgError, "second argument should be symbol");
  }

  iseq = iseq_find(obj, mid, singleton_p);
  minfo = rb_funcall(cMethodInfo, rb_intern("new"), 4, iseq->self, obj, mid, singleton_p);
  iseq_inject(iseq);
  profiler_linetrace(0);
  rb_funcall(cMethodInfo, rb_intern("register"), 1, minfo);

  return Qnil;
}

static void pline_profiler_init(void)
{
  rb_define_singleton_method(mPLine, "profile", profiler_m_profile, -1);
}

