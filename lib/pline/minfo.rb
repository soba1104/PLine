module PLine
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
end

