R"(  #"
include Narya

class Tetrimino
  attr_accessor :x,:y,:field
  def initialize(type)
    @x = 4
    @y = 0
    @bs = 8
    @type = type
    case type
    when :I
      @field = [
        [0,0,0,0],
        [1,1,1,1],
        [0,0,0,0],
        [0,0,0,0]
      ]
      @color = "002"
    when :O
      @field = [
        [1,1,0],
        [1,1,0],
        [0,0,0],
        [0,0,0]
      ]
      @color = "002"
    when :S
      @field = [
        [0,1,1],
        [1,1,0],
        [0,0,0],
      ]
      @color = "002"
    when :Z
      @field = [
        [1,1,0],
        [0,1,1],
        [0,0,0],
      ]
      @color = "002"
    when :J
      @field = [
        [1,0,0],
        [1,1,1],
        [0,0,0],
      ]
      @color = "002"
    when :L
      @field = [
        [0,0,1],
        [1,1,1],
        [0,0,0],
      ]
      @color = "002"
    when :T
      @field = [
        [0,1,0],
        [1,1,1],
        [0,0,0],
      ]
      @color = "002"
    end
    #@h = @field.size
    #@w = @field.first.size
  end
  def exist?(x,y)
    false
  end
end

class Tetris
  def initialize
    @twidth = 10
    @theight = 20
    @field = Array.new(@theight).map{Array.new(@twidth ,0)}
    @offx = 50 # Offset X
    @offy = 2  # Offset Y
    @bs = 8 # Block Size
    @state = :init
    @last_time = 0
    @update_speed = 1000
    @current_tet = nil
  end

  def update(key)
    update = false
    puts "#{@state.to_s}"
    case @state
    when :init
      draw_field
      update = true
      @state = :start_game
    when :start_game
      @state = :run
      set_new_tetrimino
      @last_time = ESP32::System::tick_ms
    when :run
      update = run_game(key)
    when :end_game
    end
    update
  end

  def draw_tetrimino(t)
    offset_x = @offx + t.x * @bs
    offset_y = @offy + t.y * @bs
    t.field.each_with_index do |line,y|
      line.each_with_index do |v,x|
        if v==1
          Display::draw_rect(
            offset_x+@bs*x,      offset_y+@bs*y,
            offset_x+@bs*(x+1)-1,offset_y+@bs*(y+1)-1,"BLUE","CYAN")
        end
      end
    end
  end

  def draw_field
    Display.draw_rect(@offx,@offy, @offx+@bs*(@twidth+3),@offy+@bs*(@theight+4),"BLACK","BLACK")

    Display.draw_rect(@offx,@offy+@bs, @offx+@bs*4,@offy+@bs*2,"GREEN","GREEN")
    Display.draw_rect(@offx,@offy+@bs, @offx+@bs,@offy+@bs*(@theight+4),"GREEN","GREEN")
    Display.draw_rect(@offx,@offy+@bs*(@theight+3), @offx+@bs*(@twidth+3),@offy+@bs*(@theight+4),"GREEN","GREEN")
    Display.draw_rect(@offx+@bs*(@twidth+2),@offy+@bs, @offx+@bs*(@twidth+3),@offy+@bs*(@theight+4),"GREEN","GREEN")
    Display.draw_rect(@offx+@bs*(@twidth-1),@offy+@bs, @offx+@bs*(@twidth+3),@offy+@bs*2,"GREEN","GREEN")
    return if @state != :run
    draw_tetrimino(@current_tet)
  end

  def run_game(key)
    current_time = ESP32::System::tick_ms
    update = false
    if key>0
      move_tetrimino(key,@bs)
    end
    if current_time > @update_speed+@last_time
      #update tetrimino position
      @last_time = current_time
      update_tetrimino
      check_field
      draw_field
      update = true
    end
    update
  end

  def set_new_tetrimino
    case rand(7)
    when 0
      @current_tet = Tetrimino.new(:I)
    when 1
      @current_tet = Tetrimino.new(:O)
    when 2
      @current_tet = Tetrimino.new(:S)
    when 3
      @current_tet = Tetrimino.new(:Z)
    when 4
      @current_tet = Tetrimino.new(:J)
    when 5
      @current_tet = Tetrimino.new(:L)
    when 6
      @current_tet = Tetrimino.new(:T)
    end
  end

  def move_tetrimino(key)

  end

  def update_tetrimino
    @current_tet.y += 1
    puts "Tet:#{@current_tet.x},#{@current_tet.y}"
  end

  def check_field

  end

end

def get_key
  return 0 unless Input::available
  k = Key::K_NONE
  k = Key::K_DOWN  if Input::keydown?(Key::K_DOWN)
  k = Key::K_LEFT  if Input::keydown?(Key::K_LEFT)
  k = Key::K_RIGHT if Input::keydown?(Key::K_RIGHT)
  k = Key::K_SPACE if Input::keydown?(Key::K_SPACE)
  k = Key::K_a     if Input::keydown?(Key::K_a)
  k = Key::K_s     if Input::keydown?(Key::K_s)
  k = -1 if Input::keydown?(Key::K_ESCAPE)
  return k
end

tetris = Tetris.new

#BG
Display.clear
Display.draw_picture(0,0,"/sample/moscow_320_200.img")
Display::swap
Display.draw_picture(0,0,"/sample/moscow_320_200.img")
Display::swap

loop do
  k = get_key
  break if k<0
  Display::swap if tetris.update(k)
  ESP32::System::delay(1000)
end
puts "End of Script"

)"
