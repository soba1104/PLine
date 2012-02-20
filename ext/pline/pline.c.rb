require "erb"

case RUBY_VERSION
when "1.9.2"
  ruby_srcdir = "ruby_source/1.9.2"
when "1.9.3"
  ruby_srcdir = "ruby_source/1.9.3"
else
  raise("Unsupported ruby version #{RUBY_VERSION}")
end

ERB.new(DATA.read, 0, "%-").run()
__END__

#include <ruby.h>

#include "<%= ruby_srcdir %>/vm_core.h"
#include "<%= ruby_srcdir %>/eval_intern.h"
#include "<%= ruby_srcdir %>/iseq.h"
#include "<%= ruby_srcdir %>/gc.h"
#include <ruby/vm.h>
#include <ruby/encoding.h>
#include "<%= ruby_srcdir %>/vm_insnhelper.h"
#include "<%= ruby_srcdir %>/vm_insnhelper.c"
#include "<%= ruby_srcdir %>/vm_exec.h"

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

#include "<%= ruby_srcdir %>/insns_info.inc"
#include "<%= ruby_srcdir %>/manual_update.h"

#ifndef _WIN32
#include <time.h>
#include <sys/time.h>
#endif

#define line2idx(l) ((l) - 1)
#define idx2line(i) ((i) + 1)
#define nano2micro(t) (((t) / 1000))
#define NOVALUE 0
#define has_value(v) ((v) != NOVALUE)

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

