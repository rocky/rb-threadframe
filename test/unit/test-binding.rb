require 'test/unit'

def outside(a)
    return eval('a', RubyVM::Frame.new(Thread::current).binding)
end

$a = 10

class TestBinding < Test::Unit::TestCase
    def test_basic
        a = 1
        c = 0
        assert_equal(5, outside(5))
        tf = RubyVM::Frame.new(Thread::current)
        b  = tf.binding
        assert_equal(1, eval('a', b))
        assert_equal(10, eval('$a', b))
        assert_equal(self, tf.self)
        puts "Reinstate Thread::current.frame()"
        1.times do |i;a|
            tf2 = RubyVM::Frame.get()
            b2  = tf2.binding
            a = 2
            assert_equal(2, eval('a', b2))
            assert_equal(0, eval('c', b2))

            # Times is C inline so prev we can't get a binding for it
            # But we can for use the instruction sequence before that.
            assert_equal(1, eval('a', tf2.prev(2).binding))
        end
        def inner(a)
            tf3 = RubyVM::Frame.get()
            # tf3 = Thread::current.frame()
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
