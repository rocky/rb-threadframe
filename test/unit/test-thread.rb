require 'test/unit'
require_relative %w(.. .. ext thread_frame)

class TestThread < Test::Unit::TestCase
  def test_basic
    assert_equal(RubyVM::ThreadFrame.new(Thread::current).thread, 
                 RubyVM::ThreadFrame::current.thread)
    assert_equal(RubyVM::ThreadFrame.new(Thread::current).thread, 
                 Thread::current)
    assert_equal(Thread::current.threadframe.thread, Thread::current)
  end

  def test_pc_offset
    tf = RubyVM::ThreadFrame::current
    offset_1 = tf.pc_offset
    assert_equal(true, offset_1 > 0,
                 "Expecting a positive integer pc offset, got %s" % offset_1)
    offset_2 = tf.pc_offset
    assert_equal(true, offset_2 > 0,
                 "Expecting a positive integer pc offset, got %s" % offset_2)
    assert_equal(true, offset_2 > offset_1,
                 "Expecting second pc offset %s to be larger than the first %s" % 
                 [offset_2, offset_1])
  end

  def test_sp
    tf = RubyVM::ThreadFrame::current.prev
    assert tf.sp(1)
    tf.sp_set(1, 5)
    assert_equal(5, tf.sp(1),
                 'chcking value of recently-set sp(1)')
  end

  def test_source_container
    cfunc_filebase = 'cfunc-use'
    load File.join(File.dirname(__FILE__), cfunc_filebase + '.rb')
    type, loc = cfunc_loc
    assert_equal(['CFUNC',  cfunc_filebase], [type, File.basename(loc, '.rb')],
                 'CFUNCs should get their file location from frame.prev*')
    cont = 'file'
    eval '1.times{cont = RubyVM::ThreadFrame.current.source_container[0]}'
    assert_equal('string',  cont, 
                 'source container[0] of an eval(...) should be "string"')
  end
    

  def test_thread_tracing
    assert_equal(false, Thread.current.tracing)
    Thread.current.tracing = true
    assert_equal(true, Thread.current.tracing)
    Thread.current.tracing = false
    assert_equal(false, Thread.current.tracing)
  end    
    
  def test_fields(notused=nil)
    tf = RubyVM::ThreadFrame::current
    pc1 = tf.pc_offset
    assert(pc1 > 0, 'Should be able to get a valid PC offset')
    # pc_offset is dynamic - it changes constantly
    pc2 = tf.pc_offset 
    assert(pc2 > pc1, 'PC offset should have changed (for the greater)')
    assert_equal('test_fields', tf.method)
    assert_equal(self, tf.self)
    assert_equal(0, tf.arity)
    assert_equal(0, tf.argc)
    assert tf.dfp(0)
    assert tf.lfp(0)

    assert_raises IndexError do
      x = tf.lfp(tf.iseq.local_size+1)
    end


    tf_prev = tf.prev
    assert(tf_prev.pc_offset > 0, "Should be valid PC offset for prev")
  
    # Is this too specific to test/unit.rb implementation details? 
    assert_equal('run', tf_prev.method)

    # 1.times creates a C frame.
    1.times do 
      tf = RubyVM::ThreadFrame::current
      tup = tf.source_container
      tup[1] = File.basename(tup[1])
      assert_equal(['file', 'test-thread.rb'], tup)
      assert_equal('block in test_fields', tf.method)
      assert_equal('CFUNC', tf.prev.type)
      assert_equal('times', tf.prev.method) 
      assert_equal(self, tf.self)
      assert_equal(0, tf.prev.arity, 'C arity should work nowadays' )
      assert_equal(0, tf.prev.argc, 'C args is the same as arity')
      assert_equal('test_fields', tf.prev.prev.method) 
      assert_equal(0, tf.arity)
      assert_equal(0, tf.argc)
    end

    # 1.upto also creates a C frame.
    1.upto(1) do 
      tf = RubyVM::ThreadFrame::current.prev
      assert_equal('CFUNC', tf.type)
      assert_equal(1, tf.arity, 'C arity should work nowadays' )
      assert_equal(1, tf.argc)
    end

    x  = lambda do |x,y| 
      frame = RubyVM::ThreadFrame::current
      assert_equal('block in test_fields', frame.method)
      assert_equal('LAMBDA', frame.type)
      assert_equal(x, tf.self)
    assert_equal(2, frame.arity)
    assert_equal(2, frame.argc)
    end
    x.call(x,2)

    x  = Proc.new do |x, y|
      frame = RubyVM::ThreadFrame::current
      assert_equal('block in test_fields', frame.method)
      assert_equal(x, tf.self)
    assert_equal('BLOCK', frame.type)
    end
    x.call(x,2)

  end

  def test_threadframe_equal
    tf = RubyVM::ThreadFrame.current
    tf2 = RubyVM::ThreadFrame.current
    assert_equal(true,  tf.equal?(tf))
    assert_equal(true,  tf.equal?(tf2))
    tf2 = tf2.prev 
    assert_equal(false, tf.equal?(tf2))
    assert_raises TypeError do
      tf.equal?(tf.iseq)
    end
  end
end

# We want to double-check we didn't mess up any pointers somewhere along
# the line.
at_exit { GC.start  }
