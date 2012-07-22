require 'test/unit'

# require 'thread_frame'  # To compare with previous version
require_relative '../../ext/thread_frame'

# Test source_location and source_container.
class TestSpSize < Test::Unit::TestCase

  def sizes
    tf = RubyVM::Frame::current
    ary = []
    0.upto(2) do |i|
      ary << tf.sp_size
      tf = tf.prev
    end
    # Swap first two items. The item that generally 
    # will vary is the a[0].
    ary[0], ary[1] = ary[1], ary[0]
    # p ary
    return ary
  end

  def f0; return sizes end
  def f1; a=1; return sizes end
  def f1a(a) return sizes end
  def f2(a,b) return sizes end

  def test_sp_size
    f0_s   = f0
    f1_s   = f1
    f1a_s  = f1a(1)
    f2_s   = f2(1,2)
    assert_equal(f0_s[0]+1,   f1_s[0])
    assert_equal(f0_s[1..-1], f1_s[1..-1])
    assert_equal(f1_s, f1a_s)
    assert_equal(f1_s[0]+1,   f2_s[0])
    assert_equal(f1_s[1..-1], f2_s[1..-1])

    assert_raises ArgumentError do 
      tf = RubyVM::Frame.current
      tf.sp_set(tf.sp_size, "Should not be able to set this.")
    end
  end

end
