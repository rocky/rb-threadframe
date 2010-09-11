require "mkmf"

fail "You need to install a threadframe-patched Ruby.
See http://github.com/rocky/rb-threadframe/wiki/How-to-Install" unless 
  RbConfig::CONFIG.member?('rb-threadframe')

config_file = File.join(File.dirname(__FILE__), 'config_options')
load config_file if File.exist?(config_file)

# Temporary: to turn off optimization
# $CFLAGS='-fno-strict-aliasing -g -fPIC'
create_makefile("thread_frame")
