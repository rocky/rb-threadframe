require_relative %w(.. ext thread_frame)
class RubyVM::InstructionSequence
  def format_args
    if iseq
      args = 0.upto(iseq.arity-1).map do |i| 
        iseq.local_name(i)
      end.join(', ')

      last = iseq.local_table_size-1
      if last > iseq.arity
        args += '; ' + args = iseq.arity.upto(last).map do |i| 
          iseq.local_name(i)
        end.join(', ')
      end
    else
      args = '?'
    end
  end
end
if __FILE__ == $0
  # Demo it.
end

