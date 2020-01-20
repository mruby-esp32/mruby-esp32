require 'rubygems'
require 'RMagick'

image = Magick::Image.read('spiffs_data/image3.jpg').first

p image.rows
p image.columns
(0..image.rows).each do |y| 
  (0..image.columns).each do |x| 
   pixel = image.pixel_color(x, y) 

  r = (pixel.red*3 / 65535.0).round
  g = (pixel.green*3  / 65535.0).round
  b = (pixel.blue*3  / 65535.0).round
  val = (b << 4) | (g << 2) | r
  #p val
  end 
end 

