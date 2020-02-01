R"(  #"
include Narya

class Tetris
end



def get_key
  return 0 unless Input::available
  k = 0
  k = 1  if Input::keydown?(Key::K_UP)
  k = 2  if Input::keydown?(Key::K_DOWN)
  k = 3  if Input::keydown?(Key::K_LEFT)
  k = 4  if Input::keydown?(Key::K_RIGHT)
  k = 5  if Input::keydown?(Key::K_SPACE)
  k = -1 if Input::keydown?(Key::K_ESCAPE)
  return k
end

loop do
end


)"
