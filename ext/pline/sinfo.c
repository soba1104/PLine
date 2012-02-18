static st_table *scoring_sinfo_table;

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

static void sinfo_mark(void *p)
{
  pline_src_info_t *s = p;

  if (!s) return;
}

static void sinfo_free(void *p)
{
  pline_src_info_t *s = p;

  if (!s) return;

  if (s->lines) {
    xfree(s->lines);
  }
  xfree(s);
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

static VALUE sinfo_find(st_table *sinfo_table, const char *s)
{
  VALUE sinfo;

  if (st_lookup(sinfo_table, (st_data_t)s, (st_data_t*)&sinfo)) {
    return sinfo;
  } else {
    return Qnil;
  }
}

static VALUE sinfo_find_process_sinfo_force(const char *s)
{
  VALUE sinfo = sinfo_find(scoring_sinfo_table, s);

  if (!RTEST(sinfo)) {
    sinfo = rb_funcall(cSourceInfo, rb_intern("new"), 0);
    rb_gc_register_mark_object(sinfo);
    st_insert(scoring_sinfo_table, (st_data_t)s, (st_data_t)sinfo);
  }

  return sinfo;
}

static VALUE sinfo_find_scoring_sinfo_force(const char *s)
{
  VALUE sinfo = sinfo_find(scoring_sinfo_table, s);

  if (!RTEST(sinfo)) {
    sinfo = rb_funcall(cSourceInfo, rb_intern("new"), 0);
    rb_gc_register_mark_object(sinfo);
    st_insert(scoring_sinfo_table, (st_data_t)s, (st_data_t)sinfo);
  }

  return sinfo;
}

static VALUE sinfo_s_find(VALUE self, VALUE path)
{
  VALUE sinfo;

  if (TYPE(path) != T_STRING) {
    rb_raise(rb_eArgError, "invalid argument");
  }

  return sinfo_find(scoring_sinfo_table, RSTRING_PTR(path));
}

static void sinfo_expand(pline_src_info_t *sinfo, long line)
{
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
}

static void __sinfo_measure(pline_src_info_t *scoring_sinfo, pline_src_info_t *process_sinfo, long line, struct timespec tp)
{
  pline_time_t t;
  long i, cidx, pidx;
  pline_line_info_t *clinfo;

  cidx = line2idx(line);
  clinfo = &process_sinfo->lines[cidx];
  pidx = clinfo->prev;

  if (pidx < 0) {
    for (i = cidx - 1; i >= 0; i--) {
      if (has_value(process_sinfo->lines[i].start)) {
        pidx = process_sinfo->lines[cidx].prev = i;
        break;
      }
    }
  }

  clinfo->start = ((pline_time_t)tp.tv_sec)*1000*1000*1000 + ((pline_time_t)tp.tv_nsec);
  if (pidx >= 0) {
    pline_line_info_t *plinfo = &process_sinfo->lines[pidx];
    if (has_value(plinfo->start)) {
      plinfo->score += (clinfo->start - plinfo->start);
      plinfo->start = NOVALUE;
    }
  }
}

static void sinfo_measure(const char *srcfile, long line)
{
  VALUE sv = sinfo_find_scoring_sinfo_force(srcfile);
  VALUE pv = sinfo_find_process_sinfo_force(srcfile);
  pline_src_info_t *scoring_sinfo = DATA_PTR(sv);
  pline_src_info_t *process_sinfo = DATA_PTR(pv);
  struct timespec tp;

  sinfo_expand(scoring_sinfo, line);
  sinfo_expand(process_sinfo, line);

  clock_gettime(CLOCK_MONOTONIC, &tp);
  __sinfo_measure(scoring_sinfo, process_sinfo, line, tp);
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
  scoring_sinfo_table = st_init_strtable();
}

