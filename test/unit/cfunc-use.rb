def cfunc_loc
  ftype, file = [nil, nil]
  1.times do
    1.times do
      f = RubyVM::Frame::get(1)
      ftype = f.type
      file  = f.source_container[1]
    end
  end
  return [ftype, file]
end
