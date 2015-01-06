require 'test/unit'
require_relative '../../ext/thread_frame' if '1.9.2' == RUBY_VERSION

class TestISeqBrkpt < Test::Unit::TestCase

  def test_iseq_brkpt
    iseq = RubyVM::Frame.get.iseq
    assert iseq
    assert_equal(nil, iseq.brkpts)
    assert_equal(true, iseq.brkpt_alloc)
    assert_equal([], iseq.brkpts)
    assert_equal(false, iseq.brkpt_alloc)

    offlines  = iseq.offsetlines
    offsets   = offlines.values
    [offsets.min[0], offsets.max[0]].each do |offset|
      assert_equal(true, iseq.brkpt_set(offset))
      assert_equal(true, iseq.brkpt_set(offset))
      assert_equal(true, iseq.brkpt_get(offset),
                   "Offset %d should be set" % offset)
      assert_equal(true, iseq.brkpt_unset(offset),
                   "Offset %d should be unset" % offset)
      assert_equal(false, iseq.brkpt_get(offset),
                   "Offset %d should be unset now" % offset)
      assert_equal(true, iseq.brkpt_unset(offset),
                   "Offset %d should be unset again" % offset)
      iseq.brkpt_set(offset) # For test below
    end
    assert_equal(2, iseq.brkpts.size)

    max_offset = offsets.max[0]

    assert_raises TypeError do iseq.brkpt_get(iseq.iseq_size) end

    assert_equal(true, iseq.brkpt_dealloc)
    assert_equal(false, iseq.brkpt_dealloc)
    assert_equal(true, iseq.brkpt_unset(max_offset),
                 "Offset %d should be unset even when deallocated" % max_offset)

    assert_raises TypeError do iseq.brkpt_set('a') end
  end

  def test_iseq_brkpt_set
    set_trace_func(Proc.new { |event, file, lineno, mid, binding, klass|
                     if 'brkpt' == event
                       $saw_brkpt = true
                     end
                    })

    $saw_brkpt = false
    tf = RubyVM::Frame.get
    tf.iseq.offsetlines.keys.each do |offset|
      tf.iseq.brkpt_set(offset)
    end
    assert_equal(true, $saw_brkpt)
    set_trace_func nil
  end
end

# We want to double-check we didn't mess up any pointers somewhere.
at_exit { GC.start  }
