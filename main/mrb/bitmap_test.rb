R"(
include Narya

def test
  Display.draw_picture(0,0,"/bktest.img")
end

def test2
  test
end

test2

sleep 1

img = Bitmap.new
img.load("/test.img")
p img.width
p img.height
img.draw

loop do
  if Input::available
    if Input::keydown?(125)
      break
    end
  end
end

)"