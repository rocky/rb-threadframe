require_relative File.join('ext', 'thread_frame')
a = 5
th = ThreadFrame.current
puts th.disasm
p th, th.prev, th.self, th.binding, eval('a', th.binding), '-' * 10
def foo()
  a = 6
  th = ThreadFrame.current
  puts th.disasm
  while th do
    p th, th.prev, th.self, th.binding, eval('a', th.binding), '-' * 10
    th = th.prev
  end
end
foo
p th, th.prev, th.self, th.binding, eval('a', th.binding), '-' * 10
