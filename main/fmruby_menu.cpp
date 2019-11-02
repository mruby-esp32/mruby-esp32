#include "fmruby.h"
#include "fmruby_app.h"
#include "fmruby_fabgl.h"
#include "fmruby_editor.h"

char* sample_script2 = 
R"(
puts "*** Family mruby v0.1 ***"

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

void print_info()
{
  Terminal.write("\e[37m* * FabGL - Loopback VT/ANSI Terminal\r\n");
  Terminal.write("\e[34m* * 2019 by Fabrizio Di Vittorio - www.fabgl.com\e[32m\r\n\n");
  Terminal.printf("\e[32mScreen Size        :\e[33m %d x %d\r\n", VGAController.getScreenWidth(), VGAController.getScreenHeight());
  Terminal.printf("\e[32mTerminal Size      :\e[33m %d x %d\r\n", Terminal.getColumns(), Terminal.getRows());
  Terminal.printf("\e[32mKeyboard           :\e[33m %s\r\n", Keyboard.isKeyboardAvailable() ? "OK" : "Error");
  Terminal.printf("\e[32mFree DMA Memory    :\e[33m %d\r\n", heap_caps_get_free_size(MALLOC_CAP_DMA));
  Terminal.printf("\e[32mFree 32 bit Memory :\e[33m %d\r\n\n", heap_caps_get_free_size(MALLOC_CAP_32BIT));
  Terminal.write("\e[32mFree typing test - press ESC to introduce escape VT/ANSI codes\r\n\n");
  Terminal.write("\e[37m");
}

FmrbEditor Editor;

void terminal_init(void)
{
  printf("start terminal_init\n");

  Terminal.begin();
  Terminal.connectLocally();      // to use Terminal.read(), available(), etc..

  Terminal.setBackgroundColor(Color::Black);
  Terminal.setForegroundColor(Color::White);
  Terminal.clear();

  print_info();

  Terminal.enableCursor(true);

}

void menu_app()
{
  printf("terminal_task\n");

  while(true){
    fabgl_terminal_mode_init();
    terminal_init();
    printf("terminal_init() done\n");

    //select app

    //1.editor, 2.run script

    int err_code = Editor.run();
    //vTaskDelay(10000 / portTICK_RATE_MS);

    Terminal.end();
    printf("Terminal.end()\n");

    fabgl_mruby_mode_init();
    mruby_engine(sample_script2);
  }
}
