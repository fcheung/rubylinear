Gem::Specification.new do |s|
  s.name = %q{ruby_linear}
  s.version = "0.1.0"

  s.required_rubygems_version = Gem::Requirement.new(">= 0") if s.respond_to? :required_rubygems_version=
  s.authors = ["Frederick Cheung"]
  s.date = %q{2012-02-29}
  s.description = %q{Ruby wrapper for LIBLINEAR }
  s.email = %q{frederick.cheung@gmail.com}
  s.extensions = ["ext/extconf.rb"]
  s.extra_rdoc_files = ["README"]
  s.files = ["COPYING", "AUTHORS", "README.markdown", "Rakefile"]
  s.files += Dir["ext/*.h"]
  s.files += Dir["ext/*.cpp"]
  s.files += Dir["ext/*.c"]
  s.files += Dir["lib/*.rb"]
  s.files += Dir["spec/**/*"]
  
  s.license = 'MIT'
  s.has_rdoc = false
  s.homepage = %q{http://spacevatican.org}
  s.require_paths = ["lib"]
  s.rubygems_version = %q{1.8.10}
  s.summary = %q{Ruby wrapper for LIBLINEAR}
  
  s.required_ruby_version = '>= 1.9.2'
end

