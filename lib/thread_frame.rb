# Boilerplate to pull shared object and helper routines.
require_relative 'iseq_extra'

class RubyVM::Frame
  VERSION = '0.50'
end

# RubyVM::ThreadFrame = RubyVM::Frame
# class Thread
#   # For compatibility with old stuff
#   alias tracing tracing?
# end
