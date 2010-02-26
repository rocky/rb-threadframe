require_relative %w(.. ext thread_frame)
class RubyVM::InstructionSequence

  # Turns a instruction sequence type field into a string name
  TYPE2STR = %w(top method block class rescue ensure eval main guard)
  
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

  def line2offsets(line_number)
    offsetlines.select do |offset, lines| 
      lines.member?(line_number) 
    end.keys
  end
end

if __FILE__ == $0
  # Demo it.
  iseq = RubyVM::ThreadFrame.current.iseq
  puts iseq.format_args
  puts iseq.disassemble
  puts iseq.lineoffsets
  p iseq.line2offsets(__LINE__)
  p iseq.line2offsets(__LINE__+100)

  def show_type
    tf = RubyVM::ThreadFrame.current
    while tf do
      is = tf.iseq
      if is
        ist = tf.iseq.type
        isn = RubyVM::InstructionSequence::TYPE2STR[ist]
        puts "instruction sequence #{is.inspect} has type #{isn} (#{ist})."
      end
      tf = tf.prev
    end
  end
  show_type
end

