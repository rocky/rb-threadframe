require 'test/unit'
require_relative '../../lib/iseq_extra'

class TestLibISeqExtra < Test::Unit::TestCase

  def test_basic
    tf = RubyVM::Frame.get
    iseq = tf.iseq
    # See that we get the same line numbers
    assert_equal(iseq.offsetlines.values.flatten.uniq.sort,
                 iseq.lineoffsets.keys.sort)
    # See that we get the same offsets
    assert_equal(iseq.lineoffsets.values.flatten.uniq.sort,
                 iseq.offsetlines.keys.sort)

    assert_equal(iseq.lineoffsets[__LINE__].sort,
                 iseq.line2offsets(__LINE__-1).sort)

    assert_equal([], iseq.line2offsets(__LINE__+100))

    iseq2 = tf.iseq
    # Different object ids...
    assert_not_equal(iseq.object_id, iseq2.object_id)
    #    but same SHA1s..
    assert_equal(iseq.sha1, iseq2.sha1)
    #    and equal instruction sequences
    assert_equal(true, iseq.equal?(iseq2))

    # Now try two different iseqs
    tf2 = tf.prev
    tf2 = tf2.prev until !tf2.prev || tf2.prev.iseq
    if tf2
      assert_not_equal(iseq.sha1, tf2.iseq.sha1)
      assert_equal(false, iseq.equal?(tf2.iseq))
    end
  end

  def test_iseq_type
    tf = RubyVM::Frame.get
    top_iseq = tf.prev(-1).iseq
    assert_equal('METHOD', tf.prev.type)
  end

  def test_format_args
    # These prototypes are from IRB
    def evaluate(context, statements, file = __FILE__, line = __LINE__); end
    def def_extend_command(cmd_name, load_file, *aliases); end

    assert_equal('context, statements; file, line',
                 method(:evaluate).iseq.format_args)
    assert_equal('cmd_name, load_file; aliases',
                 method(:def_extend_command).iseq.format_args)
  end

  def test_sha1
    sha1 = proc{ 5 }.iseq.sha1
    assert_equal(40, sha1.size)
    assert_equal(0, sha1 =~ /^[0-9a-f]+$/)
  end

  def test_iseq_parent
    parent_iseq = RubyVM::Frame::get.iseq
    1.times do
      tf = RubyVM::Frame::get
      assert_equal(true, tf.iseq.parent.equal?(parent_iseq))
    end
  end

end
