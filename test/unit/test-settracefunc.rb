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
  end

  def teardown
    set_trace_func(nil)
    RubyVM::InstructionSequence.compile_option = @original_compile_option
  end

  def test_c_call
    events = []
    eval <<-EOF.gsub(/^.*?: /, "")
     1: set_trace_func(Proc.new { |event, file, lineno, mid, binding, klass|
     2:   events << [event, lineno, mid, klass]
     3: })
     4: x = 1 + 1
     5: set_trace_func(nil)
    EOF
    assert_equal(["line", 4, __method__, self.class],
                 events.shift)
    assert_equal(["c-call", 4, :+, Fixnum],
                 events.shift)
    assert_equal(["c-return", 4, :+, Fixnum],
                 events.shift)
    assert_equal(["line", 5, __method__, self.class],
                 events.shift)
    assert_equal(["c-call", 5, :set_trace_func, Kernel],
                 events.shift)
    assert_equal([], events)
  end

  def test_call
    events = []
    eval <<-EOF.gsub(/^.*?: /, "")
     1: set_trace_func(Proc.new { |event, file, lineno, mid, binding, klass|
     2:   events << [event, lineno, mid, klass]
     3: })
     4: def add(x, y)
     5:   x + y
     6: end
     7: x = add(1, 1)
     8: set_trace_func(nil)
    EOF
    assert_equal(["line", 4, __method__, self.class],
                 events.shift)
    assert_equal(["c-call", 4, :method_added, Module],
                 events.shift)
    assert_equal(["c-return", 4, :method_added, Module],
                 events.shift)
    assert_equal(["line", 7, __method__, self.class],
                 events.shift)
    assert_equal(["call", 4, :add, self.class],
                 events.shift)
    assert_equal(["line", 5, :add, self.class],
                 events.shift)
    assert_equal(["c-call", 5, :+, Fixnum],
                 events.shift)
    assert_equal(["c-return", 5, :+, Fixnum],
                 events.shift)
    assert_equal(["return", 6, :add, self.class],
                 events.shift)
    assert_equal(["line", 8, __method__, self.class],
                 events.shift)
    assert_equal(["c-call", 8, :set_trace_func, Kernel],
                 events.shift)
    assert_equal([], events)
  end

  def test_class
    events = []
    eval <<-EOF.gsub(/^.*?: /, "")
     1: set_trace_func(Proc.new { |event, file, lineno, mid, binding, klass|
     2:   events << [event, lineno, mid, klass]
     3: })
     4: class Foo
     5:   def bar
     6:   end
     7: end
     8: x = Foo.new.bar
     9: clear_trace_func()
    EOF
    assert_equal(["line", 4, __method__, self.class],
                 events.shift)
    assert_equal(["c-call", 4, :inherited, Class],
                 events.shift)
    assert_equal(["c-return", 4, :inherited, Class],
                 events.shift)
    assert_equal(["class", 4, nil, nil],
                 events.shift)
    assert_equal(["line", 5, nil, nil],
                 events.shift)
    assert_equal(["c-call", 5, :method_added, Module],
                 events.shift)
    assert_equal(["c-return", 5, :method_added, Module],
                 events.shift)
    assert_equal(["end", 7, nil, nil],
                 events.shift)
    assert_equal(["line", 8, __method__, self.class],
                 events.shift)
    assert_equal(["c-call", 8, :new, Class],
                 events.shift)
    assert_equal(["c-call", 8, :initialize, BasicObject],
                 events.shift)
    assert_equal(["c-return", 8, :initialize, BasicObject],
                 events.shift)
    assert_equal(["c-return", 8, :new, Class],
                 events.shift)
    assert_equal(["call", 5, :bar, Foo],
                 events.shift)
    assert_equal(["return", 6, :bar, Foo],
                 events.shift)
    assert_equal(["line", 9, __method__, self.class],
                 events.shift)
    assert_equal(["c-call", 9, :clear_trace_func, Kernel],
                 events.shift)
    assert_equal([], events)
  end

  def test_return # [ruby-dev:38701]
    events = []
    eval <<-EOF.gsub(/^.*?: /, "")
     1: add_trace_func(Proc.new { |event, file, lineno, mid, binding, klass|
     2:   events << [event, lineno, mid, klass]
     3: })
     4: def foo(a)
     5:   return if a
     6:   return
     7: end
     8: foo(true)
     9: foo(false)
    10: set_trace_func(nil)
    EOF
    assert_equal(["line", 4, __method__, self.class],
                 events.shift)
    assert_equal(["c-call", 4, :method_added, Module],
                 events.shift)
    assert_equal(["c-return", 4, :method_added, Module],
                 events.shift)
    assert_equal(["line", 8, __method__, self.class],
                 events.shift)
    assert_equal(["call", 4, :foo, self.class],
                 events.shift)
    assert_equal(["line", 5, :foo, self.class],
                 events.shift)
    assert_equal(["return", 5, :foo, self.class],
                 events.shift)
    assert_equal(["line", 9, :test_return, self.class],
                 events.shift)
    assert_equal(["call", 4, :foo, self.class],
                 events.shift)
    assert_equal(["line", 5, :foo, self.class],
                 events.shift)
    assert_equal(["return", 7, :foo, self.class],
                 events.shift)
    assert_equal(["line", 10, :test_return, self.class],
                 events.shift)
    assert_equal(["c-call", 10, :set_trace_func, Kernel],
                 events.shift)
    assert_equal([], events)
  end

  def test_return2 # [ruby-core:24463]
    events = []
    eval <<-EOF.gsub(/^.*?: /, "")
     1: set_trace_func(Proc.new { |event, file, lineno, mid, binding, klass|
     2:   events << [event, lineno, mid, klass]
     3: })
     4: def foo
     5:   a = 5
     6:   return a
     7: end
     8: foo
     9: set_trace_func(nil)
    EOF
    assert_equal(["line", 4, __method__, self.class],
                 events.shift)
    assert_equal(["c-call", 4, :method_added, Module],
                 events.shift)
    assert_equal(["c-return", 4, :method_added, Module],
                 events.shift)
    assert_equal(["line", 8, __method__, self.class],
                 events.shift)
    assert_equal(["call", 4, :foo, self.class],
                 events.shift)
    assert_equal(["line", 5, :foo, self.class],
                 events.shift)
    assert_equal(["line", 6, :foo, self.class],
                 events.shift)
    assert_equal(["return", 7, :foo, self.class],
                 events.shift)
    assert_equal(["line", 9, :test_return2, self.class],
                 events.shift)
    assert_equal(["c-call", 9, :set_trace_func, Kernel],
                 events.shift)
    assert_equal([], events)
  end

  def test_raise
    events = []
    eval <<-EOF.gsub(/^.*?: /, "")
     1: set_trace_func(Proc.new { |event, file, lineno, mid, binding, klass|
     2:   events << [event, lineno, mid, klass]
     3: })
     4: begin
     5:   raise TypeError, "error"
     6: rescue TypeError => $e
     7: end
     8: set_trace_func(nil)
    EOF
    assert_equal(["line", 4, __method__, self.class],
                 events.shift)
    assert_equal(["line", 5, __method__, self.class],
                 events.shift)
    assert_equal(["c-call", 5, :raise, Kernel],
                 events.shift)
    assert_equal(["c-call", 5, :exception, Exception],
                 events.shift)
    assert_equal(["c-call", 5, :initialize, Exception],
                 events.shift)
    assert_equal(["c-return", 5, :initialize, Exception],
                 events.shift)
    assert_equal(["c-return", 5, :exception, Exception],
                 events.shift)
    assert_equal(["c-call", 5, :backtrace, Exception],
                 events.shift)
    assert_equal(["c-return", 5, :backtrace, Exception],
                 events.shift)
    assert_equal(["c-call", 5, :set_backtrace, Exception],
                 events.shift)
    assert_equal(["c-return", 5, :set_backtrace, Exception],
                 events.shift)
    assert_equal(["raise", 5, :test_raise, $e], 
                 events.shift)
    assert_equal(["c-return", 5, :raise, Kernel],
                 events.shift)
    assert_equal(["c-call", 6, :===, Module],
                 events.shift)
    assert_equal(["c-return", 6, :===, Module],
                 events.shift)
    assert_equal(["line", 8, __method__, self.class],
                 events.shift)
    assert_equal(["c-call", 8, :set_trace_func, Kernel],
                 events.shift)
    assert_equal([], events)
  end

  def chunk(list, char)
    sep = char * 30 + "\n"
    sep + list.map{|e| e.join(' ')}.join("\n") + "\n"
  end

  def showit(actual, expected)
    chunk(actual, '-') + chunk(expected, '=')
  end

  def test_break # [ruby-core:27606] [Bug #2610]
    events = []
    eval <<-EOF.gsub(/^.*?: /, "")
     1: set_trace_func(Proc.new { |event, file, lineno, mid, binding, klass|
     2:   events << [event, lineno, mid, klass]
     3: })
     4: [1,2,3].any? {|n| n}
     8: set_trace_func(nil)
    EOF

    expected = [["line", 4, __method__, self.class],
                ["c-call", 4, :any?, Enumerable],
                ["c-call", 4, :each, Array],
                ["line", 4, __method__, self.class],
                ["c-return", 4, :any?, Enumerable],
                ["line", 5, __method__, self.class],
                ["c-call", 5, :set_trace_func, Kernel]]
    events.each_with_index{|e, i|
      assert_equal(e, events[i], showit(events, expected))}
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
      events[:set] << [event, lineno, mid, klass, :set]
    }
    prc2 = Proc.new { |event, file, lineno, mid, binding, klass|
      events[:add] << [event, lineno, mid, klass, :add]
    }

    th = Thread.new do
      th = Thread.current
      eval <<-EOF.gsub(/^.*?: /, "")
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

    expected = [["c-return", 1, :set_trace_func, Thread, :set],
                ["line", 2, __method__, self.class, :set],
                ["c-call", 2, :add_trace_func, Thread, :set]]
    expected.each do |e|
      assert_equal(e, events[:set].shift, showit(events, expected))
    end

    [["c-return", 2, :add_trace_func, Thread],
     ["line", 3, __method__, self.class],
     ["c-call", 3, :inherited, Class],
     ["c-return", 3, :inherited, Class],
     ["class", 3, nil, nil],
     ["line", 4, nil, nil],
     ["c-call", 4, :method_added, Module],
     ["c-return", 4, :method_added, Module],
     ["end", 7, nil, nil],
     ["line", 8, __method__, self.class],
     ["c-call", 8, :new, Class],
     ["c-call", 8, :initialize, BasicObject],
     ["c-return", 8, :initialize, BasicObject],
     ["c-return", 8, :new, Class],
     ["call", 4, :foo, ThreadTraceInnerClass],
     ["line", 5, :foo, ThreadTraceInnerClass],
     ["c-call", 5, :+, Fixnum],
     ["c-return", 5, :+, Fixnum],
     ["return", 6, :foo, ThreadTraceInnerClass],
     ["line", 9, __method__, self.class],
     ["c-call", 9, :set_trace_func, Thread]].each do |e|
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
