require_relative %w(ext thread_frame)
a = 5
tf = RubyVM::ThreadFrame.new(Thread::current) # Same as RubyVM::ThreadFrame.current
puts tf.source_location.inspect
puts tf.method
puts tf.source_container.inspect
puts tf.iseq
puts tf.arity
puts tf.iseq.disasm
p tf, tf.prev, tf.self, tf.binding, eval('a', tf.binding), '-' * 10
def foo(x)
  a = 6
  tf = RubyVM::ThreadFrame.current
  puts "arity: #{tf.arity}"
  puts tf.iseq.disasm
  while tf do
    begin
      p tf, tf.prev, tf.self, tf.binding, eval('a', tf.binding), '-' * 10
    rescue
    end
    tf = tf.prev
  end
end
foo(5)
p tf, tf.prev, tf.self, tf.binding, eval('a', tf.binding), '-' * 10
