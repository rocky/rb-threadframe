#!/usr/bin/env rake
# -*- Ruby -*-
require 'rubygems'
require 'rake/gempackagetask'
require 'rake/rdoctask'
require 'rake/testtask'

SO_NAME = "thread_frame.so"

PACKAGE_VERSION = open("ext/thread_frame.c") do |f| 
  f.grep(/^#define THREADFRAME_VERSION/).first[/"(.+)"/,1]
end

TEST_FILES = FileList['test/**/*.rb']

COMMON_FILES = FileList[
  'README',
  'Rakefile',
]                        

ALL_FILES = COMMON_FILES + TEST_FILES

desc "Create the core ruby-debug shared library extension"
task :ext do
  Dir.chdir("ext") do
    system("#{Gem.ruby} extconf.rb && make")
  end
end

desc 'Remove built files'
task :clean do
  cd 'ext' do
    if File.exists?('Makefile')
      sh 'make clean'
      rm  'Makefile'
    end
    derived_files = Dir.glob('.o') + Dir.glob('*.so')
    rm derived_files unless derived_files.empty?
  end
end

task :default => [:test]

desc 'Test units - the smaller tests'
task :'test:unit' => [:ext] do |t|
  Rake::TestTask.new(:'test:unit') do |t|
    t.libs << './ext'
    t.test_files = FileList['test/unit/**/*.rb']
    # t.pattern = 'test/**/*test-*.rb' # instead of above
    t.verbose = true
  end
end

desc 'Test everything - unit tests for now.'
task :test do
  exceptions = ['test:unit'].collect do |task|
    begin
      Rake::Task[task].invoke
      nil
    rescue => e
      e
    end
  end.compact
  
  exceptions.each {|e| puts e;puts e.backtrace }
  raise "Test failures" unless exceptions.empty?
end

desc "Test everything - same as test."
task :check => :test

# ---------  RDoc Documentation ------
desc 'Generate rdoc documentation'
Rake::RDocTask.new('rdoc') do |rdoc|
  rdoc.rdoc_dir = 'doc/rdoc'
  rdoc.title    = 'rb-threadframe'
  # Show source inline with line numbers
  rdoc.options << '--inline-source' << '--line-numbers'
  # Make the readme file the start page for the generated html
  rdoc.options << '--main' << 'README'
  rdoc.rdoc_files.include('ext/**/thread_frame.c',
                          'README')
end

# Base GEM Specification
default_spec = Gem::Specification.new do |spec|
  spec.name = "rb-threadframe"
  
  spec.homepage = "http://github.com/rocky/rb-threadframe/tree/master"
  spec.summary = "Frame introspection"
  spec.description = <<-EOF

rb-threadframe gives introspection access for frames of a thread.
EOF

  spec.version = PACKAGE_VERSION

  spec.author = "R. Bernstein"
  spec.email = "rocky@gnu.org"
  spec.platform = Gem::Platform::RUBY
  spec.files = ALL_FILES.to_a  

  spec.required_ruby_version = '>= 1.9.1'
  spec.date = Time.now
  # spec.rubyforge_project = 'rocky-hacks'
  
  # rdoc
  spec.has_rdoc = true
  spec.extra_rdoc_files = ['README', 'threadframe.rd']
end

# Rake task to build the default package
Rake::GemPackageTask.new(default_spec) do |pkg|
  pkg.need_tar = true
end
