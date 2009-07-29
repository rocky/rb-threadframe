require "mkmf"

config_file = File.join(File.dirname(__FILE__), 'config_options')
load config_file if File.exist?(config_file)

# Temporary: to turn off optimization
# $CFLAGS='-fno-strict-aliasing -g -fPIC'
create_makefile("thread_frame")
