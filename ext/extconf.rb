require 'mkmf'

fail "You need to install a threadframe-patched Ruby.
See http://github.com/rocky/rb-threadframe/wiki/How-to-Install" unless 
  RbConfig::CONFIG.member?('rb-threadframe')

fail "No C extensions get compiled for 1.9.3; it's done now inside Ruby" if 
  '1.9.3' == RUBY_VERSION

# Allow use customization of compile options. For example, the 
# following lines could be put in config_options:
# CONFIG['optflags'] = ''   # Or -O3
# CONFIG['debugflags'] = '-g3 -ggdb' 
config_file = File.join(File.dirname(__FILE__), 'config_options.rb')
load config_file if File.exist?(config_file)

create_makefile("thread_frame", '1.9.2')
