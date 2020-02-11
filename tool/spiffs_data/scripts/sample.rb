puts "Sample script"
include Narya

loop do
  Display::clear
  Display::font_size(8,8)
  Display::draw_text(30,30,"Hello mruby!","303");
  Display::swap

  k = Input::get_key
  if k==Key::K_RETURN
    break
  end
  
  ESP32::System::delay 10
end

puts "End of script"

