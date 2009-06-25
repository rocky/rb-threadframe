require 'test/unit'
require_relative File.join('..', '..', 'ext', 'thread_frame')

class TestThread < Test::Unit::TestCase
  def test_basic
    assert_equal(Thread::Frame.current.thread, Thread::current)
  end
end
