# Boilerplate to pull shared object and helper routines.
if RUBY_VERSION == '1.9.2'
  require_relative '../ext/thread_frame'
end
require_relative 'iseq_extra'

RubyVM::ThreadFrame = RubyVM::Frame
class Thread
  # For compatibility with old stuff
  alias tracing tracing? 
end
