require 'pline.so'
require 'pline/util'
require 'pline/minfo'
require 'pline/summarize'

module PLine
  extend PLine::Util
  extend PLine::Summarize
end

at_exit{PLine.summarize()}

