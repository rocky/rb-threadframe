require 'test/unit'
require_relative '../../ext/thread_frame' if '1.9.2' == RUBY_VERSION

class TestThread < Test::Unit::TestCase
  def test_basic
    assert_equal(RubyVM::Frame.new(Thread::current).thread, 
                 RubyVM::Frame::current.thread)
    assert_equal(RubyVM::Frame.new(Thread::current).thread, 
                 Thread::current)
    assert_equal(Thread::current.frame.thread, Thread::current)
  end

  def test_pc_offset
    tf = RubyVM::Frame::current
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
    tf = RubyVM::Frame.prev
    assert tf.sp(1)
    tf.sp_set(1, 5)
    assert_equal(5, tf.sp(1), 'checking value of recently-set sp(1)')
  end

  def test_source_container
    cfunc_filebase = 'cfunc-use'
    load File.join(File.dirname(__FILE__), cfunc_filebase + '.rb')
    type, loc = cfunc_loc
    assert_equal(['CFUNC',  cfunc_filebase], [type, File.basename(loc, '.rb')],
                 'CFUNCs should get their file location from frame.prev*')
    cont = 'file'
    eval '1.times{cont = RubyVM::Frame.current.source_container[0]}'
    assert_equal('string',  cont, 
                 'source container[0] of an eval(...) should be "string"')
  end
    

  def test_source_location
    line = __LINE__
    # There is a bug in Ruby 1.9.2 and before in reporting the source
    # location when the PC is 0. Probably never reported before
    # because the location is reported on a traceback
    # and that probably can't happen at PC 0.
    def bug_when_zero_pc
      skip "reinstate this on 1.9.3" if '1.9.3' == RUBY_VERSION
      @not_first = true
      tf = RubyVM::Frame::current.prev
      pc_save = tf.pc_offset
      tf.pc_offset = 0
      loc = tf.source_location
      tf.pc_offset = pc_save
      loc
    end
    loc = bug_when_zero_pc unless @not_first
    assert_equal([line - 1], loc)
  end
    

  def test_thread_tracing
    skip "reinstate this on 1.9.3" if '1.9.3' == RUBY_VERSION
    assert_equal(false, Thread.current.tracing)
    Thread.current.tracing = true
    assert_equal(true, Thread.current.tracing)
    Thread.current.tracing = false
    assert_equal(false, Thread.current.tracing)
  end    
    
  def test_thread_exec_event_tracing
    skip "reinstate this on 1.9.3" if '1.9.3' == RUBY_VERSION
    assert_equal(false, Thread.current.exec_event_tracing)
    Thread.current.exec_event_tracing = true
    assert_equal(true, Thread.current.exec_event_tracing)
    Thread.current.exec_event_tracing = false
    assert_equal(false, Thread.current.exec_event_tracing)
  end    
    
  def test_fields(notused=nil)
    tf = RubyVM::Frame::current
    pc1 = tf.pc_offset
    assert(pc1 > 0, 'Should be able to get a valid PC offset')
    # pc_offset is dynamic - it changes constantly
    pc2 = tf.pc_offset 
    assert(pc2 > pc1, 'PC offset should have changed (for the greater)')
    assert_equal('test_fields', tf.method)
    assert_equal(self, tf.self)
    assert_equal(0, tf.arity)
    assert_equal(0, tf.argc)
    ## FIXME: Should we allow this?
    ## assert tf.dfp(0)
    ## assert tf.lfp(0)

    # assert_raises IndexError do
    #   x = tf.lfp(tf.iseq.local_size+1)
    # end


    tf_prev = tf.prev
    assert(tf_prev.pc_offset > 0, "Should be valid PC offset for prev")
  
    # Is this too specific to test/unit.rb implementation details? 
    assert_equal('run', tf_prev.method)

    # 1.times creates a C frame.
    1.times do 
      tf = RubyVM::Frame::current
      tup = tf.source_container
      tup[1] = File.basename(tup[1])
      assert_equal(['file', 'test-frame.rb'], tup)
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
      # We could use ".prev" below instead of '.current.prev", but we
      # may as well test current.prev.
      tf = RubyVM::Frame::current.prev  
      assert_equal('CFUNC', tf.type)
      assert_equal(1, tf.arity, 'C arity should work nowadays' )
      assert_equal(1, tf.argc)
    end

    x  = lambda do |x,y| 
      frame = RubyVM::Frame::current
      assert_equal('block in test_fields', frame.method)
      assert_equal('LAMBDA', frame.type)
      assert_equal(x, tf.self)
      assert_equal(2, frame.arity)
      assert_equal(2, frame.argc)
    end
    x.call(x,2)

    x  = Proc.new do |x, y|
      frame = RubyVM::Frame::current
      assert_equal('block in test_fields', frame.method)
      assert_equal(x, tf.self)
    assert_equal('BLOCK', frame.type)
    end
    x.call(x,2)

  end

  def test_frame_equal
    tf = RubyVM::Frame.current
    tf2 = RubyVM::Frame.current
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
