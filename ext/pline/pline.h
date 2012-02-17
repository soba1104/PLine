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


