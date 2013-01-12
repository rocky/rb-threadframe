# -*- Ruby -*-
# -*- encoding: utf-8 -*-
require 'rake'
require 'rubygems' unless 
  Object.const_defined?(:Gem)


PACKAGE_VERSION = open('ext/version.h') do |f| 
  f.grep(/^#define THREADFRAME_VERSION/).first[/"(.+)"/,1]
end

EXT_FILES     = FileList[%W(ext/#{RUBY_VERSION}/*.c 
                            ext/version.h
                            ext/#{RUBY_VERSION}/*.h)] 
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
A set of patches to Ruby MRI 1.9.3 and 1.9.2 that adds run-time introspection, a call frame object, and other run-time support for things like debuggers.

For MRI 1.9.2, there are additional routines are in a C extension. For MRI 1.9.3, everthing is in a patched Ruby. Necessary patches and some simple patch code are found in this repository though. See https://github.com/rocky/rb-threadframe/wiki/How-to-Install for how to install.
EOF
  spec.email        = 'rockyb@rubyforge.net'
  spec.files        = FILES.to_a  
  spec.has_rdoc     = false
  spec.homepage     = 'http://github.com/rocky/rb-threadframe/tree/master'
  spec.name         = 'rb-threadframe'
  spec.license      = 'MIT'
  spec.platform     = Gem::Platform::RUBY
  spec.require_path = 'lib'
  # spec.required_ruby_version = '~> 1.9.2frame'
  spec.summary      = 'Call stack introspection and run-time support for debuggers'

  spec.version      = PACKAGE_VERSION
  spec.extra_rdoc_files = ['README.md', 'threadframe.rd']

  # Make the readme file the start page for the generated html
  spec.rdoc_options += ['--main', 'README.md']
  spec.rdoc_options += ['--title', "ThreadFrame #{PACKAGE_VERSION} Documentation"]

end
