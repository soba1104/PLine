require 'pline.so'
require 'pline/util'

module PLine
  extend PLine::Util

  class MethodInfo
    ALL = []

    def description()
      "#{obj}#{singleton? ? '.' : '#'}#{mid}: #{spath}(#{sline} - #{eline})"
    end

    def self.register(m)
      bug() unless m.is_a?(MethodInfo)
      ALL << m
    end

    def self.each()
      ALL.each do |m|
        bug() unless m.is_a?(MethodInfo)
        yield(m)
      end
    end
  end

  def self.summarize()
    files = {}
    MethodInfo.each do |m|
      bug() unless m.is_a?(MethodInfo)
      minfos = files[m.spath] ||= []
      minfos << m
    end
    files.each do |spath, minfos|
      sinfo = SourceInfo.find(spath)
      source = File.readlines(spath)
      minfos.each do |m|
        puts("========== #{m.description} ==========")
        sinfo.lines[(m.sline - 1)..(m.eline - 1)].each_with_index do |t, idx|
          line = m.sline + idx
          print(sprintf("%5d: %12d: %s", line, t / 1000, source[line - 1]))
        end
        puts
      end
    end
  end

  at_exit{summarize()}
end

