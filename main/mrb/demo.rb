R"(  #"
include Narya
Display::font_size(8,8)

class Demo
  def initialize
    @step=0
    @type=0
    @w = Display::width
    @h = Display::height
  end

  def rcol
    "#{rand(3)}#{rand(3)}#{rand(3)}"
  end
  
  def run
    case @type
    when 0 
      demo0
    when 1
      demo1
    when 2
      demo2
    when 3
      demo3
    end
    @step+=1
    if @step > 1800
      puts "type:#{@type}"
      Display::clear
      Display::swap
      Display::clear
      @type+=1 
      @step=0
    end
    @type=0 if @type > 3
  end

  def demo0
    if Display::double_buffered?
      Display::clear
      Display::draw_text(60,@step % 200,"Hello mruby!","WHITE");
      Display::draw_text(80,@step % 200 + 20,"Ver:#{Config::firmware_ver}","GREEN");
    else
      Display::draw_text(60,@step % 200-1,"Hello mruby!","BLACK");
      Display::draw_text(80,@step % 200 + 20-1,"Ver:#{Config::firmware_ver}","BLACK");
      Display::draw_text(60,@step % 200,"Hello mruby!","WHITE");
      Display::draw_text(80,@step % 200 + 20,"Ver:#{Config::firmware_ver}","GREEN");
    end
  end

  def demo1
    x = rand(@w)
    y = rand(@h)
    r = rand(50)
    c1 = rcol
    c2 = rcol
    if(rand(2)==1)
      Display::draw_circle(x,y,r,c1,c2)
      if Display::double_buffered?
        Display::swap
        Display::draw_circle(x,y,r,c1,c2)
      end
    else
      Display::draw_circle(x,y,r,c1)
      if Display::double_buffered?
        Display::swap
        Display::draw_circle(x,y,r,c1)
      end
    end
  end
  def demo2
    if(rand(2)==1)
      Display::draw_rect(rand(@w),rand(@h),rand(@w),rand(@h),rcol,rcol)
    else
      Display::draw_rect(rand(@w),rand(@h),rand(@w),rand(@h),rcol)
    end
  end
  def demo3
    if @step < 3
      Display.draw_picture(0,0,"/spiffs/sample/moscow_320_200.img")
    end
  end
end

demo = Demo.new
loop do
  demo.run
  Display::swap
  k = Input::get_key
  if k==Key::K_RETURN
    break
  end
end

)"