Gem::Specification.new do |spec|
  spec.name		      = "pline"
  spec.version		      = "0.0.1"
  spec.platform		      = Gem::Platform::RUBY
  spec.summary		      = "Performance Profiler for Ruby1.9.3"
  spec.description	      = <<-EOS
PLine is a performance profiler for Ruby1.9.3.

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
  #spec.required_ruby_version  = '= 1.9.3'
  spec.required_ruby_version  = '>= 1.9.3'
end

