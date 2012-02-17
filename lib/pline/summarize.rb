module PLine
  module Summarize
    DEFAULT_OUTPUT = STDERR
    @@output = DEFAULT_OUTPUT
    def output=(io)
      @@output = io
    end

    def build_message(desc, labels, contents, pretty = true)
      results = []
      l_desc = desc.length
      column_size = labels.size
      contents = contents.inject([]) do |ary, c|
        bug() unless c.size == column_size
        c.map! do |v|
          v.split("\n")
        end
        max = c.inject(0) do |m, v|
          l = v.size
          m > l ? m : l
        end
        max.times do |i|
          ary << c.map{|v| v[i] || ''}
        end
        ary
      end
      l_labels = contents.inject(labels.map{|t| t.length}) do |a0, a1|
        bug() unless a0.size == a1.size
        a0.zip(a1).map do |(v0, v1)|
          length = v1.length
          v0 > length ? v0 : length
        end
      end
      title = labels.zip(l_labels).map{|(t, l)| t.center(l)}.join(" | ")
      l_title = title.length
      width = l_desc > l_title ? l_desc : l_title
      if width != l_title
        bonus = width - l_title
        adjust = column_size - bonus % column_size
        width += adjust
        bonus += adjust
        bonus /= column_size
        l_labels.map!{|l| l + bonus}
        title = labels.zip(l_labels).map{|(t, l)| t.center(l)}.join(" | ")
      end
      width += 2
      sep = "-" * width
      results << " #{sep} "
      results << "|#{desc.center(width)}|"
      results << "|#{sep}|"
      results << "|#{title.center(width)}|"
      results << "|#{sep}|"
      if pretty
        side = "|"
      else
        side = ""
      end
      contents.each do |line|
        results << "#{side} #{line.zip(l_labels).map{|c, l| pretty ? c.ljust(l) : c }.join(" | ")} #{side}"
      end
      results << " #{sep} "
      results.map{|s| s.force_encoding('ASCII-8BIT')}.join("\n")
    end

    def summarize()
      files = {}
      MethodInfo.each do |m|
        bug() unless m.is_a?(MethodInfo)
        minfos = files[m.spath] ||= []
        minfos << m
      end
      labels = ["Line", "Time(usec)", "Source"]
      files.each do |spath, minfos|
        sinfo = SourceInfo.find(spath)
        source = File.readlines(spath)
        minfos.each do |m|
          contents = []
          sinfo.lines[(m.sline - 1)..(m.eline - 1)].each_with_index do |t, idx|
            line = m.sline + idx
            contents << [line.to_s, (t / 1000).to_s, source[line - 1]]
          end
          @@output.puts()
          desc = "   #{m.description}   "
          msg = build_message(desc, labels, contents)
          @@output.puts(msg)
        end
      end
    end
  end
end

