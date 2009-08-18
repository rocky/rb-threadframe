require 'test/unit'
require_relative %w(.. .. ext thread_frame)

class TestProc < Test::Unit::TestCase
  def test_basic
    assert_equal(true, 
                 Proc.new{|x,y| x+y}.iseq.is_a?(RubyVM::InstructionSequence))
  end
end
