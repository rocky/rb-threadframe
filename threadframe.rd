# = NAME  
# +Thread+::+Frame+ and +RubyVM+::+ThreadFrame+
#
# = SYNOPSES 
#
# The +RubyVM+::+ThreadFrame+ class gives call-stack frame information
# (controlled access to +rb_control_frame_t+)
#
#
# It is possible that a The +Thread+::+Frame+ may be proposed for
# all Ruby 1.9 implementations.  +RubyVM+::+ThreadFrame+
# contains routines in the YARV 1.9 implementation.
#
# Should there be a +Thread+::+Frame+ +RubyVM+::+ThreadFrame+ would be
# a subclass of +Thread+::+Frame+. In code:
#
#   class Thread
#     class Frame  # In all 1.9 implementations
#        # ...
#     end
#     def threadframe        
#        RubyVM::ThreadFrame.new(self) # or possibly Thread::Frame.new(self)
#     end
#   end
#           
#   class RubyVM::ThreadFrame < Thread::Frame # In YARV 
#     def initialize(thread_object)
#       # implementation-specific code
#     end    
#     def iseq
#       # implementation of iseq
#     end 
#     # ...  
#   end
#
# A +RubyVM+::Thread+Frame+ contains information from a frame running +Thread+
# object, and the information may change or disappear in the course of
# running that thread. Therefore, it is advisable ensure that the
# threads of the ThreadFrame objects are blocked.
#
#
# === RubyVM::ThreadFrame::new(thread_object)
# Creates a thread-frame object for the given thread object.
#
# === RubyVM::ThreadFrame::current
# Shorthand for RubyVM::ThreadFrame.new(Thread::current)
#
# === Thread#threadframe
#   tf = Thread::current.threadframe() 
#
# Creates a thread-frame object for the given thread object.
# Note:
#  Thread::current.threadframe() == RubyVM::ThreadFrame.new(Thread::current)
#
# == RubyVM::ThreadFrame Instance Methods
# 
# === RubyVM::ThreadFrame#prev
#   tf.prev() -> tf or nil
#
# Returns the previous control frame. If tail recursion has removed
# frames as seen in the source, deal with it. ;-) +nil+ can be
# returned if there is no previous frame or if tf is no longer exists.
#
# === RubyVM::ThreadFrame#exist?
#   tf.exist?() -> boolean
#
# Returns true if tf is still a currently active frame. Beware: if the
# tf.thread is not blocked, it is possible that between the time the
# frame was created and when it is subsequently used in execution
# of the program causes the frame to no longer exist. 
# 
#
# === RubyVM::ThreadFrame#thread
#   tf.thread() -> Thread
#   RubyVM::ThreadFrame.current().thread == Thread.current
#
# === RubyVM::ThreadFrame#type
#   tf.type() -> 'C' or 'Ruby'
#
# Indicates whether the frame is implemented in C or Ruby.
#
# === RubyVM::ThreadFrame#source_container
#  tf.source_container() -> [Type, String]
#
# Returns a tuple representing kind of container, e.g. file
# eval'd string object, and the name of the container. If file,
# it would be a file name. If an eval'd string it might be the string.
# 
# === RubyVM::ThreadFrame#source_location
#  tf.source_location() -> Array 
#
# Returns an array of source location positions that match
# +tf.instruction_offset+. A source location position is left
# implementation dependent. It could be line number, a line number
# and start and end column, or a start line number, start column, end
# line number, end column.

# === RubyVM::ThreadFrame#binding
#  tf.binding() -> binding
#
#
# If the frame is still valid, a binding that is in effect for the
# scope of the frame is created. As with all bindings, over the
# execution of the program, variables may spring into existence and
# values may change.

# == RubyVM::ThreadFrame 
#
# === RubyVM::new(thread_object)

# Like RubyVM::ThreadFrame.new(thread_object), but has additional information
# available.
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
#
# = THANKS
#
# Martin Davis for suggesting 
# RubyVM::ThreadFrame#new == Thread::current.threadframe
