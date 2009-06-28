require_relative File.join('ext', 'thread_frame')
a = 5
tf = Thread::Frame.new(Thread::current) # Same as Thread::Frame.current
# puts th.disasm
puts tf.iseq
puts tf.iseq.disasm
p tf, tf.prev, tf.self, tf.binding, eval('a', tf.binding), '-' * 10
def foo()
  a = 6
  tf = Thread::Frame.current
  puts tf.iseq.disasm
  while tf do
    begin
      p tf, tf.prev, tf.self, tf.binding, eval('a', tf.binding), '-' * 10
    rescue
    end
    tf = tf.prev
  end
end
foo
p tf, tf.prev, tf.self, tf.binding, eval('a', tf.binding), '-' * 10
