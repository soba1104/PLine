static st_table *pline_table;

typedef long long int pline_time_t;

/* pline_sinfo_container */
typedef struct pline_sinfo_container {
  st_table *sinfo_table;
} pline_sinfo_container_t;
static ID sinfo_container_id;

static void sinfo_container_mark(void *p)
{
  pline_sinfo_container_t *s = p;

  if (!s) return;
}

static void sinfo_container_free(void *p)
{
  pline_sinfo_container_t *s = p;

  if (!s) return;

  st_free_table(s->sinfo_table);
  xfree(s);
}

static const rb_data_type_t sinfo_container_data_type = {
  "pline_sinfo_container",
  {sinfo_container_mark, sinfo_container_free, NULL,},
};

static VALUE sinfo_container_s_alloc(VALUE klass)
{
  VALUE obj;
  pline_sinfo_container_t *s;

  obj = TypedData_Make_Struct(klass, pline_sinfo_container_t, &sinfo_container_data_type, s);
  s->sinfo_table = st_init_strtable();

  return obj;
}

static st_table* sinfo_container_current_sinfo_table(void)
{
  VALUE scon, thval = rb_thread_current();
  pline_sinfo_container_t *s;

  scon = rb_thread_local_aref(thval, sinfo_container_id);
  if (!RTEST(scon)) {
    scon = rb_funcall(cSourceInfoContainer, rb_intern("new"), 0);
    rb_thread_local_aset(thval, sinfo_container_id, scon);
  }
  s = DATA_PTR(scon);

  return s->sinfo_table;
}

/* pline_src_info */

typedef struct pline_line_info {
  pline_time_t score;
  pline_time_t start;
  pline_time_t prev;
} pline_line_info_t;

typedef struct pline_src_info {
  pline_line_info_t *lines;
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
  s->lines = NULL;
  s->size = 0;

  return obj;
}

static VALUE sinfo_m_lines(VALUE self)
{
  pline_src_info_t *s = DATA_PTR(self);
  VALUE lines = rb_ary_new2(s->size);
  int i;

  for (i = 0; i < s->size; i++) {
    rb_ary_push(lines, LL2NUM(s->lines[i].score));
  }

  return lines;
}

static VALUE sinfo_find(const char *s)
{
  VALUE sinfo;

  if (st_lookup(pline_table, (st_data_t)s, (st_data_t*)&sinfo)) {
    return sinfo;
  } else {
    return Qnil;
  }
}

static VALUE sinfo_find_force(const char *s)
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

static void __sinfo_measure(pline_src_info_t *sinfo, long line, struct timespec tp)
{
  pline_time_t t;
  long i, cidx, pidx;
  pline_line_info_t *clinfo;

  cidx = line2idx(line);
  clinfo = &sinfo->lines[cidx];
  pidx = clinfo->prev;

  if (pidx < 0) {
    for (i = cidx - 1; i >= 0; i--) {
      if (has_value(sinfo->lines[i].start)) {
        pidx = sinfo->lines[cidx].prev = i;
        break;
      }
    }
  }

  clinfo->start = ((pline_time_t)tp.tv_sec)*1000*1000*1000 + ((pline_time_t)tp.tv_nsec);
  if (pidx >= 0) {
    pline_line_info_t *plinfo = &sinfo->lines[pidx];
    if (has_value(plinfo->start)) {
      plinfo->score += (clinfo->start - plinfo->start);
      plinfo->start = NOVALUE;
    }
  }
}

static void sinfo_measure(VALUE v, long line)
{
  pline_src_info_t *sinfo = DATA_PTR(v);
  struct timespec tp;
  long i;

  if (sinfo->size < line) {
    if (sinfo->lines) {
      REALLOC_N(sinfo->lines, pline_line_info_t, line);
    } else {
      sinfo->lines = ALLOC_N(pline_line_info_t, line);
    }
    for (i = sinfo->size; i < line; i++) {
      pline_line_info_t *linfo = &sinfo->lines[i];
      linfo->score = 0;
      linfo->start = NOVALUE;
      linfo->prev  = -1;
    }
    sinfo->size = line;
  }

  clock_gettime(CLOCK_MONOTONIC, &tp);
  __sinfo_measure(sinfo, line, tp);
}

static void pline_sinfo_init(void)
{
  cSourceInfo = rb_define_class_under(mPLine, "SourceInfo", rb_cObject);
  rb_define_singleton_method(cSourceInfo, "find", sinfo_s_find, 1);
  rb_define_method(cSourceInfo, "lines", sinfo_m_lines, 0);
  rb_define_alloc_func(cSourceInfo, sinfo_s_alloc);
  cSourceInfoContainer = rb_define_class_under(cSourceInfo, "SourceInfoContainer", rb_cObject);
  rb_define_alloc_func(cSourceInfoContainer, sinfo_container_s_alloc);
  sinfo_container_id = rb_intern("__pline_sinfo_container");
  pline_table = st_init_strtable();
}

