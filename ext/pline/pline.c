#include <ruby.h>

#include "ruby_source/1.9.3/vm_core.h"
#include "ruby_source/1.9.3/eval_intern.h"
#include "ruby_source/1.9.3/iseq.h"
#include "ruby_source/1.9.3/gc.h"
#include <ruby/vm.h>
#include <ruby/encoding.h>
#include "ruby_source/1.9.3/vm_insnhelper.h"
#include "ruby_source/1.9.3/vm_insnhelper.c"
#include "ruby_source/1.9.3/vm_exec.h"

#define TRUE 1
#define FALSE 0

#ifdef  USE_INSN_STACK_INCREASE
#undef  USE_INSN_STACK_INCREASE
#endif
#define USE_INSN_STACK_INCREASE 1

#ifdef USE_INSN_RET_NUM
#undef USE_INSN_RET_NUM
#endif
#define USE_INSN_RET_NUM 1

#include "ruby_source/1.9.3/insns_info.inc"
#include "ruby_source/1.9.3/manual_update.h"

#ifndef _WIN32
#include <time.h>
#include <sys/time.h>
#endif

#define line2idx(l) ((l) - 1)
#define idx2line(i) ((i) + 1)
#define nano2micro(t) (((t) / 1000))
#define has_value(v) (v > 0)
#define NOVALUE -1

static VALUE mPLine, cSourceInfo, cMethodInfo, cSourceInfoContainer;

#include "iseq.c"
#include "sinfo.c"
#include "minfo.c"
#include "profiler.c"

VALUE Init_pline()
{
  mPLine = rb_define_module("PLine");
  pline_sinfo_init();
  pline_minfo_init();
  pline_profiler_init();
}

