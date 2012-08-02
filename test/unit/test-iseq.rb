require 'test/unit'
require_relative '../../ext/thread_frame' if '1.9.2' == RUBY_VERSION

class TestISeq < Test::Unit::TestCase

  class C
    def initialize(test_obj, optional=true)
      iseq = RubyVM::Frame::current.iseq
      test_obj.assert_equal('test_obj', iseq.local_name(0))
      test_obj.assert_equal(1, iseq.arity)
      test_obj.assert_equal(-1, iseq.arg_block)
      test_obj.assert_equal(1, iseq.argc)
    end
  end
  
  def test_fields
    start_lineno = __LINE__ - 1;
    iseq = RubyVM::Frame::current.iseq
    assert iseq
    assert_equal('test_fields', iseq.name)
    ## FIXME: Why does this fail? 
    ## assert_equal(start_lineno, iseq.lineno, 'iseq.lineno')
    assert_equal(0, iseq.arity)
    assert_equal(-1, iseq.arg_block)
    assert_equal(0, iseq.argc)
    assert_equal(0, iseq.arg_opts)
    assert_equal(4, iseq.local_table_size)
    x  = lambda do |x,y| 
      iseq = RubyVM::Frame::current.iseq
      assert iseq
      assert_equal(2, iseq.arity)
      assert_equal(-1, iseq.arg_block)
      assert_equal(2, iseq.argc)
      assert_equal(0, iseq.arg_opts)
      assert_equal(3, iseq.local_table_size)
      ['x', 'y'].each_with_index do |expect, i|
        assert_equal(expect, iseq.local_name(i))
      end
      
      assert_equal('x', iseq.local_name(-1))
      assert_raise IndexError do
        x = iseq.local_name(10)
      end
    end
    x.call(1,2)

    x  = Proc.new do |a|
      iseq = RubyVM::Frame::current.iseq
      assert iseq
      assert_equal(1, iseq.arity)
      assert_equal(-1, iseq.arg_block)
      assert_equal(1, iseq.argc)
      assert_equal(0, iseq.arg_opts)
      assert_equal(1, iseq.local_table_size)
      ['a'].each_with_index do |expect, i|
        assert_equal(expect, iseq.local_name(i))
      end
      assert_raises IndexError do
        x = iseq.local_name(100)
      end
      assert_raises TypeError do 
        p iseq.local_name('a')
      end
    end
    x.call(1,2)
    C.new(self, 5)
    end_lineno = __LINE__ + 3
    assert_equal((start_lineno..end_lineno), 
                 method(:test_fields).iseq.line_range, 'line range')
  end

  def test_iseq_equal
    tf = RubyVM::Frame.current
    tf2 = RubyVM::Frame.current
    while !tf.iseq do
      tf = tf.prev
      tf2 = tf2.prev
    end
    assert_equal(false, tf.iseq.equal?(nil))
    assert_equal(true,  tf.iseq.equal?(tf.iseq))
    assert_equal(true,  tf.iseq.equal?(tf2.iseq))
    tf2 = tf2.prev 
    while !tf2.iseq do tf2 = tf2.prev end
    assert_equal(false, tf.iseq.equal?(tf2.iseq))
    assert_raises TypeError do
      tf.iseq.equal?(tf)
    end
  end

  # FIXME: killcache interface will probably change. Try make less sensitive
  # to compile sequence
  def test_iseq_killcache
    skip "killcache doesn't work on 1.9.3" if '1.9.3' == RUBY_VERSION
    iseq = RubyVM::Frame.current.iseq
    count = iseq.killcache
    if 0 != count
      assert_equal(0, iseq.killcache, 
                 'Doing killcache a second time should do nothing')
    end
  end

  def test_offsetlines
    start     = __LINE__ - 1
    tf        = RubyVM::Frame::current
    iseq      = tf.iseq
    offlines  = iseq.offsetlines
    pc        = tf.pc_offset
    assert_equal(__LINE__, offlines[pc][0]+1)
    offlines.values.each do |value|
      assert(value[0] >= start, "#{value[0]} should be not less than starting line #{start}")
      # Rough count of # of lines is less than 20
      assert(value[0] < start + 20, "#{value[0]} should be less than starting line #{start}")
    end
    offlines.keys.each do |offset|
      assert_equal offlines[offset][0], iseq.offset2lines(offset)[0]
    end
  end

end

# We want to double-check we didn't mess up any pointers somewhere.
at_exit { GC.start  }
