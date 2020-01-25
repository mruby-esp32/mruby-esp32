require 'rubygems'
require 'RMagick'

#convert image.png -colors 64 -depth 2 +dither image3.png
#convert -resize 640x350! test2a.jpg test2c.jpg

if ARGV.size != 2
  puts "please set an input file path and an output file path."
  exit
end
input_fname = ARGV[0]
output_fname = ARGV[1]
image = Magick::Image.read(input_fname).first

height = image.rows
width  = image.columns
puts "#{input_fname} > w:#{width} h:#{height}"
exit if height > 256*256
exit if width > 256*256

header = 0xFFFFFFFF

File.open(output_fname,"wb") do |fp|
  fp.write([ header ].pack("V")) #pack uint32_t
  fp.write([ width  ].pack("v")) #pack uint16_t little:v  big:n
  fp.write([ height ].pack("v"))

  (0...height).each do |y| 
    (0...width).each do |x| 
      pixel = image.pixel_color(x, y) 

      a = 0 #alpha
      r = (pixel.red*3 / 65535.0).round
      g = (pixel.green*3 / 65535.0).round
      b = (pixel.blue*3 / 65535.0).round

      val = (a << 6) | (b << 4) | (g << 2) | r
      #puts "#{pixel.red},#{pixel.green},#{pixel.blue}"
      #puts "#{r},#{g},#{b}"
      fp.write([val].pack("C"))
    end 
  end

end

puts "Done"


