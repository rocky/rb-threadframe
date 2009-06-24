# = NAME  
# +Thread+::+Frame+ and +RubyVM+::+ThreadFrame+
#
# = SYNOPSES 
#
# The +Thread+::+Frame+ and +RubyVM+::+ThreadFrame+ class gives call-stack
# frame information (controlled access to +rb_control_frame_t+)
#
# The +ThreadFrame+ class is proposed for all Ruby 1.9 implementations
# while +RubyVM+::+ThreadFrame+ contains routines in the YARV 1.9 implementation.
# Other implementations may or may not also implement some of these.
# So +RubyVM+::+ThreadFrame+ is a subclass of +Thread+::+Frame+. In code:
#
#   class Thread
#     class Frame  # In all 1.9 implementations
#        def initialize(thread_object)
#          # implementation-specific code
#        end    
#        def prev
#          # implementation of prev
#        end
#        # ...
#     end
#     def threadframe        
#        Frame.new(self) # or RubyVM::ThreadFrame.new(self)
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
# A +Thread+::+Frame+ contains information from a frame running +Thread+
# object, and the information may change or disappear in the course of
# running that thread. Therefore, it is advisable ensure that the
# threads of the ThreadFrame objects are blocked.
#
#
# === Thread::Frame::new(thread_object)
# Creates a thread-frame object for the given thread object.
#
# === Thread#threadframe
#   tf = Thread::current.threadframe() 
#
# Creates a thread-frame object for the given thread object.
# Note:
#  Thread::current.threadframe() == Thread::Frame.new(Thread::current)
#
# == Thread::Frame Instance Methods
# 
# === Thread::Frame#prev
#   tf.prev() -> tf or nil
#
# Returns the previous control frame. If tail recursion has removed
# frames as seen in the source, deal with it. ;-) +nil+ can be
# returned if there is no previous frame or if tf is no longer exists.
#
# === Thread::Frame#exist?
#   tf.exist?() -> boolean
#
# Returns true if tf is still a currently active frame. Beware: if the
# tf.thread is not blocked, it is possible that between the time the
# frame was created and when it is subsequently used in execution
# of the program causes the frame to no longer exist. 
# 
#
# === Thread::Frame#thread
#   tf.thread() -> Thread
#   Thread::Frame.current().thread == Thread.current
#
# === Thread::Frame#type
#   tf.type() -> 'C' or 'Ruby'
#
# Indicates whether the frame is implemented in C or Ruby.
#
# === Thread::Frame#source_container
#  tf.source_container() -> [Type, String]
#
# Returns a tuple representing kind of container, e.g. file
# eval'd string object, and the name of the container. If file,
# it would be a file name. If an eval'd string it might be the string.
# 
# === Thread::Frame#source_location
#  tf.source_location() -> Array 
#
# Returns an array of source location positions that match
# +tf.instruction_offset+. A source location position is left
# implementation dependent. It could be line number, a line number
# and start and end column, or a start line number, start column, end
# line number, end column.

# === Thread::Frame#binding
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

# Like Thread::Frame.new(thread_object), but has additional information
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
