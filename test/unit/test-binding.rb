require 'test/unit'
require_relative File.join('..', '..', 'ext', 'thread_frame')

def outside(a)
    return eval('a', RubyVM::ThreadFrame.new(Thread::current).binding)
end

$a = 10

class TestBinding < Test::Unit::TestCase
  def test_basic
    a = 1
    c = 0
    assert_equal(5, outside(5))
    tf = RubyVM::ThreadFrame.new(Thread::current)
    b  = tf.binding
    assert_equal(1, eval('a', b))
    assert_equal(10, eval('$a', b))
    assert_equal(self, tf.self)
    1.upto(1) do |i;a| 
      tf2 = Thread::current.threadframe()
      b2  = tf2.binding
      a = 2
      assert_equal(2, eval('a', b2))
      assert_equal(0, eval('c', b2))

      # FIXME: times is C inline so prev doesn't work.
      # assert_equal(1, eval('a', tf2.prev.binding))
    end
    def inner(a)
      tf3 = Thread::current.threadframe()
      b3  = tf3.binding
      if a == 4
        assert_equal(4, eval('a', b3))
        inner(a-1)
      else
        assert_equal(3, eval('a', b3))
        assert_equal(4, eval('a', tf3.prev.binding))
      end
    end
    inner(4)
  end
end
