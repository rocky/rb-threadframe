require 'test/unit'
require_relative File.join(%w(.. .. ext thread_frame))

class TestISeq < Test::Unit::TestCase
  def test_fields
    iseq = RubyVM::ThreadFrame::current.iseq
    assert iseq
    assert_equal(0, iseq.arity)
    assert_equal(-1, iseq.arg_block)
    assert_equal(0, iseq.argc)
    assert_equal(0, iseq.arg_opts)
    assert_equal(2, iseq.local_table_size)

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

  end
end
