require 'test/unit'

# require 'thread_frame'  # To compare with previous version
require_relative '../../ext/thread_frame'

# Test source_location and source_container.
class TestReturnStop < Test::Unit::TestCase

  def setup
    @tuples = []
    @p = Proc.new { |event, file, lineno, mid, binding, klass|
      @tuples << [event, lineno, mid, klass]
      # p [event, lineno, mid, klass]
    }
  end

  # Another method to call used to make sure we have turned off
  # tracing.
  def five; 5 end

  def recurse(a, trace_off)
    tf = RubyVM::ThreadFrame::current
    if a==1
      assert_equal false, tf.return_stop?
      tf.return_stop=trace_off
      assert_equal trace_off, tf.return_stop?
      tf.trace_off=true
      assert_equal true, tf.trace_off?
      set_trace_func(@p)
      return recurse(2, trace_off)
    else
      five
    end
  end

  def tup_to_s(t)
    @tuples.map do |t| 
      '[' + t.map{|t2| t2.inspect}.join(', ') + ']'
    end.join("\n")
  end

  def test_return_stop
    recurse(1, true)
    set_trace_func(nil)
    first = @tuples.dup
    assert_equal('return', @tuples[0][0],
                 "First tuple recorded should have been a return event," + 
                 "got: #{ tup_to_s(@tuples)}")
    ## FIXME: there's still a bug in the code somewhere.
    # Compare with what we get when we don't turn tracing off.
    # recurse(1, false)

    # assert_equal(true, @tuples.size > first.size,
    #              'should have gotten more tuples recorded')
    # assert_equal(true, @tuples.member?(first[0]),
    #              'should find "return" event in longer trace')
    # puts tup_to_s(first)
    # puts '-' * 30
    # puts tup_to_s(@tuples)
    # assert_equal(true, @tuples.index(first[0]) > 0,
    #              'should not find "return" event as the first event')
  end

end
