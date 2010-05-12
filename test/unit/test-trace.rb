# Test of additional tracing flag use to selectively turn on off tracing
require 'test/unit'
require_relative '../../ext/thread_frame'

class TestTracing < Test::Unit::TestCase
  def test_basic_query_set_unset
    tf = RubyVM::ThreadFrame::current
    # Default values
    assert_equal(false, tf.trace_off?)
    assert_equal(false, tf.return_stop?)

    # Set true
    tf.trace_off = true
    assert_equal(true, tf.trace_off?)
    tf.return_stop = true
    assert_equal(true, tf.return_stop?)

    # Set off when on
    tf.trace_off = nil
    assert_equal(false, tf.trace_off?)
    tf.return_stop = false
    assert_equal(false, tf.return_stop?)
  end

  def test_return_stop
    @levels = []
    def trace_hook(event, file, line, id, binding, klass)
      @levels << RubyVM::ThreadFrame::current.stack_size
    end

    def bar
      5
    end
    def foo(set_stop)
      if set_stop
        tf = RubyVM::ThreadFrame::current
        tf.return_stop = true
      end
      bar
    end
    # 0x10 is the mask for tracing return events
    Kernel.set_trace_func(method(:trace_hook).to_proc, 0x10)
    foo(false)
    Kernel.set_trace_func(nil)
    assert_equal(2,  @levels.size)

    # FIXME:
    # @levels = []
    # Kernel.set_trace_func(method(:trace_hook).to_proc, 16)
    # foo(true)
    # Kernel.set_trace_func(nil)
    # assert_equal(1,  @levels)

  end

end
