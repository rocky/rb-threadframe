require 'test/unit'
require_relative File.join(%w(.. .. ext thread_frame))

class TestThread < Test::Unit::TestCase
  def test_basic
    assert_equal(RubyVM::ThreadFrame.new(Thread::current).thread, 
                 RubyVM::ThreadFrame::current.thread)
    assert_equal(RubyVM::ThreadFrame.new(Thread::current).thread, 
                 Thread::current)
    assert_equal(Thread::current.threadframe.thread, Thread::current)
  end

  def test_fields
    tf = RubyVM::ThreadFrame::current
    pc1 = tf.pc_offset
    assert(pc1 > 0, 'Should be able to get a valid PC offset')
    # pc_offset is dynamic - it changes constantly
    pc2 = tf.pc_offset 
    assert(pc2 > pc1, 'PC offset should have changed (for the greater)')
    assert_equal( __LINE__, tf.source_location[0])
    assert_equal(['file',  __FILE__], tf.source_container)
    assert_equal('test_fields', tf.method)
    assert_equal(self, tf.self)
    assert_equal(0, tf.arity)

    tf_prev = tf.prev
    assert(tf_prev.pc_offset > 0, "Should be valid PC offset for prev")
  
    # Is this too specific to test/unit.rb implementation details? 
    tup = tf_prev.source_container
    tup[1] = File.basename(tup[1])
    assert_equal(['file',  'unit.rb'], tup)
    assert_equal('run', tf_prev.method)

    # Test prev with an argument count
    assert tf.prev(2)
    assert_equal(nil, tf.prev(-1))
    assert_equal(nil, tf.prev(1000))

    # 1.times creates a C frame.
    1.times do 
      tf = RubyVM::ThreadFrame::current
      tup = tf.source_container
      tup[1] = File.basename(tup[1])
      assert_equal(['file', 'test-thread.rb'], tup)
      assert_equal('block in test_fields', tf.method)
      assert_equal('CFUNC', tf.prev.type)
      assert_equal('times', tf.prev.method) 
      assert_equal(Integer, tf.prev.method_class)
      assert_equal(self, tf.self)
      assert_equal(0, tf.prev.arity, 'C arity should work nowadays' )
      assert_equal('test_fields', tf.prev.prev.method) 
      assert_equal(0, tf.arity)
    end

    # 1.upto also creates a C frame.
    1.upto(1) do 
      tf = RubyVM::ThreadFrame::current.prev
      assert_equal('CFUNC', tf.type)
      assert_equal(1, tf.arity, 'C arity should work nowadays' )
    end

    x  = lambda do |x,y| 
      frame = RubyVM::ThreadFrame::current
      assert_equal('block in test_fields', frame.method)
      assert_equal('LAMBDA', frame.type)
      assert_equal(x, tf.self)
      assert_equal(x.class, tf.method_class)
      assert_equal(2, frame.arity)
    end
    x.call(x,2)

    x  = Proc.new do |x, y|
      frame = RubyVM::ThreadFrame::current
      assert_equal('block in test_fields', frame.method)
      assert_equal(x, tf.self)
      assert_equal(x.class, tf.method_class)
      assert_equal('BLOCK', frame.type)
    end
    x.call(x,2)

  end
end
