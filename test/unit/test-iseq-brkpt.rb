require 'test/unit'
require_relative %w(.. .. ext thread_frame)

class TestISeqBrkpt < Test::Unit::TestCase

  def test_iseq_brkpt
    iseq = RubyVM::ThreadFrame.current.iseq
    assert iseq
    assert_equal(true, iseq.brkpt_alloc)
    assert_equal(false, iseq.brkpt_alloc)

    offlines  = iseq.offsetlines
    offsets   = offlines.values
    [offsets.min[0], offsets.max[0]].each do |offset|
      assert_equal(true, iseq.brkpt_set(offset))
      assert_equal(true, iseq.brkpt_set(offset))
      assert_equal(true, iseq.brkpt_get(offset),
                   "Offset %d should be set" % offset)
      assert_equal(true, iseq.brkpt_clear(offset),
                   "Offset %d should be clear" % offset)
      assert_equal(false, iseq.brkpt_get(offset),
                   "Offset %d should be cleared now" % offset)
      assert_equal(true, iseq.brkpt_clear(offset),
                   "Offset %d should be cleared again" % offset)
    end
    
    max_offset = offsets.max[0]

    assert_raises TypeError do iseq.brkpt_get(iseq.iseq_size) end

    assert_equal(true, iseq.brkpt_dealloc)
    assert_equal(false, iseq.brkpt_dealloc)
    assert_equal(true, iseq.brkpt_clear(max_offset),
                 "Offset %d should be clear even when deallocated" % max_offset)

    assert_raises TypeError do iseq.brkpt_get('a') end
    assert_raises TypeError do iseq.brkpt_set('a') end

  end

end

# We want to double-check we didn't mess up any pointers somewhere.
at_exit { GC.start  }
