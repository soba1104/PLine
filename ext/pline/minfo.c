typedef struct pline_method_info {
  VALUE obj;
  VALUE mid;
  VALUE spath;
  VALUE sline;
  VALUE eline;
  VALUE singleton_p;
} pline_method_info_t;

static void minfo_mark(void *ptr)
{
  pline_method_info_t *m = ptr;

  if (!m) return;
  rb_gc_mark(m->obj);
  rb_gc_mark(m->mid);
  rb_gc_mark(m->spath);
  rb_gc_mark(m->sline);
  rb_gc_mark(m->eline);
  rb_gc_mark(m->singleton_p);
}

static void minfo_free(void *p)
{
  if (p) xfree(p);
}

static const rb_data_type_t minfo_data_type = {
  "pline_method_info",
  {minfo_mark, minfo_free, NULL,},
};

static VALUE minfo_s_alloc(VALUE klass)
{
  VALUE obj;
  pline_method_info_t *m;

  obj = TypedData_Make_Struct(klass, pline_method_info_t, &minfo_data_type, m);
  m->spath = Qnil;
  m->sline = Qnil;
  m->eline = Qnil;

  return obj;
}

static void minfo_source_information_error(void)
{
  rb_raise(rb_eArgError, "unexpected source information");
}

static void check_line_information(VALUE line)
{
  if (rb_class_of(line) != rb_cFixnum || FIX2LONG(line) < 0) {
    minfo_source_information_error();
  }
}

static VALUE minfo_spath_from_iseq(VALUE iseqval)
{
  rb_iseq_t *iseq = DATA_PTR(iseqval);
  VALUE spath = iseq->filepath;
  VALUE valid = rb_funcall(rb_cFile, rb_intern("exist?"), 1, spath);

  if (!RTEST(valid)) {
    minfo_source_information_error();
  }

  return spath;
}

static VALUE minfo_sline_from_iseq(VALUE iseqval)
{
  rb_iseq_t *iseq = DATA_PTR(iseqval);
  VALUE sline = iseq->line_no;

  check_line_information(sline);

  return sline;
}

static VALUE minfo_eline_from_iseq(VALUE iseqval)
{
  rb_iseq_t *iseq = DATA_PTR(iseqval);
  VALUE eline = iseq->line_no;
  unsigned int i;

  check_line_information(eline);

  for (i = 0; i < iseq->insn_info_size; i++) {
    VALUE l = LONG2FIX(iseq->insn_info_table[i].line_no);
    if (l > eline) {
      eline = l;
    }
  }

  return eline;
}

static VALUE minfo_m_init(VALUE self, VALUE iseq, VALUE obj, VALUE mid, VALUE singleton_p)
{
  pline_method_info_t *m = DATA_PTR(self);
  VALUE spath, sline, eline;

  if (rb_obj_class(mid) != rb_cSymbol ||
      rb_obj_class(iseq) != rb_cISeq) {
    rb_raise(rb_eArgError, "invalid arguments");
  }

  m->obj = obj;
  m->mid = mid;
  m->spath = minfo_spath_from_iseq(iseq);
  m->sline = minfo_sline_from_iseq(iseq);
  m->eline = minfo_eline_from_iseq(iseq);
  m->singleton_p = singleton_p;

  return Qnil;
}

static VALUE minfo_m_spath(VALUE self)
{
  pline_method_info_t *m = DATA_PTR(self);
  return m->spath;
}

static VALUE minfo_m_sline(VALUE self)
{
  pline_method_info_t *m = DATA_PTR(self);
  return m->sline;
}

static VALUE minfo_m_eline(VALUE self)
{
  pline_method_info_t *m = DATA_PTR(self);
  return m->eline;
}

static VALUE pline_s_each_minfo(VALUE self)
{
  long i;

  for (i=0; i<RARRAY_LEN(minfo_table); i++) {
    rb_yield(RARRAY_PTR(minfo_table)[i]);
  }

  return Qnil;
}

static void pline_minfo_init(void)
{
  minfo_table = rb_ary_new();
  rb_gc_register_mark_object(minfo_table);

  cMethodInfo = rb_define_class_under(mPLine, "MethodInfo", rb_cObject);
  rb_define_method(cMethodInfo, "spath", minfo_m_spath, 0);
  rb_define_method(cMethodInfo, "sline", minfo_m_sline, 0);
  rb_define_method(cMethodInfo, "eline", minfo_m_eline, 0);
  rb_define_method(cMethodInfo, "initialize", minfo_m_init, 4);
  rb_define_alloc_func(cMethodInfo, minfo_s_alloc);

  rb_define_singleton_method(mPLine, "each_minfo", pline_s_each_minfo, 0);
}

