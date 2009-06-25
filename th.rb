require_relative File.join('ext', 'thread_frame')
a = 5
th = Thread::Frame.current
# puts th.disasm
puts th.iseq
puts th.iseq.disasm
p th, th.prev, th.self, th.binding, eval('a', th.binding), '-' * 10
def foo()
  a = 6
  th = Thread::Frame.current
  puts th.iseq.disasm
  while th do
    p th, th.prev, th.self, th.binding, eval('a', th.binding), '-' * 10
    th = th.prev
  end
end
foo
p th, th.prev, th.self, th.binding, eval('a', th.binding), '-' * 10
