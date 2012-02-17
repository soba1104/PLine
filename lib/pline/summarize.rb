module PLine
  module Summarize
    DEFAULT_OUTPUT = STDERR
    @@output = DEFAULT_OUTPUT
    def output=(io)
      @@output = io
    end

    def summarize()
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
          @@output.puts()
          @@output.puts("========== #{m.description} ==========")
          @@output.puts("line |    usec     | source")
          sinfo.lines[(m.sline - 1)..(m.eline - 1)].each_with_index do |t, idx|
            line = m.sline + idx
            @@output.print(sprintf("%5d: %12d: %s", line, t / 1000, source[line - 1]))
          end
        end
      end
    end
  end
end
