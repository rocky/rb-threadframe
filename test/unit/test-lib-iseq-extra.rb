require 'test/unit'
require_relative %w(.. .. lib iseq_extra)

class TestLibISeqExtra < Test::Unit::TestCase

  def test_basic
    tf = RubyVM::ThreadFrame.current
    iseq = tf.iseq
    # See that we get the same line numbers
    assert_equal(iseq.offsetlines.values.flatten.uniq.sort, 
                 iseq.lineoffsets.keys.sort)
    # See that we get the same offsets
    assert_equal(iseq.lineoffsets.values.flatten.uniq.sort, 
                 iseq.offsetlines.keys.sort)

    assert_equal(iseq.lineoffsets[__LINE__].sort, 
                 iseq.line2offsets(__LINE__-1).sort)

    assert_equal([], iseq.line2offsets(__LINE__+100))
    top_iseq = tf.prev(-1).iseq
    assert_equal('method', RubyVM::InstructionSequence::TYPE2STR[top_iseq.type])

    iseq2 = tf.iseq
    # Different object ids...
    assert_not_equal(iseq.object_id, iseq2.object_id)
    #    but same SHA1s..
    assert_equal(iseq.sha1, iseq2.sha1)
    #    and equal instruction sequences
    assert_equal(true, iseq.equal?(iseq2))
    
    # Now try two different iseqs
    tf2 = tf.prev
    tf2 = tf2.prev until !tf2.prev || tf2.prev.iseq
    if tf2
      assert_not_equal(iseq.sha1, tf2.iseq.sha1) 
      assert_equal(false, iseq.equal?(tf2.iseq))
    end
  end

end
