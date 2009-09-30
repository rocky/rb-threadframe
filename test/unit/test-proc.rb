# Test of additional proc and method
require 'test/unit'
require_relative %w(.. .. ext thread_frame)

class TestProcAndMethod < Test::Unit::TestCase
  def test_proc_iseq
    assert_equal(true, 
                 Proc.new{|x,y| x+y}.iseq.is_a?(RubyVM::InstructionSequence))
  end
  def test_method_extra
    m = self.method :test_method_extra
    assert_equal(1, m.alias_count)
    assert_equal(:test_method_extra, m.original_id)
    self.instance_eval { assert_equal(1, m.alias_count) }
    assert_equal(1, m.alias_count)
    self.instance_eval { alias :two :test_method_extra }
    assert_equal(2, m.alias_count)
    assert_equal(3, self.method(:test_method_extra).alias_count)
    assert_equal(3, m.alias_count)
    assert_equal(4, self.method(:two).alias_count)
    assert_equal(:test_method_extra, self.method(:two).original_id)
  end
end
