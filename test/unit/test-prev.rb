require 'test/unit'

class TestThread < Test::Unit::TestCase
    def test_stack_size_with_prev
        tf = RubyVM::Frame.new(Thread::get)

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

        assert RubyVM::Frame::prev(Thread::current, 1),
        'should allow 2-arg prev'
        assert RubyVM::Frame::prev(Thread::current),
        'should allow 1-arg thread prev'
        assert(RubyVM::Frame::prev(2),
               'There should be at least two prior frames in single Fixnum prev')

        top_frame = RubyVM::Frame::prev(Thread::get, -1)
        assert(top_frame, 'Should give back the top frame for two arg and -1')
        assert_equal('TOP', top_frame.type,
                     'The type of the top frame should be "TOP"')

        top_frame = RubyVM::Frame::prev(-1)
        assert(top_frame, 'Should give back the top frame for one arg and -1')
        assert_equal('TOP', top_frame.type,
                     'The type of the top frame should be "TOP"')

        assert_equal(nil, RubyVM::Frame::prev(Thread::get, 1000))

        tf = RubyVM::Frame.prev

        assert tf.prev(2)
        assert_equal(tf,  tf.prev(0),
                     'tf.prev(0) is defined as be tf')
        assert tf.prev(-1)
        assert_equal(nil, tf.prev(1000))

        assert_raises TypeError do
            tf.prev('a')
        end
        assert_raises ArgumentError do
            tf.prev(RubyVM::Frame::get, 1, 'bad_arg')
        end
        assert_raises TypeError do
            RubyVM::Frame::prev([1])
        end
        assert_raises TypeError do
            RubyVM::Frame::prev(RubyVM::Frame::get, [1])
        end

  end
end
