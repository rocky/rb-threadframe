require 'test/unit'
require_relative '../../ext/thread_frame'

class TestARGC < Test::Unit::TestCase

  def setup
    @original_compile_option = RubyVM::InstructionSequence.compile_option
    RubyVM::InstructionSequence.compile_option = {
      :trace_instruction => false,
      :specialized_instruction => false
    }
  end

  def teardown
    set_trace_func(nil)
    RubyVM::InstructionSequence.compile_option = @original_compile_option
  end

  def test_C_argc
    cmd='File.basename("/tmp/foo.rb");File.basename("/tmp/foo.rb",".rb")'
    iseq = RubyVM::InstructionSequence.compile(cmd)
    events = []
    all_events = []
    eval <<-EOF.gsub(/^.*?: /, "")
     1: set_trace_func(Proc.new { |event, file, lineno, mid, binding, klass|
     2:   tf = RubyVM::ThreadFrame.prev
     3:   all_events << [tf.argc, tf.arity, tf.type, mid]
     4:   if :basename == mid 
     5:     events << [tf.argc, tf.arity, tf.type, mid]
     6:   end
     7: })
     8: iseq.eval
     9: set_trace_func(nil)
    EOF
    # p all_events
    assert_equal([[1, -1, "CFUNC", :basename],  # 1-arg c-call
                  [1, -1, "CFUNC", :basename],  # 1-arg c-return
                  [2, -1, "CFUNC", :basename],  # 2-arg c-call
                  [2, -1, "CFUNC", :basename]   # 2-arg c-return
                 ], events)
  end
end

# We want to double-check we didn't mess up any pointers somewhere.
at_exit { GC.start  }
