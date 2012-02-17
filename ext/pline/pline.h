#include <ruby.h>

#include <time.h>
#include <sys/time.h>

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

#define line2idx(l) ((l) - 1)
#define idx2line(i) ((i) + 1)
#define nano2micro(t) (((t) / 1000))
#define has_value(v) (v > 0)
#define NOVALUE -1

typedef long long int pline_time_t;
typedef struct pline_src_info {
  pline_time_t *vals;
  pline_time_t *starts;
  long size;
} pline_src_info_t;

