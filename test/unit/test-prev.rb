require 'test/unit'
require_relative File.join(%w(.. .. ext thread_frame))

class TestThread < Test::Unit::TestCase
  def test_stack_size_with_prev
    tf = RubyVM::ThreadFrame.new(Thread::current)
    n = tf.stack_size
    assert_equal(nil, tf.prev(n), 
                 'Should have accessed past the top of the stack')
    top_frame = tf.prev(n-1)
    assert(top_frame, 
           'Should have gotten the top frame')
    assert(top_frame)
    assert_equal('TOP', top_frame.type,
                 'The type of the top frame should be "TOP"')
  end

  def test_prev
    tf = RubyVM::ThreadFrame::current.prev

    # Test prev with an argument count
    assert tf.prev(2)
    assert_equal(nil, tf.prev(-1))
    assert_equal(nil, tf.prev(1000))

  end
end
