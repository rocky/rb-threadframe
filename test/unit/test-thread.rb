require 'test/unit'
require_relative File.join('..', '..', 'ext', 'thread_frame')

class TestThread < Test::Unit::TestCase
  def test_basic
    assert_equal(Thread::Frame.new(Thread::current).thread, Thread::current)
    assert_equal(Thread::current.threadframe.thread, Thread::current)
  end
end
