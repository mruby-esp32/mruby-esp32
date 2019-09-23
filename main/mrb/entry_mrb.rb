puts "*** Family mruby v0.0 ***"

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
  Narya::Display::draw_circle(ball.x,ball.y,ball.r,ball.color)
end

def load_balls
  balls = []
  15.times do 
    balls << Ball.new(0, rand(230), rand(10)+10, rand(7) ,rand(15)+2)
  end
  balls
end

balls = load_balls
count = 0
loop do
  Narya::Display::clear
  balls.each do |ball|
    ball.move(ball.speed,ball.speed/2)
    draw ball
  end
  Narya::Display::swap
  ESP32::System::delay(1)
  count += 1
  if count > 500
    count = 0
    balls = load_balls
  end
end