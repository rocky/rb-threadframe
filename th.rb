require_relative %w(ext thread_frame)
a = 5
tf = RubyVM::Frame.new(Thread::current) # Same as RubyVM::Frame.current
puts tf.source_location.inspect
puts tf.method
puts tf.source_container.inspect
puts tf.iseq
puts tf.arity
puts tf.iseq.disasm
first_time = true
offset_1 = RubyVM::Frame.current.pc_offset
puts "Current PC offset %d\n" % offset_1
puts "Current PC offset is now %d\n" % RubyVM::Frame.current.pc_offset
if first_time
  puts "Trying again...."
  first_time = false
  RubyVM::Frame.current.pc_offset = offset_1 
  puts "pc_offset did not take. Perhaps pc is saved in local variable/register?"
end
p tf, tf.prev, tf.self, tf.binding, eval('a', tf.binding), '-' * 10
def foo(x)
  a = 6
  tf = RubyVM::Frame.current
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
