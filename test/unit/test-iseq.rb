require 'test/unit'
require_relative File.join(%w(.. .. ext thread_frame))

class TestISeq < Test::Unit::TestCase

  class C
    def initialize(test_obj, optional=true)
      iseq = RubyVM::ThreadFrame::current.iseq
      test_obj.assert_equal('test_obj', iseq.local_name(0))
      test_obj.assert_equal(1, iseq.arity)
      test_obj.assert_equal(-1, iseq.arg_block)
      test_obj.assert_equal(1, iseq.argc)
    end
  end
  
  def test_fields
    iseq = RubyVM::ThreadFrame::current.iseq
    assert iseq
    assert_equal(0, iseq.arity)
    assert_equal(-1, iseq.arg_block)
    assert_equal(0, iseq.argc)
    assert_equal(0, iseq.arg_opts)
    assert_equal(2, iseq.local_table_size)
    assert_equal(nil, iseq.local_name(-10))

    x  = lambda do |x,y| 
      iseq = RubyVM::ThreadFrame::current.iseq
      assert iseq
      assert_equal(2, iseq.arity)
      assert_equal(-1, iseq.arg_block)
      assert_equal(2, iseq.argc)
      assert_equal(0, iseq.arg_opts)
      assert_equal(3, iseq.local_table_size)
      ['x', 'y'].each_with_index do |expect, i|
        assert_equal(expect, iseq.local_name(i))
      end
      assert_equal(nil, iseq.local_name(-1))
      assert_equal(nil, iseq.local_name(10))
    end
    x.call(1,2)

    x  = Proc.new do |a|
      iseq = RubyVM::ThreadFrame::current.iseq
      assert iseq
      assert_equal(1, iseq.arity)
      assert_equal(-1, iseq.arg_block)
      assert_equal(1, iseq.argc)
      assert_equal(0, iseq.arg_opts)
      assert_equal(1, iseq.local_table_size)
      ['a'].each_with_index do |expect, i|
        assert_equal(expect, iseq.local_name(i))
      end
      assert_equal(nil, iseq.local_name(100))
      assert_raises TypeError do 
        iseq.local_name('a')
      end
    end
    x.call(1,2)
    C.new(self, 5)
  end
end
