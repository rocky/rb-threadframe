# Test of additional tracing flag use to selectively turn on off tracing
require 'test/unit'
require_relative '../../ext/thread_frame'

class TestTracingMasks < Test::Unit::TestCase
  @@EVENT2MASK = {
    'line'     => 0x01,
    'call'     => 0x08,
    'return'   => 0x10,
    'c-call'   => 0x20,
    'c-return' => 0x40,
    'raise'    => 0x80,
  }

  def something_to_test(n)
    def sqr(x)
      x * x 
    end
    begin
      n += sqr(5)
      raise TypeError, 'error'
    rescue TypeError => $e
    end
    return [1,2,3].all? {|n| n}
  end

  def chunk(list, char='-')
    sep = char * 30 + "\n"
    sep + list.map{|e| e.join(' ')}.join("\n") + "\n"
  end

  def checkit(event_name)
    chunk(@events) if $DEBUG
    assert @events.all?{|e| e[1] == event_name}, chunk(@events)  
    assert @events.size > 0, "Expecting at least one #{event_name}"
    @counts[event_name] = @events.size if @keep_count
  end  

  def test_thread_trace_mask
    def trace_hook(event, file, line, id, binding, klass)
      @events << [line, event, id, klass]
    end

    @keep_count = true
    @counts = {}
    @@EVENT2MASK.each do |event_name, mask|
      @events = []
      Thread.current.set_trace_func(method(:trace_hook).to_proc, mask)
      something_to_test(5)
      Thread.current.set_trace_func(nil)
      checkit(event_name)
    end
    [%w(call return),
     %w(c-call c-return)].each do |c, r|
      assert_equal(@counts[c], @counts[r], 
                   "Expecting # of #{c}'s to equal # of #{r}'s")
    end

    @keep_count = false

    [%w(line call),
     %w(raise c-return return),
    ].each do |event_names|
      @events = []
      mask = event_names.map{|name| 
        @@EVENT2MASK[name]
      }.inject do 
        |mask, bit| 
        mask |= bit
      end
      Thread.current.set_trace_func(method(:trace_hook).to_proc, mask)
      something_to_test(5)
      Thread.current.set_trace_func(nil)
      total = event_names.map{|name| @counts[name]}.inject do 
        |sum, n| 
        sum += n
      end
      assert_equal(total, @events.size, chunk(@events))
    end

    # Try without a mask and see that we get the sum of all events
    @events = []
    Thread.current.set_trace_func(method(:trace_hook).to_proc)
    something_to_test(5)
    Thread.current.set_trace_func(nil)
    total = @counts.values.inject {|sum, n| sum += n}
    assert_equal(total, @events.size, chunk(@events))
  end

end
