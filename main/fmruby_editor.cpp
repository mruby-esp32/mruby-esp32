#include "fabgl.h"
#include "fmruby_app.h"
#include "fmruby_editor.h"

const char* sample_script = 
R"(
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

puts "Sprite.new"
sp = Narya::Sprite.new
sp.move_to(100,100)

balls = load_balls
count = 0
loop do
  Narya::Display::clear

  Narya::Display::draw_text(20,5,"Family mruby DEMO!")
  balls.each do |ball|
    ball.move(-ball.speed,0)
    draw ball
  end
  sp.move(3,0)
  
  Narya::Display::swap
end
)";

FmrbEditor::FmrbEditor(){

}

static void wait_key(void){
  while(true)
  {
    if (Terminal.available())
    {
      char c = Terminal.read();
      if(c==0x0D)return;
    }
  }
}

void FmrbEditor::begin(void){
  printf("Editor begin\n");
  load(sample_script);
  printf("Editor wait_key\n");
  wait_key();
  Terminal.clear();

  while(true)
  {
    if (Terminal.available())
    {
      char c = Terminal.read();
      switch (c) {
        case 0x7F:       // DEL -> backspace + ESC[K
          Terminal.write("\b\e[K");
          break;
        case 0x0D:       // CR  -> CR + LF
          Terminal.write("\r\n");
          break;
        default:
          Terminal.write(c);
          break;
      }
    }
  }
}

void FmrbEditor::close(void){

}

void FmrbEditor::load(const char* buf)
{

}