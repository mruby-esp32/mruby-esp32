R"( #"
puts "*** Family mruby v0.6 ***"
include Narya

class Ball
  def initialize(x,y,r,col,speed)
    @x = x
    @y = y
    @r = r
    @color = col
    @speed = speed
  end
  attr_accessor :x, :y, :r, :color, :speed

  def move(x,y)
    @x += x
    @x = 0 if @x > 320
    @x = 320 if @x < 0
    @y += y
    @y = 0 if @y > 200
    @y = 200 if @y < 0
  end
end

def draw(ball)
  Display::draw_circle(ball.x,ball.y,ball.r,ball.color)
end

def load_balls
  balls = []
  10.times do 
    balls << Ball.new(rand(320), rand(200)+20, 2, 7, 1 )
  end
  5.times do 
    balls << Ball.new(rand(320), rand(200)+20, 7, 6, 3 )
  end
  2.times do 
    balls << Ball.new(rand(320), rand(200)+20, 12, 5, 4 )
  end
  balls
end

sp = Sprite.new
sp.move_to(100,100)

balls = load_balls
count = 0
loop do
  Display::clear
  Display::draw_text(20,5,"Family mruby DEMO!")
  balls.each do |ball|
    ball.move(-ball.speed,0)
    draw ball
  end
  mx=0
  my=0
  if Input::available
    #151 UP, 153 DOWN, 155 LEFT, 157 RIGHT
    if Input::keydown?(151)
      my=-2
    end
    if Input::keydown?(153)
      my=2
    end
    if Input::keydown?(155)
      mx=-2
    end
    if Input::keydown?(157)
      mx=2
    end
    if Input::keydown?(125)
      break
    end
  end
  sp.move(mx,my)
  
  Display::swap
end
)";
