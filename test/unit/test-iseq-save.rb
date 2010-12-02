require 'test/unit'
require_relative '../../ext/thread_frame'

class TestISeqSave < Test::Unit::TestCase

  def test_ISEQS__
    Object.const_set("ISEQS__", {})
    eval "def five; 5 end"
    iseq = Object.const_get('ISEQS__')['five'][0]
    assert_equal RubyVM::InstructionSequence, iseq.class
    assert_equal Hash, iseq.compile_options.class
    old_verbose = $VERBOSE
    $VERBOSE = nil
    Object.const_set("ISEQS__", nil)
    $VERBOSE = old_verbose
  end
end

# We want to double-check we didn't mess up any pointers somewhere.
at_exit { GC.start  }
