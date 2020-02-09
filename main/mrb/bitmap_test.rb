R"( #"
include Narya

def test
  Display.draw_picture(0,0,"/spiffs/assets/topimage.img")
  Display.swap
end

def test2
  test
end

#test2
#puts "Draw Done"

def wait_key
  loop do
    if Input::available
      if Input::keydown?(125)
        break
      end
      ESP32::System::delay 10
    end
  end
end


Display::clear
img = Bitmap.new
img.load("/spiffs/sample/test.img")
p img.width
p img.height
img.draw(20,20)


puts "Done1"
Display::swap
puts "Done2"

ESP32::System::delay 3000

sp = Sprite.new(img)
sp.move_to(0,100)

cnt=0

Display::clear
Display::font_size(8,8)
Display::draw_text(30,30,"Hello mruby!","303");
Display::font_size(8,14)
Display::draw_text(30,60,"Hello mruby!","303","021");
Display::draw_circle(100,100,8,"WHITE")
Display::swap
Display::clear
Display::font_size(8,8)
Display::draw_text(30,30,"Hello mruby!","303");
Display::font_size(8,14)
Display::draw_text(30,60,"Hello mruby!","303","021");
Display::draw_circle(100,100,8,"WHITE")
Display::swap

100.times do
  Display.scroll(10,10)
  Display::swap
  ESP32::System::delay 100
end

loop do
  Display::clear
  Display::font_size(8,8)
  Display::draw_text(30,30,"Hello mruby!","303");
  Display::font_size(8,14)
  Display::draw_text(30,60,"Hello mruby!","303","021");
  #img.draw(20,20)
  sp.move(3,0)
  sp.move_to(0,100) if cnt % 20 == 0
  #Display.draw_picture(20,20,"/test.img")
  Display::draw_circle(100,100,8,"WHITE")
  Display::swap
  k = Input::get_key
  if k==Key::K_RETURN
    break
  end
  cnt+=1
  ESP32::System::delay 10
end

)"