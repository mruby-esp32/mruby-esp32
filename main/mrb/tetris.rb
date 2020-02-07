R"(  #"
include Narya

class Tetrimino
  attr_accessor :x,:y
  attr_accessor :field,:type
  def initialize(type)
    @x = 4
    @y = 0
    @type = type
    @r = 0
    case type
    when :I
      set_I
      @field = @array_I[@r]
      @color = "002"
    when :O
      @field = [
        [1,1,0],
        [1,1,0],
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
  end

  def set_I
    @array_I = []
    @array_I << [
      [0,0,0,0],
      [1,1,1,1],
      [0,0,0,0],
      [0,0,0,0]
    ]
    @array_I << [
      [0,0,1,0],
      [0,0,1,0],
      [0,0,1,0],
      [0,0,1,0]
    ]
    @array_I << [
      [0,0,0,0],
      [0,0,0,0],
      [1,1,1,1],
      [0,0,0,0]
    ]
    @array_I << [
      [0,1,0,0],
      [0,1,0,0],
      [0,1,0,0],
      [0,1,0,0]
    ]
  end

  def rotate(dir)
    case type
    when :I
      if dir>0
        @r += 1
      else
        @r -= 1
      end
      @r = 0 if @r > 3
      @r = 3 if @r < 0
      @field = @array_I[@r]
    when :S,:Z,:J,:L,:T
      temp = @field.map(&:dup)
      @field[0][0] = temp[2][0]
      @field[0][1] = temp[1][0]
      @field[0][2] = temp[0][0]
      @field[1][2] = temp[0][1]
      @field[2][2] = temp[0][2]
      @field[2][1] = temp[1][2]
      @field[2][0] = temp[2][2]
      @field[1][0] = temp[2][1]
    when :O
    end
  end
end

class Tetris
  def initialize
    @twidth = 10+2
    @theight = 20+4
    @field = Array.new(@theight).map{Array.new(@twidth ,:E)}
    @offx = 50 # Offset X
    @offy = 2  # Offset Y
    @bs = 8 # Block Size
    @state = :init
    @last_time = 0
    @update_speed = 500
    @current_tet = nil
  end

  def update(key)
    update = false
    case @state
    when :init
      init_field
      draw_field
      @state = :start_game
      update = true
    when :start_game
      @state = :run
      set_new_tetrimino
      @last_time = ESP32::System::tick_ms
      draw_tetrimino(@current_tet)
      update = true
    when :run
      update = run_game(key)
    when :close_game
      init_field
      draw_field
      @state = :start_game
      update = true
    end
    update
  end

  def init_field
    # E: Empty
    # W: Wall
    (0...@theight).each{|y|(0...@twidth).each{|x|@field[y][x]=:E}}
    (0...4).each{|x| @field[2][x]=:W }
    (8...@twidth).each{|x| @field[2][x]=:W }
    (0...@twidth).each{|x| @field[@theight-1][x]=:W }
    (1...@theight).each{|y| @field[y][0]=:W }
    (1...@theight).each{|y| @field[y][@twidth-1]=:W }
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
    #puts "--dr1:#{ESP32::System::tick_ms}"
    Display.draw_rect(@offx,@offy, @offx+@bs*@twidth-1,@offy+@bs*@theight-1,"BLACK","BLACK")
    @field.each_with_index do |line,y|
      line.each_with_index do |v,x|
        if v == :E
          next
        end
        color1 = color2 ="BLACK"
        case v
        when :W
          color1 = color2 = "GREEN"
        when :I,:O,:S,:Z,:J,:L,:T
          color1 = "BLUE"
          color2 = "CYAN"
        end
        Display.draw_rect(@offx+@bs*x,@offy+@bs*y, @offx+@bs*(x+1)-1,@offy+@bs*(y+1)-1,color1,color2)        
      end
    end
    #puts "--dr2:#{ESP32::System::tick_ms}"
    draw_tetrimino(@current_tet) if @current_tet
    #puts "--dr3:#{ESP32::System::tick_ms}"
  end

  def run_game(key)
    current_time = ESP32::System::tick_ms
    update = false
    if key>0
      move_tetrimino(key)
      update = true
    end
    if current_time > @update_speed+@last_time
      #update tetrimino position
      @last_time = current_time
      update_tetrimino
      check_field
      update = true
    end
    if update
      draw_field
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
    return if @current_tet.y < 3
    case key
    when Key::K_LEFT
      @current_tet.x -= 1
      @current_tet.x += 1 if check_collision(@current_tet)
    when Key::K_RIGHT
      @current_tet.x += 1
      @current_tet.x -= 1 if check_collision(@current_tet)
    when Key::K_DOWN
      loop do
        @current_tet.y += 1
        if check_collision(@current_tet)
          @current_tet.y -= 1
          break
        end
      end
    when Key::K_SPACE
      @current_tet.rotate(1)
      @current_tet.rotate(-1) if check_collision(@current_tet)
    end
  end

  def check_collision(t)
    t.field.each_with_index do |line,y|
      line.each_with_index do |v,x|
        if v==1
          return true if @field[t.y+y][t.x+x] != :E
        end
      end
    end
    false
  end

  def fix_tetrimino(t)
    t.field.each_with_index do |line,y|
      line.each_with_index do |v,x|
        if v==1
          @field[t.y+y][t.x+x] = t.type
        end
      end
    end
  end

  def update_tetrimino
    @current_tet.y += 1
    if check_collision(@current_tet)
      @current_tet.y -= 1
      if @current_tet.y <= 1
        @state = :close_game
      else
        fix_tetrimino(@current_tet)
        set_new_tetrimino
        GC.start
      end
    end
  end

  def shift_lines(i)
    loop do
      break if i < 4
      (0...10).each do |x|
        @field[3+i][1+x] = @field[3+i-1][1+x]
      end
      i -= 1
    end
  end

  def check_field
    (0...20).each do |y|
      all_set = true
      (0...10).each do |x|
        if @field[3+y][1+x] == :E
          all_set = false
          break
        end
      end
      if all_set
        shift_lines(y)
      end
    end
  end

end

tetris = Tetris.new

double_buffer = true
#BG
Display.clear
Display.draw_picture(0,0,"/sample/moscow_320_200.img")
if double_buffer
 Display::swap
 Display.draw_picture(0,0,"/sample/moscow_320_200.img")
 Display::swap
end


def get_pad_key
  k = Key::K_NONE
  k = Key::K_DOWN  if Input::paddown?(Pad::DOWN)
  k = Key::K_LEFT  if Input::paddown?(Pad::LEFT)
  k = Key::K_RIGHT  if Input::paddown?(Pad::RIGHT)
  k = Key::K_SPACE  if Input::paddown?(Pad::A)
  k = Key::K_ESCAPE if Input::paddown?(Pad::SELECT)
  return k
end

loop do
  #k = Input::get_key
  k = get_pad_key
  break if k == Key::K_ESCAPE
  #puts "A--#{ESP32::System::tick_ms}--"
  if double_buffer
    if tetris.update(k)
      #puts "swap:#{ESP32::System::tick_ms}"
      Display::swap
    end
  else
    tetris.update(k)
  end
end
puts "End of Script"

)"
