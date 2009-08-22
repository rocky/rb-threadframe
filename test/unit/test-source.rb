require 'test/unit'

# require 'thread_frame'  # To compare with previous version
require_relative File.join(%w(.. .. ext thread_frame))

# Test source_location and source_container.
class TestSource < Test::Unit::TestCase

  def test_basic
    tf = RubyVM::ThreadFrame::current
    # Is this too specific to test/unit.rb implementation details? 
    tup = tf.source_container
    tup[1] = File.basename(tup[1])
    assert_equal(['file',  File.basename(__FILE__)], tup)
    assert_equal(__LINE__, tf.source_location[0])

    # 1.times creates a C frame.
    1.times do 
      expect_line = __LINE__ - 1
      tf = RubyVM::ThreadFrame::current
      tup = tf.source_container
      tup[1] = File.basename(tup[1])
      assert_equal(['file',  File.basename(__FILE__)], tup)
      assert_equal(tf.source_location[0], __LINE__)
      tf = tf.prev
      assert_equal('CFUNC', tf.type)
      tup = tf.source_container
      tup[1] = File.basename(tup[1])
      assert_equal(expect_line, tf.source_location[0])
    end

    # 1.upto also creates a C frame.
    1.upto(1) do 
      expect_line = __LINE__ - 1
      tf = RubyVM::ThreadFrame::current
      assert_equal('BLOCK', tf.type)
      tup = tf.source_container
      tup[1] = File.basename(tup[1])
      assert_equal(['file',  File.basename(__FILE__)], tup)
      assert_equal(__LINE__, tf.source_location[0])
      tf = tf.prev
      assert_equal('CFUNC', tf.type)
      tup = tf.source_container
      tup[1] = File.basename(tup[1])
      assert_equal(expect_line, tf.source_location[0])
    end

    x  = lambda do |expect_line| 
      tf = RubyVM::ThreadFrame::current
      assert_equal('LAMBDA', tf.type)
      tup = tf.source_container
      tup[1] = File.basename(tup[1])
      assert_equal(['file',  File.basename(__FILE__)], tup)
      assert_equal(__LINE__, tf.source_location[0])
      tf = tf.prev
      assert_equal('CFUNC', tf.type)
      tup = tf.source_container
      tup[1] = File.basename(tup[1])
      assert_equal(expect_line, tf.source_location[0])
    end
    x.call(__LINE__)

    x  = Proc.new do |expect_line|
      tf = RubyVM::ThreadFrame::current
      tup = tf.source_container
      tup[1] = File.basename(tup[1])
      assert_equal(['file',  File.basename(__FILE__)], tup)
      assert_equal(__LINE__, tf.source_location[0])
      tf = tf.prev
      tup = tf.source_container
      tup[1] = File.basename(tup[1])
      # FIXME? 
      # assert_equal(['file',  File.basename(__FILE__)], tup)
      assert_equal(expect_line, tf.source_location[0])
    end
    x.call(__LINE__)
  end
end

# We want to double-check we didn't mess up any pointers somewhere along
# the line.
at_exit { GC.start  }
