R"( #"
include Narya

class Ball
  attr_accessor :speed
  def initialize(x,y,r,col,speed)
    @x = x
    @y = y
    @r = r
    @color = col
    @speed = speed
  end

  def move(x,y)
    @x += x
    @x = 0 if @x > 320
    @x = 320 if @x < 0
    @y += y
    @y = 0 if @y > 200
    @y = 200 if @y < 0
  end

  def draw
    Display::draw_circle(@x,@y,@r,@color,@color)
  end
end

rand(200)
def load_balls
  balls = []
  10.times do 
    balls << Ball.new(rand(320), rand(200)+2, 2, "CYAN", 1 )
  end
  5.times do 
    balls << Ball.new(rand(320), rand(200)+2, 7, "GREEN", 3 )
  end
  3.times do 
    balls << Ball.new(rand(320), rand(200)+2, 12, "MAGENTA", 4 )
  end
  balls
end

bitmap = Bitmap.new.load("/sample/man.img")
sp = Sprite.new(bitmap)
sp.move_to(100,100)

balls = load_balls
count = 0
loop do
  Display::clear
  Display::draw_text(20,5,"Family mruby DEMO!")
  Display::draw_line(0,15,Display::width-1,15,"311")
  Display::draw_pixel(10,20,"003")
  Display::draw_rect(40,40,50,50,"030")
  Display::draw_rect(70,40,80,50,"030","333")
  balls.each do |ball|
    ball.move(-ball.speed,0)
    ball.draw
  end
  mx=0
  my=0
  if Input::available
    if Input::keydown?(Key::K_UP)
      my=-2
    end
    if Input::keydown?(Key::K_DOWN)
      my=2
    end
    if Input::keydown?(Key::K_LEFT)
      mx=-2
    end
    if Input::keydown?(Key::K_RIGHT)
      mx=2
    end
    if Input::keydown?(Key::K_ESCAPE)
      break
    end
  end
  sp.move(mx*1,my*1)
  
  Display::swap
end
)";
