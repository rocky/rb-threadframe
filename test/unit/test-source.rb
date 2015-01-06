require 'test/unit'

require_relative '../../ext/thread_frame' if '1.9.2' == RUBY_VERSION

# Test source_location and source_container.
class TestSource < Test::Unit::TestCase

  def test_iseq_source_container
    test_basic_lineno = __LINE__ - 1
    tup = method(:test_iseq_source_container).iseq.source_container
    tup[1] = File.basename(tup[1])
    assert_equal(['file',  File.basename(__FILE__)], tup[0...-1])

    eval('def foo; 5 end')
    tup = method(:foo).iseq.source_container
    assert_equal('string',  tup[0])
    # puts tup[1]

    iseq = RubyVM::InstructionSequence.compile("1+2")
    assert_equal('string',  iseq.source_container[0])
    # puts iseq.source_container[1]

    eval_str = '  RubyVM::Frame.get.source_container # test'
    tuple = eval(eval_str)
    assert_equal('string',  tuple[0])
    assert_equal(eval_str,  tuple[1])

  end

  def test_basic
    tf = RubyVM::Frame::get
    # Is this too specific to test/unit.rb implementation details?
    tup = tf.source_container
    tup[1] = File.basename(tup[1])
    assert_equal(['file',  File.basename(__FILE__)], tup[0...-1])
    assert_equal(__LINE__, tf.source_location[0])

    # 1.times creates a C frame.
    1.times do
      expect_line = __LINE__ - 1
      tf = RubyVM::Frame::get
      tup = tf.source_container
      tup[1] = File.basename(tup[1])
      assert_equal(['file',  File.basename(__FILE__)], tup[0...-1])
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
      tf = RubyVM::Frame::get
      assert_equal('BLOCK', tf.type)
      tup = tf.source_container
      tup[1] = File.basename(tup[1])
      assert_equal(['file',  File.basename(__FILE__)], tup[0...-1])
      assert_equal(__LINE__, tf.source_location[0])
      tf = tf.prev
      assert_equal('CFUNC', tf.type)
      tup = tf.source_container
      tup[1] = File.basename(tup[1])
      assert_equal(expect_line, tf.source_location[0])
    end

    x  = lambda do |expect_line|
      tf = RubyVM::Frame::get
      assert_equal('LAMBDA', tf.type)
      tup = tf.source_container
      tup[1] = File.basename(tup[1])
      assert_equal(['file',  File.basename(__FILE__)], tup[0...-1])
      assert_equal(__LINE__, tf.source_location[0])
      tf = tf.prev
      assert_equal('CFUNC', tf.type)
      tup = tf.source_container
      tup[1] = File.basename(tup[1])
      assert_equal(expect_line, tf.source_location[0])
    end
    x.call(__LINE__)

    x  = Proc.new do |expect_line|
      tf = RubyVM::Frame::get
      tup = tf.source_container
      tup[1] = File.basename(tup[1])
      assert_equal(['file',  File.basename(__FILE__)], tup[0...-1])
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
