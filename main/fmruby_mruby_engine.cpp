
#include "mruby.h"
#include "mruby/irep.h"
#include "mruby/compile.h"
#include "mruby/error.h"
#include "mruby/string.h"

#include "fmruby_fabgl.h"
#include "fmruby_app.h"

#include "entry_mrb.h"

const char* sample_script2 = 
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

void* mrb_esp32_psram_allocf(mrb_state *mrb, void *p, size_t size, void *ud)
{
  if (size == 0) {
    free(p);
    return NULL;
  }
  else {
    return heap_caps_realloc(p, size, MALLOC_CAP_SPIRAM);
  }
}


void mruby_task(void *pvParameter)
{
  mrb_state *mrb = mrb_open_allocf(mrb_esp32_psram_allocf,NULL);
  int ai = mrb_gc_arena_save(mrb);

  mrb_value val = mrb_load_string(mrb,sample_script2);
  if (mrb->exc) {
    printf("Exception occurred: %s\n", mrb_str_to_cstr(mrb, mrb_inspect(mrb, mrb_obj_value(mrb->exc))));
    mrb->exc = 0;
  } else {
    printf("Success\n");
  }

  mrb_gc_arena_restore(mrb, ai);
  mrb_close(mrb);

/*
  mrb_state *mrb = mrb_open_allocf(mrb_esp32_psram_allocf,NULL);
  int ai = mrb_gc_arena_save(mrb);
  printf("Loading binary...!\n");
  mrb_load_irep(mrb, entry_mrb);
  if (mrb->exc) {
    printf("Exception occurred: %s\n", mrb_str_to_cstr(mrb, mrb_inspect(mrb, mrb_obj_value(mrb->exc))));
    mrb->exc = 0;
  } else {
    printf("Success\n");
  }
  mrb_gc_arena_restore(mrb, ai);
  mrb_close(mrb);
*/
  printf("End of mruby task\n");
  while (1) {
	  vTaskDelay(1);
  }
}
