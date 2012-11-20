require 'digest/sha1'
require_relative '../ext/thread_frame' if '1.9.2' == RUBY_VERSION
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


  ##
  # Locates the instruction address offset of the first instruction on
  # the specified line or nil if no match for the specified line is
  # found.
  #
  # @return [Fixnum, NilClass] returns
  #   nil if nothing is found, else the first offset for the line
  def locate_line(line)
    offsetlines.each_pair do |offset, val|
      return offset if val.member?(line)
    end
    nil
  end

  # iseq and instruction address offset of the first instruction on
  # the specified line. This method recursively examines child
  # compiled methods until an exact match for the searched line is
  # found.  It returns both the matching CompiledMethod and the OFFSET
  # of the first instruction on the requested line, or nil if no match
  # for the specified line is found.
  #
  # @return [(RubyVM::InstructionSequence, Fixnum), NilClass] returns
  #   nil if nothing is found, else an array of size 2 containing the method
  #   the line was found in and the offset pointing there.
  def locate_line_with_children(line)
    iseq = self
    offset = iseq.locate_line(line)
    return iseq, offset if offset
    
    # Didn't find line in this iseq, so check if a contained
    # InstructionSequence encompasses the line searched for
    until offset
      current_iseq = iseq
      iseq = iseq.parent
      unless iseq
        # current_iseq is the top-most scope. Search down from here.
        top_iseq.child_iseqs.each do |child_iseq|
          next if child_iseq.equal? current_iseq
          if res = child_iseq.locate_line_with_children(line)
            return res
          end
        end
        # No child method is a match - fail
        return nil
      end
      offset = iseq.locate_line(line)
    end
    return parent_iseq, offset
  end

  def lines
    offsetlines.values.flatten.uniq
  end

  # Returns an InstructionSequence for the specified line. We search the
  # current method +meth+ and then up the parent scope.  If we hit
  # the top and we can't find +line+ that way, then we
  # reverse the search from the top and search down. This will add
  # all siblings of ancestors of +meth+.
  def find_iseq_with_line(line)

    lines = self.lines
    iseq = self
    until lines.member?(line) do
      child_iseq = iseq
      iseq = iseq.parent
      unless iseq
        # child is the top-most scope. Search down from here.
        pair = child_iseq.locate_line_with_children(line)
        ## pair = iseq.locate_line(line)
        return pair ? pair[0] : nil
      end
      lines = iseq.lines
    end
    return iseq
  end
end

if __FILE__ == $0
  # Demo it.
  iseq = RubyVM::Frame.current.iseq
  ## FIXME
  ## puts iseq.format_args
  puts iseq.disassemble
  puts iseq.lineoffsets
  puts iseq.sha1
  p iseq.line2offsets(__LINE__)
  p iseq.line2offsets(__LINE__+100)

  def show_type # :nodoc:
    tf = RubyVM::Frame.current
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
  puts '-' * 40
  
  line = __LINE__
  def find_line(line) # :nodoc
    tf = RubyVM::Frame.current
    puts "find_line has lines: #{tf.iseq.lines}"
    p tf.iseq.find_iseq_with_line(line)
  end

  tf = RubyVM::Frame.current
  puts tf.iseq.disassemble
  puts("offset %d above should be at line %d" % 
       [tf.iseq.locate_line(line), line])
  find_line(line+2)
  find_line(line)
  p tf.iseq.find_iseq_with_line(line+2)
end

