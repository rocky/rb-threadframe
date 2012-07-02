# = NAME  
# +RubyVM+::+Frame+
#
# = SYNOPSES 
#
# The +RubyVM+::+Frame+ class gives call-stack frame information
# (controlled access to +rb_control_frame_t+)
#
#
# It is possible that a The +Thread+::+Frame+ may be proposed for
# all Ruby 1.9 implementations.  +RubyVM+::+Frame+
# contains routines in the YARV 1.9 implementation.
#
# Should there be a +Thread+::+Frame+ +RubyVM+::+Frame+ would be
# a subclass of +Thread+::+Frame+. In code:
#
#   class Thread
#     class Frame  # In all 1.9 implementations
#        # ...
#     end
#     def threadframe        
#        RubyVM::Frame.new(self) # or possibly Thread::Frame.new(self)
#     end
#   end
#           
#   class RubyVM::Frame < Thread::Frame # In YARV 
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
# threads of the Frame objects are blocked.
#
#
# === RubyVM::Frame::new(thread_object)
# Creates a thread-frame object for the given thread object.
#
# === RubyVM::Frame::current
# Shorthand for RubyVM::Frame.new(Thread::current)
#
# === Thread#threadframe
#   tf = Thread::current.threadframe() 
#
# Creates a thread-frame object for the given thread object.
# Note:
#  Thread::current.threadframe() == RubyVM::Frame.new(Thread::current)
#
# == RubyVM::Frame Instance Methods
# 
# === RubyVM::Frame#prev
#   tf.prev(n) -> tf or nil
#   tf.prev() -> tf or nil  # same as tf.prev(1)
#
# Returns the previous control frame. If tail recursion has removed
# frames as seen in the source, deal with it. ;-) +nil+ can be
# returned if there is no previous frame or if tf is no longer exists.
# If a number is passed go back that many frames. The value 0 gives back
# tf. A negative number of a number greater than the number of frames
# returns nil. 
#
# === RubyVM::Frame#invalid?
#   tf.invalid?() -> boolean
#
# Returns true if the frame is no longer valid. On the other hand,
# since the test we use is weak, returning false might not mean the
# frame is valid, just that we can't disprove that it is not invalid.
# 
# It is suggested that frames are used in a way that ensures they will
# be valid. In particular frames should have local scope and frames to 
# threads other than the running one should be stopped while the frame 
# variable is active.
# 
#
# === RubyVM::Frame#thread
#   tf.thread() -> Thread
#   RubyVM::Frame.current().thread == Thread.current
#
# === RubyVM::Frame#type
#   tf.type() -> 'C' or 'Ruby'
#
# Indicates whether the frame is implemented in C or Ruby.
#
# === RubyVM::Frame#source_container
#  RubyVM::Threadframe#source_container() -> [Type, String]
#
# Returns a tuple representing kind of container, e.g. file
# eval'd string object, and the name of the container. If file,
# it would be a file name. If an eval'd string it might be the string.
# 
# === RubyVM::Frame#source_location
#  RubyVM::Frame#.source_location() -> Array 
#
# Returns an array of source location positions that match
# +tf.instruction_offset+. A source location position is left
# implementation dependent. It could be line number, a line number
# and start and end column, or a start line number, start column, end
# line number, end column.
#
# === RubyVM::Frame#stack_size
#  RubyVM::Frame#.stack_size -> Fixnum
#
#  Returns the number of entries 
#
# === RubyVM::Frame#binding
#  tf.binding() -> binding
#
#
# If the frame is still valid, a binding that is in effect for the
# scope of the frame is created. As with all bindings, over the
# execution of the program, variables may spring into existence and
# values may change.

# == RubyVM::Frame 
#
# === RubyVM::new(thread_object)

# Like RubyVM::Frame.new(thread_object), but has additional information
# available.
#
# === RubyVM::Frame#iseq
#   tf.iseq() -> ISeq
#
# Returns an instruction sequence object from the instruction sequence
# found inside the +Frame+ object or +nil+ if there is none.
# But if iseq is +nil+ and tf.type is not C, +binding+, and
# +instruction_offset+, and +source_location+ are probably meaningless
# and will be +nil+ as well.
#
#
# === RubyVM::Frame#instruction_offset
#  tf.instruction_offset -> Fixnum
# Offset inside ISeq of instruction that the frame is currently on.
#
# === RubyVM::Frame#instruction_offset=
#
#  tf.instruction_offset=(Fixnum)
# Sets the threadframe to a new offset. Some restrictions may apply, e.g.
# the offset will have to refer to a valid offset location and the scope
# and block level has to be the same.
#
# <em>Don't need to implement initially.</em>
#
# === RubyVM::Frame#return_changed?
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
# RubyVM::Frame#new == Thread::current.threadframe
