puts "*** Family mruby v0.0 ***"

class Ball
  def initialize
    @x = 0
    @y = 0
  end

  def update
    @x += 1
    @y += 1
  end
end

def draw(ball)
  p ball
end

balls = []
10.times do 
  balls << Ball.new
end

loop do
  balls.each do |ball|
    ball.update
    draw ball
  end
  #Narya::update
end
