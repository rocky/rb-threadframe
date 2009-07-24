require 'test/unit'
require_relative File.join(%w(.. .. ext thread_frame))

class TestProc < Test::Unit::TestCase
  def test_basic
    @tf = RubyVM::ThreadFrame::current
    assert_equal(false, @tf.invalid?, 
                 'Frame should be valid right after ThreadFrame::current')
    def notgood(test_tf=nil)
      if test_tf
        assert_equal(test_tf != @tf, test_tf.invalid?)
      end
      return RubyVM::ThreadFrame::current
    end

    def inner_fn(tf)
      tf.invalid?
    end
      
    invalid_tf = notgood
    # FIXME:
    # assert_equal(true, invalid_tf.invalid?,
    #             'current thread frame should not be returned from a fn')
    # begin
    #   b = invalid_tf.binding
    #   assert false, 'Should have raised an ThreadFrameError'
    # rescue ThreadFrameError
    #   assert true
    # end
    # Add a new local variable
    x = 5
    assert_equal(false, @tf.invalid?, 
                 'Frame should still be valid after adding more locals')
    assert_equal(false, inner_fn(@tf),
                 'outer thread frame should ok inside a called fn')
    notgood(invalid_tf)
    notgood(@tf)
  end
end
