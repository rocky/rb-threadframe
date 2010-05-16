#!/usr/bin/env ruby

# This program is from Ruby 1.9. We run it here (again) just in case we didn't
# run as part of checking Ruby 1.9 sanity.

require 'test/unit'

class TestSetTraceFunc < Test::Unit::TestCase
  def setup
    @original_compile_option = RubyVM::InstructionSequence.compile_option
    RubyVM::InstructionSequence.compile_option = {
      :trace_instruction => true,
      :specialized_instruction => false
    }
    @events = []
  end

  def teardown
    set_trace_func(nil)
    RubyVM::InstructionSequence.compile_option = @original_compile_option
  end

  def chunk(list, char='-')
    sep = char * 30 + "\n"
    sep + list.map{|e| e.join(' ')}.join("\n") + "\n"
  end

  def showit(actual, expected)
    chunk(actual, '-') + chunk(expected, '=')
  end

  def checkit(actual, expected)
    actual.each_with_index do |e, i|
      assert_equal(e, actual[i], showit(actual, expected))
    end
    assert_equal(expected.size, actual.size, showit(actual, expected))
  end

  def test_c_call
    eval <<-EOF.gsub(/^.*?: /, '')
     1: set_trace_func(Proc.new { |event, file, lineno, mid, binding, klass|
     2:   @events << [lineno, event, mid, klass]
     3: })
     4: x = 1 + 1
     5: set_trace_func(nil)
    EOF

    expected = [[4, 'line',     __method__, self.class],
                [4, "c-call",   :+,         Fixnum],
                [4, "c-return", :+,         Fixnum],
                [5, "line",     __method__, self.class],
                [5, "c-call",   :set_trace_func, Kernel]]
    checkit(@events, expected)
  end

  def test_call
    eval <<-EOF.gsub(/^.*?: /, '')
     1: set_trace_func(Proc.new { |event, file, lineno, mid, binding, klass|
     2:   @events << [lineno, event, mid, klass]
     3: })
     4: def add(x, y)
     5:   x + y
     6: end
     7: x = add(1, 1)
     8: set_trace_func(nil)
    EOF

    expected = [[4, 'line',     __method__,    self.class],
                [4, 'c-call',   :method_added, Module],
                [4, 'c-return', :method_added, Module],
                [7, 'line',     __method__,    self.class],
                [4, 'call',     :add,          self.class],
                [5, 'line',     :add,          self.class],
                [5, 'c-call',   :+,            Fixnum],
                [5, 'c-return', :+,            Fixnum],
                [6, 'return',   :add,          self.class],
                [8, 'line',      __method__,   self.class],
                [8, 'c-call',   :set_trace_func, Kernel]]
    checkit(@events, expected)
  end

  def test_class
    eval <<-EOF.gsub(/^.*?: /, '')
     1: set_trace_func(Proc.new { |event, file, lineno, mid, binding, klass|
     2:   @events << [lineno, event, mid, klass]
     3: })
     4: class Foo
     5:   def bar
     6:   end
     7: end
     8: x = Foo.new.bar
     9: clear_trace_func()
    EOF
    expected = [[4, 'line',     __method__,   self.class],
                [4, 'c-call',   :inherited,   Class],
                [4, 'c-return', :inherited,   Class],
                [4, 'class',    nil,           nil],
                [5, 'line',     nil,           nil],
                [5, 'c-call',   :method_added, Module],
                [5, 'c-return', :method_added, Module],
                [7, 'end',      nil,           nil],
                [8, 'line',     __method__,    self.class],
                [8, 'c-call',   :new,          Class],
                [8, 'c-call',   :initialize,   BasicObject],
                [8, 'c-return', :initialize,   BasicObject],
                [8, 'c-return', :new,          Class],
                [5, 'call',     :bar,          Foo],
                [6, 'return',   :bar,          Foo],
                [9, 'line',     __method__,    self.class],
                [9, 'c-call',   :clear_trace_func, Kernel]]
    checkit(@events, expected)
  end

  def test_return # [ruby-dev:38701]
    eval <<-EOF.gsub(/^.*?: /, '')
     1: add_trace_func(Proc.new { |event, file, lineno, mid, binding, klass|
     2:   @events << [lineno, event, mid, klass]
     3: })
     4: def foo(a)
     5:   return if a
     6:   return
     7: end
     8: foo(true)
     9: foo(false)
    10: set_trace_func(nil)
    EOF
    expected = [[ 4, 'line',     __method__,   self.class],
                [ 4, 'c-call',   :method_added, Module],
                [ 4, 'c-return', :method_added, Module],
                [ 8, 'line',     __method__,   self.class],
                [ 4, 'call',     :foo,         self.class],
                [ 5, 'line',     :foo,         self.class],
                [ 5, 'return',   :foo,         self.class],
                [ 9, 'line',     :test_return, self.class],
                [ 4, 'call',     :foo,         self.class],
                [ 5, 'line',     :foo,         self.class],
                [ 7, 'return',   :foo,         self.class],
                [10, 'line',     :test_return, self.class],
                [10, 'c-call',   :set_trace_func, Kernel]]
    checkit(@events, expected)
  end

  def test_return2 # [ruby-core:24463]
    eval <<-EOF.gsub(/^.*?: /, '')
     1: set_trace_func(Proc.new { |event, file, lineno, mid, binding, klass|
     2:   @events << [lineno, event, mid, klass]
     3: })
     4: def foo
     5:   a = 5
     6:   return a
     7: end
     8: foo
     9: set_trace_func(nil)
    EOF

    expected = [[4, 'line',     __method__, self.class],
                [4, 'c-call',   :method_added, Module],
                [4, 'c-return', :method_added, Module],
                [8, 'line',     __method__, self.class],
                [4, 'call',     :foo, self.class],
                [5, 'line',     :foo, self.class],
                [6, 'line',     :foo, self.class],
                [7, 'return',   :foo, self.class],
                [9, 'line',     :test_return2, self.class],
                [9, 'c-call',   :set_trace_func, Kernel]]
    @events.each_with_index{|e, i|
      assert_equal(e, @events[i], showit(@events, expected))}
    assert_equal(expected.size, @events.size, showit(@events, expected))
  end

  def test_raise
    events = []
    eval <<-EOF.gsub(/^.*?: /, '')
     1: set_trace_func(Proc.new { |event, file, lineno, mid, binding, klass|
     2:   events << [event, lineno, mid, klass]
     3: })
     4: begin
     5:   raise TypeError, 'error'
     6: rescue TypeError => $e
     7: end
     8: set_trace_func(nil)
    EOF
    
    expected = [[4, 'line',     __method__,     self.class],
                [5, 'line',     __method__,     self.class],
                [5, 'c-call',   :raise,         Kernel],
                [5, 'c-call',   :exception,     Exception],
                [5, 'c-call',   :initialize,    Exception],
                [5, 'c-return', :initialize,    Exception],
                [5, 'c-return', :exception,     Exception],
                [5, 'c-call',   :backtrace,     Exception],
                [5, 'c-return', :backtrace,     Exception],
                [5, 'c-call',   :set_backtrace, Exception],
                [5, 'c-return', :set_backtrace, Exception],
                [5, 'raise',    :test_raise,    $e], 
                [5, 'c-return', :raise,         Kernel],
                [6, 'c-call',   :===,           Module],
                [6, 'c-return', :===,           Module],
                [8, 'line',     __method__,     self.class],
                [8, 'c-call',   :set_trace_func, Kernel]]
    checkit(events, expected)
  end

  def test_break # [ruby-core:27606] [Bug #2610]
    events = []
    eval <<-EOF.gsub(/^.*?: /, '')
     1: set_trace_func(Proc.new { |event, file, lineno, mid, binding, klass|
     2:   events << [lineno, event, mid, klass]
     3: })
     4: [1,2,3].any? {|n| n}
     8: set_trace_func(nil)
    EOF

    expected = [[4, 'line',     __method__, self.class],
                [4, 'c-call',   :any?,      Enumerable],
                [4, 'c-call',   :each,      Array],
                [4, 'line',     __method__, self.class],
                [4, 'c-return', :each,      Array],
                [4, 'c-return', :any?,      Enumerable],
                [5, 'line',     __method__, self.class],
                [5, 'c-call',   :set_trace_func, Kernel]]
    checkit(events, expected)
  end

  def test_invalid_proc
      assert_raise(TypeError) { set_trace_func(1) }
  end

  def test_raise_in_trace
    set_trace_func proc {raise rescue nil}
    assert_equal(42, (raise rescue 42), '[ruby-core:24118]')
  end

  def test_thread_trace
    events = {:set => [], :add => []}
    prc = Proc.new { |event, file, lineno, mid, binding, klass|
      events[:set] << [lineno, event, mid, klass, :set]
    }
    prc2 = Proc.new { |event, file, lineno, mid, binding, klass|
      events[:add] << [lineno, event, mid, klass, :add]
    }

    th = Thread.new do
      th = Thread.current
      eval <<-EOF.gsub(/^.*?: /, '')
       1: th.set_trace_func(prc)
       2: th.add_trace_func(prc2)
       3: class ThreadTraceInnerClass
       4:   def foo
       5:     x = 1 + 1
       6:   end
       7: end
       8: ThreadTraceInnerClass.new.foo
       9: th.set_trace_func(nil)
      EOF
    end
    th.join

    expected = [[1, 'c-return', :set_trace_func, Thread, :set],
                [2, 'line',     __method__, self.class, :set],
                [2, 'c-call',   :add_trace_func, Thread, :set]]
    expected.each do |e|
      assert_equal(e, events[:set].shift, showit(events, expected))
    end

    [[2, 'c-return', :add_trace_func, Thread],
     [3, 'line',     __method__, self.class],
     [3, 'c-call',   :inherited, Class],
     [3, 'c-return', :inherited, Class],
     [3, 'class',    nil, nil],
     [4, 'line',     nil, nil],
     [4, 'c-call',   :method_added, Module],
     [4, 'c-return', :method_added, Module],
     [7, 'end',      nil, nil],
     [8, 'line',     __method__, self.class],
     [8, 'c-call',   :new, Class],
     [8, 'c-call',   :initialize, BasicObject],
     [8, 'c-return', :initialize, BasicObject],
     [8, 'c-return', :new, Class],
     [4, 'call',     :foo, ThreadTraceInnerClass],
     [5, 'line',     :foo, ThreadTraceInnerClass],
     [5, 'c-call',   :+, Fixnum],
     [5, 'c-return', :+, Fixnum],
     [6, 'return',   :foo, ThreadTraceInnerClass],
     [9, 'line',     __method__, self.class],
     [9, 'c-call',   :set_trace_func, Thread]].each do |e|
      [:set, :add].each do |type|
        assert_equal(e + [type], events[type].shift)
      end
    end
    assert_equal([], events[:set])
    assert_equal([], events[:add])
  end

  def test_trace_proc_that_raises_exception_recovery
    $first_time = true
    $traced = []
    s = Proc.new {|event|
      if $first_time
        $first_time = false
        raise RuntimeError
      end
      $traced << event
    }
    begin
      set_trace_func(s)
      assert_equal(false, 'hook should have raised error')
    rescue RuntimeError
      x = 1
      set_trace_func(nil)
      assert_equal(false, $traced.empty?, $traced)
    end
  end

end
