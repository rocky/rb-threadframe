require 'test/unit'
require_relative File.join('..', 'ext', 'thread_frame')

def outside(a)
    return eval('a', Thread::Frame.current.binding)
end

$a = 10

class TestBinding < Test::Unit::TestCase
  def test_basic
    a = 1
    b = 0
    assert_equal(5, outside(5))
    assert_equal(1, eval('a', Thread::Frame.current.binding))
    assert_equal(10, eval('$a', Thread::Frame.current.binding))
    assert_equal(self, Thread::Frame.current.self)
    1.times do |a| 
      a = 2
      assert_equal(2, eval('a', Thread::Frame.current.binding))
      assert_equal(0, eval('b', Thread::Frame.current.binding))
      # FIXME? 
      # assert_equal(1, eval('a', Thread::Frame.current.prev.binding))
    end
    def inner(a)
      if a == 4
        assert_equal(4, eval('a', Thread::Frame.current.binding))
        inner(a-1)
      else
        assert_equal(3, eval('a', Thread::Frame.current.binding))
        assert_equal(4, eval('a', Thread::Frame.current.prev.binding))
      end
    end
    inner(4)
  end
end
