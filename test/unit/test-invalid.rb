require 'test/unit'

class TestInvalid < Test::Unit::TestCase
  def test_basic
      @tf = RubyVM::Frame::get
      assert_equal(true, @tf.valid?,
                   'Frame should be valid right after RubyVM::Frame::get')
      def notgood(test_tf=nil)
          # FIXME
          # if test_tf
          #   assert_equal(test_tf == @tf, test_tf.valid?)
          # end
          return RubyVM::Frame::get
      end

      def inner_fn(tf)
          tf.valid?
      end

      invalid_tf = notgood
      # FIXME:
      # assert_equal(true, invalid_tf.valid?,
      #             'current thread frame should not be returned from a fn')
      # begin
      #   b = invalid_tf.binding
      #   assert false, 'Should have raised an ThreadFrameError'
      # rescue ThreadFrameError
      #   assert true
      # end
      # Add a new local variable
      x = 5
      assert_equal(true, @tf.valid?,
                   'Frame should still be valid after adding more locals')
      assert_equal(true, inner_fn(@tf),
                   'outer thread frame should ok inside a called fn')
      notgood(invalid_tf)
      notgood(@tf)
  end
end
