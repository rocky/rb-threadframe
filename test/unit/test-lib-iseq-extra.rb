require 'test/unit'
require_relative %w(.. .. lib iseq_extra)

class TestLibISeqExtra < Test::Unit::TestCase

  def test_basic
    iseq = RubyVM::ThreadFrame.current.iseq
    # See that we get the same line numbers
    assert_equal(iseq.offsetlines.values.flatten.uniq.sort, 
                 iseq.lineoffsets.keys.sort)
    # See that we get the same offsets
    assert_equal(iseq.lineoffsets.values.flatten.uniq.sort, 
                 iseq.offsetlines.keys.sort)

    assert_equal(iseq.lineoffsets[__LINE__].sort, 
                 iseq.line2offsets(__LINE__-1).sort)

    assert_equal([], iseq.line2offsets(__LINE__+100))
    top_iseq = RubyVM::ThreadFrame.current.prev(-1).iseq
    assert_equal('method', RubyVM::InstructionSequence::TYPE2STR[top_iseq.type])
  end

end
