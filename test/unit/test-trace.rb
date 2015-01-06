# Test of additional tracing flag use to selectively turn on off tracing
require 'test/unit'
require_relative '../../ext/thread_frame' if '1.9.2' == RUBY_VERSION

class TestTracing < Test::Unit::TestCase
  def test_basic_query_set_unset
      skip "FIXME: figure out what to do with trace_off"
    tf = RubyVM::Frame::get
    # Test default values
    assert_equal(false, tf.trace_off?)
    assert_equal(false, tf.return_stop?)

    # Test set true
    tf.trace_off = true
    assert_equal(true, tf.trace_off?)
    tf.return_stop = true
    assert_equal(true, tf.return_stop?)

    # Test setting off when on
    tf.trace_off = nil
    assert_equal(false, tf.trace_off?)
    tf.return_stop = false
    assert_equal(false, tf.return_stop?)
  end

  def test_trace_off
      skip "FIXME: figure out what to do with trace_off"
    @levels = []
    def trace_hook(event, file, line, id, binding, klass)
      @levels << RubyVM::Frame::get.stack_size
    end

    def baz
      6
    end
    def bar(set_off)
      RubyVM::Frame::get.trace_off = true if set_off
      baz
      5
    end
    def foo(set_off)
      bar(set_off)
    end
    # 0x10 is the mask for tracing return events
    Thread.get.set_trace_func(method(:trace_hook).to_proc, 0x10)
    foo(false)
    assert_equal(3,  @levels.size)

    @levels = []
    Thread.get.set_trace_func(method(:trace_hook).to_proc, 0x10)
    foo(true)
    Thread.get.set_trace_func(nil)
    assert_equal(1,  @levels.size)

  end

end
