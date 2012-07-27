# Boilerplate to pull shared object and helper routines.
require_relative '../ext/thread_frame'
require_relative 'iseq_extra'
RubyVM::ThreadFrame = RubyVM::Frame
class Thread
  # For compatibility with old stuff
  alias tracing tracing? 
end
