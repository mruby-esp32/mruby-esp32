R"(  #"
include Narya
class WalkMap
  def initialize
    @pos_x = 0
    @pos_y = 0
    @map = [
      [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,],
      [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,],
      [0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,],
      [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,],
      [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,],
      [0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,],
      [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,],
      [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,],
      [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,],
      [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,],
      [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,],
      [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,],
      [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,],
      [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,],
      [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,],
      [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,],
      [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,],
      [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,],
      [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,],
      [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,]
     ]
     @max_x = @map[0].size
     @max_y = @map.size
     @bitmap_list = []
     @bitmap_list << Bitmap.new.load("/grass.img")
     @bitmap_list << Bitmap.new.load("/road.img")
     @bitmap_list << Bitmap.new.load("/water.img")
     @bitmap_list << Bitmap.new.load("/castle.img")
     @character_list = []
     @character_list << Bitmap.new.load("/man.img")

     @offset_x = 40
     @offset_y = 20
  end
  
  def set(x,y)
    @pos_x = x
    @pos_y = y
  end

  def move(k)
    case k
    when 1 #UP
      @pos_y = @pos_y - 1
    when 2 #DOWN
      @pos_y = @pos_y + 1
    when 3 #LEFT
      @pos_x = @pos_x - 1
    when 4 #RIGHT
      @pos_x = @pos_x + 1
    end
    @pos_x = 0 if @pos_x < 0
    @pos_x = @max_x - 1 if @pos_x >= @max_x
    @pos_y = 0 if @pos_y < 0
    @pos_y = @max_y - 1 if @pos_y >= @max_y
    puts " pos( #{@pos_x},#{@pos_y} ) "
  end

  def draw
    (0...9).each do |y|
      (0...9).each do |x|
        sx = @pos_x + x - 6
        sy = @pos_y + y - 6
        chip = 0
        if(sx<0 or sx>@max_x or sy<0 or sy>@max_y)
          chip = 0
        else
          chip = @map[sx][sy]
        end
        @bitmap_list[chip].draw(x*16+@offset_x ,y*16+@offset_y)
      end
    end
    @character_list[0].draw(4*16+@offset_x ,4*16+@offset_y)
  end

end

world_map = WalkMap.new

def get_key
  return 0 unless Input::available
  k = 0
  #151 UP, 153 DOWN, 155 LEFT, 157 RIGHT
  k = 1 if Input::keydown?(151)
  k = 2 if Input::keydown?(153)
  k = 3 if Input::keydown?(155)
  k = 4 if Input::keydown?(157)
  k = -1 if Input::keydown?(125) #ESC
  return k
end


world_map.set(5,5) 
Display::clear
world_map.draw
Display::swap

loop do
  k = get_key
  if k>0
    world_map.move(k) 
    Display::clear
    world_map.draw
    Display::swap
    ESP32::System::delay 150
  end
  break if k<0
  ESP32::System::delay 10
end

)";