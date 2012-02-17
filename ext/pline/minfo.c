typedef struct pline_method_info {
  VALUE spath;
  VALUE sline;
  VALUE eline;
} pline_method_info_t;

static void minfo_mark(void *ptr)
{
  pline_method_info_t *m = ptr;

  if (!m) return;
  rb_gc_mark(m->spath);
  rb_gc_mark(m->sline);
  rb_gc_mark(m->eline);
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

static VALUE minfo_m_init(VALUE self, VALUE spath, VALUE sline, VALUE eline)
{
  pline_method_info_t *m = DATA_PTR(self);

  if (rb_obj_is_kind_of(spath, rb_cString)  != Qtrue ||
      rb_obj_is_kind_of(sline, rb_cInteger) != Qtrue ||
      rb_obj_is_kind_of(eline, rb_cInteger) != Qtrue) {
    rb_raise(rb_eArgError, "invalid arguments");
  }

  m->spath = spath;
  m->sline = sline;
  m->eline = eline;

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

VALUE pline_method_info_init(VALUE mPLine)
{
  VALUE cMethodInfo = rb_define_class_under(mPLine, "MethodInfo", rb_cObject);

  rb_define_method(cMethodInfo, "spath", minfo_m_spath, 0);
  rb_define_method(cMethodInfo, "sline", minfo_m_sline, 0);
  rb_define_method(cMethodInfo, "eline", minfo_m_eline, 0);
  rb_define_method(cMethodInfo, "initialize", minfo_m_init, 3);
  rb_define_alloc_func(cMethodInfo, minfo_s_alloc);

  return cMethodInfo;
}

