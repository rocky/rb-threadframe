require 'test/unit'
require_relative '../../ext/thread_frame'

class TestThread < Test::Unit::TestCase
  def test_stack_size_with_prev
    tf = RubyVM::ThreadFrame.new(Thread::current)

    # valid prev counts are -stack_size .. stack_size-1
    n = tf.stack_size
    assert_equal(nil, tf.prev(n), 
                 'Should have accessed past the top of the stack')
    top_frame = tf.prev(n-1)
    assert(top_frame, 
           'Should have gotten the top frame')
    assert_equal(tf, tf.prev(-n),
                 'tf.prev(tf.stack_size) == tf')
    assert(top_frame)
    assert_equal('TOP', top_frame.type,
                 'The type of the top frame should be "TOP"')
  end

  def test_prev

    assert RubyVM::ThreadFrame::prev(Thread::current, 0)
    assert(RubyVM::ThreadFrame::prev(Thread::current, 2),
           'There should be at least two prior frames')

    top_frame = RubyVM::ThreadFrame::prev(Thread::current, -1)
    assert(top_frame, 'Should give back the top frame')
    assert_equal('TOP', top_frame.type,
                 'The type of the top frame should be "TOP"')

    assert_equal(nil, RubyVM::ThreadFrame::prev(Thread::current, 1000))

    tf = RubyVM::ThreadFrame::current.prev

    assert tf.prev(2)
    assert_equal(tf,  tf.prev(0), 
                 'tf.prev(0) is defined as be tf')
    assert tf.prev(-1)
    assert_equal(nil, tf.prev(1000))

    assert_raises TypeError do
      tf.prev('a')
    end
    assert_raises TypeError do
      RubyVM::ThreadFrame::prev([1])
    end
    assert_raises TypeError do
      RubyVM::ThreadFrame::prev(RubyVM::ThreadFrame::current, [1])
    end

  end
end
