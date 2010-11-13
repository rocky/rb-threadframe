require 'test/unit'
require_relative '../../ext/thread_frame'
require_relative '../../lib/thread_frame'

$global_test_line = __LINE__

class TestLibISeq < Test::Unit::TestCase

  TEST_LINE = __LINE__

  def test_sha1
    iseq1 = RubyVM::ThreadFrame::current.iseq
    iseq2 = RubyVM::ThreadFrame::current.iseq
    assert_equal(iseq1.sha1, iseq2.sha1, 
                 "SHA1 for same threadframe should match")
  end

  def test_lines
    line = __LINE__
    iseq = RubyVM::ThreadFrame::current.iseq
    assert_equal((line-1..__LINE__+2).to_a, iseq.lines,
                 "lines of test_lines() don't match")
  end

  def test_locate_line
    line = __LINE__
    iseq = RubyVM::ThreadFrame::current.iseq
    assert iseq.locate_line(line)
    assert_nil iseq.locate_line(line - 2)
  end

  def test_iseq_with_line
    # FIXME: We get a more stringent test if we test of offset.
    # It is lame how little we can do here.
    line = __LINE__
    def find_line(line) # :nodoc
      tf = RubyVM::ThreadFrame.current
      assert(tf.iseq.find_iseq_with_line(line), 
             "should have found line #{line}")
    end
    tf = RubyVM::ThreadFrame.current
    find_line(line+2)
    # line2 = nil
    # 1.times do 
    #   line2 = __LINE__
    # end
    # find_line(line2)
    # find_line(TEST_LINE)
    # find_line($global_test_line)
  end

end

# We want to double-check we didn't mess up any pointers somewhere.
at_exit { GC.start  }
