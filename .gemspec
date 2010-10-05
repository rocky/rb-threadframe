# -*- Ruby -*-
# -*- encoding: utf-8 -*-
require 'rake'
require 'rubygems' unless 
  Object.const_defined?(:Gem)

PACKAGE_VERSION = open("ext/thread_frame.c") do |f| 
  f.grep(/^#define THREADFRAME_VERSION/).first[/"(.+)"/,1]
end

EXT_FILES     = FileList[%w(ext/*.c ext/*.h)]
INCLUDE_FILES = FileList['include/*.h']
LIB_FILES     = FileList['lib/*.rb']
TEST_FILES    = FileList['test/**/*.rb']
COMMON_FILES  = FileList[%w(README.md Rakefile Makefile LICENSE NEWS)]
FILES         = COMMON_FILES + INCLUDE_FILES + LIB_FILES + EXT_FILES + 
  TEST_FILES

Gem::Specification.new do |spec|
  spec.authors      = ['R. Bernstein']
  spec.date         = Time.now
  spec.description = <<-EOF

rb-threadframe gives introspection access for frames of a thread.
EOF
  spec.email        = 'rockyb@rubyforge.net'
  spec.extensions   = ["ext/extconf.rb"]
  spec.files        = FILES.to_a  
  spec.has_rdoc     = false
  spec.homepage     = "http://github.com/rocky/rb-threadframe/tree/master"
  spec.name         = "rb-threadframe"
  spec.license      = 'MIT'
  spec.platform     = Gem::Platform::RUBY
  spec.require_path = 'lib'
  spec.required_ruby_version = '= 1.9.2'
  spec.summary      = "Frame introspection"

  spec.version      = PACKAGE_VERSION
  spec.extra_rdoc_files = ['README.md', 'threadframe.rd']

  # Make the readme file the start page for the generated html
  spec.rdoc_options += ['--main', 'README.md']
  spec.rdoc_options += ['--title', "ThreadFrame #{PACKAGE_VERSION} Documentation"]

end
