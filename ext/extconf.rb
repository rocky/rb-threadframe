require "mkmf"
# Temporary: to turn off optimization
# $CFLAGS='-fno-strict-aliasing -g -fPIC'
create_makefile("thread_frame")
