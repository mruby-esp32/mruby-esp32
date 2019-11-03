
#include <stdio.h>
#include "fabgl.h"
#include "fmruby.h"
#include "fmruby_app.h"
#include "fmruby_editor.h"

const char* sample_script = 
R"(puts "*** Family mruby v0.1 ***"

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

FmrbEditor::FmrbEditor():
  m_buff_head(NULL),
  m_lineno_shift(6),
  m_disp_head_line(1)
{

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
  m_disp_height = m_height - 1;
  m_disp_width = m_width;

  printf("Editor begin\n");
  wait_key(0x0D);
  Terminal.clear();
  move_cursor(m_lineno_shift+1,1);
  load(sample_script);
  update();

  int escape = 0;
  char escape_c[4] = {0};
  while(true)
  {
    if (Terminal.available())
    {
      char c = Terminal.read();
      //printf("> %02x\n",c);

      if(!escape)
      {
        if(c>=0x20 && c<=0x7E){
          //Visible character
          insert_ch(c);
        }else{
          switch(c){
            case 0x7F: // BS
              printf("BS\n");
              delete_ch();
              break;
            case 0x0D: // CR
              printf("RETURN\n");
              break;
            case 0x1A: // Ctrl-z
              printf("Ctrl-z\n");
              break;
            case 0x18: // Ctrl-x
              printf("Ctrl-x\n");
              break;
            case 0x16: // Ctrl-v
              printf("Ctrl-v\n");
              break;
            case 0x03: // Ctrl-c
              printf("Ctrl-c\n");
              break;
            case 0x04: // Ctrl-d
              printf("Ctrl-d\n");
              break;
            case 0x1B: // ESC
              escape = 1;
              break;
          }
        }

      }else{ // Escape

        if(escape==1){
          switch(c){
            case 0x5B: // '[' : Cursor/ 
            case 0x4F: // 'O' : Function key
              escape_c[0] = c;
              escape=2;
              break;
            default:
              escape=0;
              break;
          }
        }else if(escape==2){
          if(escape_c[0]==0x5B){
            switch(c){
              case 0x41:  // ESC[A : UP
              case 0x42:  // ESC[B : DOWN
              case 0x43:  // ESC[C : RIGHT
              case 0x44:  // ESC[D : LEFT
                move_edit_cursor(c);
                print_csr_info();
                escape = 0;
                break;
              case 0x31:  // ESC[1 : ...
              case 0x32:  // ESC[2 : ...
              case 0x33:  // ESC[3 : ...
                escape_c[1] = c;
                escape = 3;
                break;
              default:
                escape = 0;
              break;
            }
          }else if(escape_c[0]==0x4F){
            switch(c){
              case 0x50: // ESC OP : F1
                printf("F1\n");
                break;
              case 0x51: // ESC OP : F2
                printf("F2\n");
                break;
              case 0x52: // ESC OP : F3
                printf("F3\n");
                return 0;
                break;
              case 0x53: // ESC OP : F4
                printf("F4\n");
                break;
            }
            escape = 0;
          }else{
            escape = 0;
          }
        }else if(escape==3){
          if(escape_c[1]==0x31){
            switch(c){
              case 0x35: // ESC[15 : ..  F5
              case 0x37: // ESC[17 : ..  F6
              case 0x38: // ESC[18 : ..  F7
              case 0x39: // ESC[19 : ..  F8
                escape_c[2] = c;
                escape = 4;
                break;
              default:
                escape = 0;
                break;
            }
          }else if(escape_c[1]==0x32){
            switch(c){
              case 0x30: // ESC[20 : ..  F9
              case 0x31: // ESC[21 : ..  F10
              case 0x33: // ESC[23 : ..  F11
              case 0x34: // ESC[24 : ..  F12
                escape_c[2] = c;
                escape = 4;
                break;
              default:
                escape = 0;
                break;
            }
          }else if(escape_c[1]==0x33){
            if(c==0x7E){ // ESC[3~ : DEL
              printf("DEL\n");
              delete_ch();
            }
            escape=0;
          }else{
            escape=0;
          }
        }else if(escape==4){
          switch(c){
            case 0x7E: //  ESC[1*~ : FN
              printf("FN\n");
              break;
            default:
              break;
          }
          escape=0;
        }else{ //escape > 4
          escape=0;
        }

        if(escape==0){
          escape_c[0] = 0;
          escape_c[1] = 0;
          escape_c[2] = 0;
          escape_c[3] = 0;
        }
      }
    }
  }
}

void FmrbEditor::finalize(void){

}

static EditLine* alloc_new_line()
{
  struct EditLine* line = (EditLine*)fmrb_spi_malloc(sizeof(struct EditLine));
  if(line){
    memset(line,0,sizeof(EditLine));
  }
  return line;
}

#define LINE_MAX (16)
struct EditLine* FmrbEditor::load_line(const char* in)
{
  struct EditLine* line_p = alloc_new_line();
  if(NULL==line_p){
    m_error = EDIT_MEM_ERROR;
    return NULL;
  }
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
  m_total_line = 0;
  struct EditLine* fist_line = alloc_new_line();
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
    m_total_line += 1;
    line->lineno = m_total_line;
    csr += line->length;
    line->prev = last_line;
    last_line->next = line;

    csr++;
    last_line = line;
  }
  last_line->next=NULL;
  m_buff_head = fist_line;
}

void FmrbEditor::print_csr_info(void)
{
  printf("(%02d,%02d) head=%d total=%d\n",m_x,m_y,m_disp_head_line,m_total_line);
}

void FmrbEditor::move_edit_cursor(int dir)
{
  int current_line_n = m_disp_head_line-1 + m_y;
  //printf("current_line_n=%d\n",current_line_n);
  EditLine* line = seek_line(current_line_n);
  if(NULL==line){
    return;
  }
  switch(dir){
    case 0x41:  // A : UP
      if(1==m_y){
        //scroll
        if(m_disp_head_line > 1){
          m_disp_head_line -= 1;
          update();
        }
      }else if(m_y > 1){
        m_y = m_y - 1;
        if( m_x > m_lineno_shift + line->prev->length + 1 ){
          m_x = m_lineno_shift + line->prev->length + 1;
        }
        move_cursor(m_x, m_y);
      }
      break;
    case 0x42:  // B : DOWN
      if(m_disp_height == m_y){
        //scroll
        if(m_disp_head_line-1+m_disp_height < m_total_line){
          m_disp_head_line += 1;
          update();
        }
      }else if(m_y < m_disp_height ){
        m_y = m_y + 1;
        if( m_x > m_lineno_shift + line->next->length + 1 ){
          m_x = m_lineno_shift + line->next->length + 1;
        }
        move_cursor(m_x, m_y);
      }
      break;
    case 0x43:  // C : RIGHT
      if(m_x <= m_lineno_shift + line->length){
        move_cursor(m_x + 1, m_y);
      }
      break;
    case 0x44:  // D : LEFT
      if(m_x > m_lineno_shift + 1){
        move_cursor(m_x - 1, m_y);
      }
      break;
    default:
      break;
  }

}

void FmrbEditor::move_cursor(int x,int y)
{
  if(x>m_width)x=m_width;
  if(x<1)x=1;
  if(y>m_height)y=m_height;
  if(y<1)y=1;
  m_x = x;
  m_y = y;
  move(m_x,m_y);
}

void FmrbEditor::move(int x,int y)
{
  if(x>m_width)x=m_width;
  if(x<1)x=1;
  if(y>m_height)y=m_height;
  if(y<1)y=1;
  char buf[10];
  sprintf(buf,"\e[%d;%dH",y,x);
  Terminal.write(buf);
}


EditLine* FmrbEditor::seek_line(int n)
{
  if(n<1 || n>m_total_line)
  {
    printf("seek line > total line\n");
    return NULL;
  }
  if(NULL==m_buff_head)
  {
    printf("m_buff_head is NULL\n");
    return NULL;
  }
  if(NULL == m_buff_head->next){
    return NULL; 
  } 
  EditLine* line = m_buff_head;
  for(int i=1;i<=n;i++){
    line = line->next;
    if(NULL==line){
      printf("line %d doesn't exist\n",n);
      return NULL;
    }
  }
  return line;
}

void FmrbEditor::draw_line(int disp_y,EditLine* line)
{
  move(1,disp_y);
  Terminal.write("\e[2K"); // delete line

  Terminal.printf("\e[34m%04d: \e[0m",line->lineno);
  Terminal.write(line->text);
  //Terminal.write("\r\n");
}


void FmrbEditor::update()
{
  EditLine* line = seek_line(m_disp_head_line);
  int cnt=1;
  while(NULL != line){
    draw_line(cnt,line);
    if(m_y == cnt){
      if( m_x >= line->length ){
        move(line->length, m_y);
      }
    }
    line = line->next;
    cnt++;
    if(cnt > m_disp_height)
    {
      break;
    }
  }

  //Draw Functions
  int bottom = m_height;
  move(1,bottom);
  Terminal.write("\e[30m\e[46m UPDATE \e[0m");
  move(11,bottom);
  Terminal.write("\e[30m\e[46m  MENU  \e[0m");
  move(21,bottom);
  Terminal.write("\e[30m\e[46m  RUN   \e[0m");

  move(m_x,m_y);
}

void FmrbEditor::insert_ch(char c)
{

}

void FmrbEditor::insert_ret()
{

}

void FmrbEditor::delete_ch()
{

}
