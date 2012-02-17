static st_table *pline_table;

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

static VALUE sinfo_find_force(char *s)
{
  VALUE sinfo = sinfo_find(s);

  if (!RTEST(sinfo)) {
    sinfo = rb_funcall(cSourceInfo, rb_intern("new"), 0);
    rb_gc_register_mark_object(sinfo);
    st_insert(pline_table, (st_data_t)s, (st_data_t)sinfo);
  }

  return sinfo;
}

static VALUE sinfo_s_find(VALUE self, VALUE path)
{
  VALUE sinfo;

  if (TYPE(path) != T_STRING) {
    rb_raise(rb_eArgError, "invalid argument");
  }

  return sinfo_find(RSTRING_PTR(path));
}

static void sinfo_measure(VALUE v, long line)
{
  pline_src_info_t *sinfo = DATA_PTR(v);
  struct timespec tp;
  pline_time_t t;
  long i;

  if (sinfo->size < line) {
    if (sinfo->starts && sinfo->vals) {
      REALLOC_N(sinfo->starts, pline_time_t, line);
      REALLOC_N(sinfo->vals, pline_time_t, line);
    } else {
      sinfo->starts = ALLOC_N(pline_time_t, line);
      sinfo->vals = ALLOC_N(pline_time_t, line);
    }
    for (i = sinfo->size; i < line; i++) {
      sinfo->starts[i] = NOVALUE;
      sinfo->vals[i] = 0;
    }
    sinfo->size = line;
  }

  clock_gettime(CLOCK_MONOTONIC, &tp);
  t = ((pline_time_t)tp.tv_sec)*1000*1000*1000 + ((pline_time_t)tp.tv_nsec);
  sinfo->starts[line2idx(line)] = t;
  for (i = line2idx(line) - 1; i >= 0; i--) {
    if (has_value(sinfo->starts[i])) {
      pline_time_t s = sinfo->starts[i];
      sinfo->starts[i] = NOVALUE;
      sinfo->vals[i] += t - s;
      break;
    }
  }
}

static void pline_sinfo_init(void)
{
  cSourceInfo = rb_define_class_under(mPLine, "SourceInfo", rb_cObject);
  rb_define_singleton_method(cSourceInfo, "find", sinfo_s_find, 1);
  rb_define_method(cSourceInfo, "lines", sinfo_m_lines, 0);
  rb_define_alloc_func(cSourceInfo, sinfo_s_alloc);
  pline_table = st_init_strtable();
}

