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
      rb_raise(rb_eArgError, "expected class or module");
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

