require 'digest/sha1'
require_relative '../ext/thread_frame'
# Some additions to RubyVM::InstructionSequence
class RubyVM::InstructionSequence

  # Turns a instruction sequence type field into a string name
  TYPE2STR = %w(top method block class rescue ensure eval main guard)

  # Returns a String containing a list of arguments for the RubyVM::InstructionSequence
  # A semicolon separates required arguments from optional ones.
  # For example: for 
  #  def evaluate(context, statements, file = __FILE__, line = __LINE__)
  # we return:
  #  context, statements; file, line
  def format_args
    required_max = arity < 0 ? -arity-1 : arity
    args = 0.upto(required_max-1).map do |i| 
      local_name(i)
    end.join(', ')
    
    last = local_table_size-1
    if last >= required_max
      opt_args = required_max.upto(last).map do |i| 
        local_name(i)
      end.join(', ')
      args += '; ' + opt_args
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

  # Return An array of VM instruction byte offsets (Fixnums) for a
  # given line_number.
  def line2offsets(line_number)
    offsetlines.select do |offset, lines| 
      lines.member?(line_number) 
    end.keys
  end

  # Returns a cryptographic checksum (in particluar a SHA1) for the
  # encoded bytes of the instruction sequence.
  # 
  # ==== Example
  # 
  #   proc{ 5 }.iseq.sha1 => 'b361a73f9efd7dc4d2c5e86d4e94d40b36141d42'
  def sha1
    Digest::SHA1.hexdigest(encoded)
  end

end

if __FILE__ == $0
  # Demo it.
  iseq = RubyVM::ThreadFrame.current.iseq
  puts iseq.format_args
  puts iseq.disassemble
  puts iseq.lineoffsets
  puts iseq.sha1
  p iseq.line2offsets(__LINE__)
  p iseq.line2offsets(__LINE__+100)

  def show_type # :nodoc:
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

