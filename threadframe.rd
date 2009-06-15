# = NAME  
# +ThreadFrame+ and +RubyVM+::+ThreadFrame+
#
# = SYNOPSES 
#
# The +ThreadFrame+ and +RubyVM+::+ThreadFrame+ class gives call-stack
# frame information (controlled access to +rb_control_frame_t+)
#
# The +ThreadFrame+ class is proposed for all Ruby 1.9 implementations
# +RubyVM+::+ThreadFrame+ contains routines in the YARV 1.9 implementation.
# Other implementations may or may not also implement some of these.
# So +RubyVM+::+ThreadFrame+ is a subclass of +ThreadFrame+. In code:
#
#   class ThreadFrame  # In all 1.9 implementations
#      def prev
#         # implementation of prev
#      end
#      # ...
#   end
#   class RubyVM::ThreadFrame < ThreadFrame # In YARV 
#     def iseq
#       # implementation of iseq
#     end 
#     # ...  
#   end
#
# A +ThreadFrame+ contains information from a frame running +Thread+
# object, and the information may change or disappear in the course of
# running that thread. Therefore, it is advisable ensure that the
# threads of the ThreadFrame objects are blocked.
#
# Although +ThreadFrame+ and +RubyVM+::+ThreadFrame+ are classes, you
# probably will not have need to ever use a +new+ constructor, and
# possibly such constructor methods will not exist. Instead,
# +ThreadFrame+ objects get created in other ways such as through the
# +Thread+#+threadframe+ instance method.
#
#
# === Thread#threadframe
#   tf = Thread::current.threadframe() 
# Returns thread frame of currently executed program.
#
# == ThreadFrame Instance Methods
# 
# === ThreadFrame#prev
#   tf.prev() -> tf or nil
#
# Returns the previous control frame. If tail recursion has removed
# frames as seen in the source, deal with it. ;-) +nil+ can be
# returned if there is no previous frame or if tf is no longer exists.
#
# === ThreadFrame#exist?
#   tf.exist?() -> boolean
#
# Returns true if tf is still a currently active frame. Beware: if the
# tf.thread is not blocked, it is possible that between the time the
# frame was created and when it is subsequently used in execution
# of the program causes the frame to no longer exist. 
# 
#
# === ThreadFrame#thread
#   tf.thread() -> Thread
#   RubyVM::ThreadFrame.current().thread == Thread.current
#
# === ThreadFrame#type
#   tf.type() -> 'C' or 'Ruby'
#
# Indicates whether the frame is implemented in C or Ruby.
#
# === tf.source_location
#  tf.source_location() -> Array 
#
# Returns an array of source location positions that match
# +tf.instruction_offset+. A source location position is left
# implementation dependent. It could be line number, a line number
# and start and end column, or a start line number, start column, end
# line number, end column.
# 
# == RubyVM::ThreadFrame Instance Methods
#
# === RubyVM::ThreadFrame#iseq
#   tf.iseq() -> ISeq
#
# Returns an instruction sequence object from the instruction sequence
# found inside the +ThreadFrame+ object or +nil+ if there is none.
# But if iseq is +nil+ and tf.type is not C, +binding+, and
# +instruction_offset+, and +source_location+ are probably meaningless
# and will be +nil+ as well.
#
#
# === RubyVM::ThreadFrame#instruction_offset
#  tf.instruction_offset -> Fixnum
# Offset inside ISeq of instruction that the frame is currently on.
#
# === RubyVM::ThreadFrame#instruction_offset=
#
#  tf.instruction_offset=(Fixnum)
# Sets the threadframe to a new offset. Some restrictions may apply, e.g.
# the offset will have to refer to a valid offset location and the scope
# and block level has to be the same.
#
# <em>Don't need to implement initially.</em>
#
# === RubyVM::ThreadFrame#return_changed?
#   tf.return_changed?() -> boolean
#
# Returns true if tf _may_ be part of tail recursion removal so when
# the code in frame returns it may not be to the immediate caller as
# seen in the source code. Koichi says this is too difficult to do in
# YARV.
#
# <em>Don't need to implement initially.</em>
