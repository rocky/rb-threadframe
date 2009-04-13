require 'test/unit'
require_relative File.join('..', 'ext', 'thread_frame')

def outside(a)
    return eval('a', ThreadFrame.current.binding)
end

$a = 10

class TestBinding < Test::Unit::TestCase
  def test_basic
    a = 1
    b = 0
    assert_equal(5, outside(5))
    assert_equal(1, eval('a', ThreadFrame.current.binding))
    assert_equal(10, eval('$a', ThreadFrame.current.binding))
    1.times do |a| 
      a = 2
      assert_equal(2, eval('a', ThreadFrame.current.binding))
      assert_equal(0, eval('b', ThreadFrame.current.binding))
      # FIXME? 
      # assert_equal(1, eval('a', ThreadFrame.current.prev.binding))
    end
    def inner(a)
      if a == 4
        assert_equal(4, eval('a', ThreadFrame.current.binding))
        inner(a-1)
      else
        assert_equal(3, eval('a', ThreadFrame.current.binding))
        assert_equal(4, eval('a', ThreadFrame.current.prev.binding))
      end
    end
    inner(4)
  end
end
