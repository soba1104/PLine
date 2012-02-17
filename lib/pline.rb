require 'pline.so'
require 'pline/util'

module PLine
  extend PLine::Util

  def self.source(path)
    todo() unless File.exist?(path)
    File.readlines(path)
  end

  def self.summarize_line(src, line, time)
    bug() unless src.length >= (line - 1)
    sprintf("%5d: %12d: %s", line, time, src[line - 1].chomp)
  end

  class MethodInfo
    def description()
      "#{obj}#{singleton? ? '.' : '#'}#{mid}: #{spath}(#{sline} - #{eline})"
    end
  end

  at_exit(&lambda{
    result = summarize()
    return if result.empty?
    STDERR.puts(result.join("\n"))

    each_minfo do |m|
      bug() unless m.is_a?(MethodInfo)
      puts m.description
    end
  })
end

