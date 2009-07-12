require 'test/unit'
require_relative File.join('..', '..', 'ext', 'thread_frame')

class TestThread < Test::Unit::TestCase
  def test_basic
    assert_equal(RubyVM::ThreadFrame.new(Thread::current).thread, 
                 RubyVM::ThreadFrame::current.thread)
    assert_equal(RubyVM::ThreadFrame.new(Thread::current).thread, 
                 Thread::current)
    assert_equal(Thread::current.threadframe.thread, Thread::current)
    tf = RubyVM::ThreadFrame::current
    pc1 = tf.pc_offset
    assert(pc1 > 0, 'Should be able to get a valid PC offset')
    # pc_offset is dynamic - it changes constantly
    pc2 = tf.pc_offset 
    assert(pc2 > pc1, 'PC offset should have changed (for the greater)')
    assert_equal( __LINE__, tf.source_location[0])
    assert_equal(['file',  __FILE__], tf.source_container)
  end
end
