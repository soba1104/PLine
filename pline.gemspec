Gem::Specification.new do |spec|
  spec.name		      = "pline"
  spec.version		      = "0.0.3"
  spec.platform		      = Gem::Platform::RUBY
  spec.summary		      = "Performance Profiler for Ruby1.9.3 and Ruby1.9.2"
  spec.description	      = <<-EOS
PLine is a profiler for Ruby1.9.3 and Ruby1.9.2.
PLine profiles each line of Ruby method (method written in Ruby) you specified.
Using PLine, you can profile each line of Ruby method easily.
  EOS
  spec.files		      = Dir['{lib/**/*,ext/**/*}'] + %w[
				  pline.gemspec
                                  README
				]
  #spec.bindir		      = 'bin'
  #spec.executables	      << 'pline'
  spec.require_path	      = 'lib'
  spec.extensions	      = 'ext/pline/extconf.rb'
  spec.has_rdoc		      = false
  spec.extra_rdoc_files      = ['README']
  #spec.test_files	      = Dir['test/*']
  spec.author		      = 'Satoshi Shiba'
  spec.email		      = 'shiba@rvm.jp'
  spec.homepage		      = 'https://github.com/soba1104/PLine'
  #spec.rubyforge_project     = 'pline'
  spec.required_ruby_version  = '>= 1.9.2'
end

