
#include <stdio.h>
#include "fabgl.h"
#include "fmruby.h"
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

FmrbEditor::FmrbEditor():m_buff_head(NULL),m_disp_head_line(0){

}

static void wait_key(char target){
  while(true)
  {
    if (Terminal.available())
    {
      char c = Terminal.read();
      if(c == target){
        return;
      }
    }
  }
}

int FmrbEditor::run(void){
  m_height = Terminal.getRows();
  m_width  = Terminal.getColumns();
  printf("Editor begin\n");
  wait_key(0x0D);
  printf("Editor got key\n");
  Terminal.clear();
  move(0,0);
  load(sample_script);
  update();

  while(true)
  {
    if (Terminal.available())
    {
      char c = Terminal.read();
      printf("> %02x\n",c);
      switch (c) {
        //---FUNCTION KEY---
        //UPDATE
        //SAVE
        //LOAD
        //RUN
        //---Other KEY---
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

void FmrbEditor::finalize(void){

}

#define LINE_MAX (16)
struct EditLine* FmrbEditor::load_line(const char* in)
{
  struct EditLine* line_p = (struct EditLine*)fmrb_spi_malloc(sizeof(struct EditLine));
  if(NULL==line_p){
    m_error = EDIT_MEM_ERROR;
    return NULL;
  }
  line_p->text = NULL;
  line_p->flag = 0;
  int csr=0;
  bool end_flag=false;
  while(!end_flag)
  {
    if(csr % LINE_MAX == 0){
      if(line_p->text==NULL)
      {
        line_p->text = (char*)fmrb_spi_malloc(LINE_MAX);
        if(NULL==line_p->text)
        {
          free(line_p);
          return NULL;
        }
      }else{
        if (NULL==fmrb_spi_realloc(line_p->text,csr+LINE_MAX))
        {
          free(line_p->text);
          free(line_p);
          return NULL;
        }
      }
    }
    line_p->text[csr] = in[csr];
    //if(in[csr]==0x0A)
    if(in[csr]=='\r' || in[csr]=='\n')
    {
      line_p->text[csr] = '\0';
      line_p->length = csr;
      end_flag = true;
    }
    csr++;
  }
  return line_p;
}


void FmrbEditor::load(const char* buf)
{
  m_error = EDIT_NO_ERROR;
  int csr=0;
  struct EditLine* fist_line = (struct EditLine*)malloc(sizeof(struct EditLine));
  if(NULL==fist_line){
    m_error = EDIT_MEM_ERROR;
    return;
  }
  fist_line->prev=NULL;
  struct EditLine* last_line = fist_line;
  while(buf[csr]!='\0')
  {
    struct EditLine* line = load_line(&buf[csr]);
    if(NULL==line){
      m_error = EDIT_MEM_ERROR;
      return;
    }
    printf("load size=%04d : %s\n",line->length,line->text);
    csr += line->length;
    line->prev = last_line;
    last_line->next = line;

    csr++;
    last_line = line;
  }
  last_line->next=NULL;
  m_buff_head = fist_line;
}

void FmrbEditor::move(int x,int y)
{
  if(x>80)x=80-1;
  if(x<0)x=0;
  if(y>25)y=25-1;
  if(y<0)y=0;
  m_x = x;
  m_y = y;
  char buf[10];
  sprintf(buf,"\e[%d;%dH",m_y,m_x);
  Terminal.write(buf);
}

EditLine* FmrbEditor::seek_line(int n)
{
  if(NULL==m_buff_head)
  {
    printf("m_buff_head is NULL\n");
    return NULL;
  }
  if(NULL == m_buff_head->next){
    return NULL; 
  } 
  EditLine* line = m_buff_head->next;
  for(int i=0;i<n;i++){
    line = line->next;
    if(NULL==line){
      printf("line %d doesn't exist\n",n);
      return NULL;
    }
  }
  return line;
}

void FmrbEditor::update()
{
  EditLine* line = seek_line(m_disp_head_line);
  int cnt=0;
  printf("m_height=%d\n",m_height);
  while(NULL != line){
    Terminal.write(line->text);
    Terminal.write("\r\n");
    line = line->next;
    cnt++;
    if(cnt >= m_height-1)
    {
      break;
    }
  }
  int bottom = m_height;
  move(0,bottom);
  Terminal.write("\e[7mLOAD\e[0m");
  move(8,bottom);
  Terminal.write("\e[7mSAVE\e[0m");
  move(16,bottom);
  Terminal.write("\e[7mRUN \e[0m");

  move(0,0);
}

