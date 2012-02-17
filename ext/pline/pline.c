#include "pline.h"

static VALUE mPLine, cMethodInfo;
static st_table *pline_table;
static VALUE minfo_table;

#include "minfo.c"
#include "summarize.c"

static rb_iseq_t *pline_find_iseq(VALUE obj, VALUE mid, VALUE singleton_p)
{
  rb_method_entry_t *me;
  rb_method_definition_t *def;
  VALUE km;
  int class;
  const char *msg = NULL;
  VALUE name;
  const char *sep = RTEST(singleton_p) ? "." : "#";

  if (RTEST(singleton_p)) {
    class = 1;
    km = rb_class_of(obj);
  } else {
    VALUE c = rb_obj_class(obj);
    if (c == rb_cClass) {
      class = 1;
    } else if (c == rb_cModule) {
      class = 0;
    } else {
      rb_bug("pline_find_iseq: should not be reached(0)");
    }
    km = obj;
  }

  me = search_method(km, SYM2ID(mid));
  if (!me) {
    if (class) {
      name = rb_class_path(km);
    } else {
      name = rb_mod_name(km);
    }
    rb_raise(rb_eArgError, "method not found (%s%s%s)",
             RSTRING_PTR(name), sep, rb_id2name(SYM2ID(mid)));
  }

  def = me->def;
  switch (def->type) {
    case VM_METHOD_TYPE_ISEQ:
      return def->body.iseq;
    case VM_METHOD_TYPE_CFUNC:
      msg = "PLine cannot handle C method";
      break;
    case VM_METHOD_TYPE_ATTRSET:
      msg = "PLine cannot handle attr_writer";
      break;
    case VM_METHOD_TYPE_IVAR:
      msg = "PLine cannot handle attr_reader";
      break;
    case VM_METHOD_TYPE_BMETHOD:
      msg = "Currently, PLine cannot handle method defined by define_method";
      break;
    case VM_METHOD_TYPE_ZSUPER:
      msg = "Unsupported method type zsuper";
      break;
    case VM_METHOD_TYPE_UNDEF:
      msg = "Unsupported method type undef";
      break;
    case VM_METHOD_TYPE_NOTIMPLEMENTED:
      msg = "Unsupported method type notimplemented";
      break;
    case VM_METHOD_TYPE_OPTIMIZED:
      msg = "Unsupported method type optimized";
      break;
    case VM_METHOD_TYPE_MISSING:
      msg = "Unsupported method type missing";
      break;
    default:
      msg = NULL;
  }

  if (!msg) {
    rb_bug("pline_find_iseq: should not be reached(1)");
  }

  if (class) {
    name = rb_class_path(km);
  } else {
    name = rb_mod_name(km);
  }
  rb_raise(rb_eArgError, "%s (%s%s%s)",
           msg, RSTRING_PTR(name), sep, rb_id2name(SYM2ID(mid)));
}

static void pline_inject(rb_iseq_t *iseq)
{
  int idx, len = iseq->iseq_size;
  VALUE *seq = iseq->iseq;
  VALUE *enc = iseq->iseq_encoded;

  for (idx = 0; idx < len; idx += insn_len(seq[idx])) {
    VALUE insn = seq[idx];
    VALUE *op0 = &seq[idx] + 1;
    VALUE *op1 = &enc[idx] + 1;
    rb_num_t nf;
    rb_iseq_t *child = NULL;

    switch(insn) {
    case BIN(trace):
      nf = op0[0];
      if (nf == RUBY_EVENT_LINE || nf == RUBY_EVENT_RETURN) {
        op0[0] = RUBY_EVENT_END;
        op1[0] = RUBY_EVENT_END;
      }
      break;
    case BIN(putiseq):
      child = (rb_iseq_t *)op0[0];
      break;
    case BIN(defineclass):
      child = (rb_iseq_t *)op0[1];
      break;
    case BIN(send):
      child = (rb_iseq_t *)op0[2];
      break;
    case BIN(invokesuper):
      child = (rb_iseq_t *)op0[1];
      break;
    }
    if (child) {
      pline_inject(child);
    }
  }
}

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
  pline_src_info_t *info;
  int i;
  struct timespec tp;
  pline_time_t t;

  if (!success) return;

  if (!st_lookup(pline_table, (st_data_t)srcfile, (st_data_t*)&info)) {
    info = ALLOC(pline_src_info_t);
    info->vals = NULL;
    info->starts = NULL;
    info->size = 0;
    st_insert(pline_table, (st_data_t)srcfile, (st_data_t)info);
  }

  if (info->size < line) {
    if (info->starts && info->vals) {
      REALLOC_N(info->starts, pline_time_t, line);
      REALLOC_N(info->vals, pline_time_t, line);
    } else {
      info->starts = ALLOC_N(pline_time_t, line);
      info->vals = ALLOC_N(pline_time_t, line);
    }
    for (i = info->size; i < line; i++) {
      info->starts[i] = NOVALUE;
      info->vals[i] = 0;
    }
    info->size = line;
  }

  clock_gettime(CLOCK_MONOTONIC, &tp);
  t = ((pline_time_t)tp.tv_sec)*1000*1000*1000 + ((pline_time_t)tp.tv_nsec);
  info->starts[line2idx(line)] = t;
  for (i = line2idx(line) - 1; i >= 0; i--) {
    if (has_value(info->starts[i])) {
      pline_time_t s = info->starts[i];
      info->starts[i] = NOVALUE;
      info->vals[i] += t - s;
      break;
    }
  }

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
  rb_ary_push(minfo_table, minfo);

  return Qnil;
}

VALUE Init_pline()
{
  mPLine = rb_define_module("PLine");
  rb_define_singleton_method(mPLine, "profile", pline_m_profile, 3);
  pline_table = st_init_strtable();
  minfo_table = rb_ary_new();
  rb_gc_register_mark_object(minfo_table);

  pline_minfo_init();
  pline_summarize_init();
}

