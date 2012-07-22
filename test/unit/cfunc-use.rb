def cfunc_loc
  ftype, file = [nil, nil]
  1.times do
    1.times do
      f = RubyVM::Frame::current.prev
      ftype = f.type 
      file  = f.source_container[1]
    end
  end
  return [ftype, file]
end
