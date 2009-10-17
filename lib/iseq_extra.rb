require_relative %w(.. ext thread_frame)
class RubyVM::InstructionSequence
  def format_args
    args = 0.upto(arity-1).map do |i| 
      local_name(i)
    end.join(', ')
    
    last = local_table_size-1
    if last > arity
      args += '; ' + args = arity.upto(last).map do |i| 
        local_name(i)
      end.join(', ')
    else
      args = '?'
    end
  end

  # Basically hash.invert but since each offset can represent many
  # lines, we have to double loop. FIXME: Is there a more efficient way?
  def lineoffsets
    result = {}
    offsetlines.each do |offset, lines|
      lines.each do |line|
        result[line] ||= []
        result[line] << offset
      end
    end
    result
  end
end

if __FILE__ == $0
  iseq = RubyVM::ThreadFrame.current.iseq
  puts iseq.format_args
  puts iseq.disassemble
  puts iseq.lineoffsets
  # Demo it.
end

