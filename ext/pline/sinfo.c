typedef long long int pline_time_t;
typedef struct pline_src_info {
  pline_time_t *vals;
  pline_time_t *starts;
  long size;
} pline_src_info_t;

static void sinfo_mark(void *ptr)
{
  pline_src_info_t *s = ptr;

  if (!s) return;
}

static void sinfo_free(void *p)
{
  if (p) xfree(p);
}

static const rb_data_type_t sinfo_data_type = {
  "pline_src_info",
  {sinfo_mark, sinfo_free, NULL,},
};

static VALUE sinfo_s_alloc(VALUE klass)
{
  VALUE obj;
  pline_src_info_t *s;

  obj = TypedData_Make_Struct(klass, pline_src_info_t, &sinfo_data_type, s);
  s->vals = NULL;
  s->starts = NULL;
  s->size = 0;

  return obj;
}

static VALUE sinfo_m_lines(VALUE self)
{
  pline_src_info_t *s = DATA_PTR(self);
  VALUE lines = rb_ary_new2(s->size);
  int i;

  for (i = 0; i < s->size; i++) {
    RARRAY_PTR(lines)[i] = LL2NUM(s->vals[i]);
  }

  return lines;
}

static VALUE sinfo_find(char *s)
{
  VALUE sinfo;

  if (st_lookup(pline_table, (st_data_t)s, (st_data_t*)&sinfo)) {
    return sinfo;
  } else {
    return Qnil;
  }
}

static VALUE sinfo_s_find(VALUE self, VALUE path)
{
  VALUE sinfo;

  if (TYPE(path) != T_STRING) {
    rb_raise(rb_eArgError, "invalid argument");
  }

  return sinfo_find(RSTRING_PTR(path));
}

static void pline_sinfo_init(void)
{
  cSourceInfo = rb_define_class_under(mPLine, "SourceInfo", rb_cObject);
  rb_define_method(cSourceInfo, "lines", sinfo_m_lines, 0);
  rb_define_alloc_func(cSourceInfo, sinfo_s_alloc);
}

