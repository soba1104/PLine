# coding=utf-8

require 'mkmf'
require 'rbconfig'
extend RbConfig

$defs.push '-DCABI_OPERANDS' if enable_config 'cabi-operands', true
$defs.push '-DCABI_PASS_CFP' if enable_config 'cabi-pass-cfp', true

$INCFLAGS << ' -I$(srcdir)/ruby_source'
$objs = %w'$(srcdir)/pline.o'
create_header
create_makefile 'pline'

