
#include <stdio.h>
#include "fabgl.h"
#include "fmruby.h"
#include "fmruby_app.h"
#include "fmruby_fabgl.h"
#include "fmruby_editor.h"


const char* null_script = "\n";

const char* sample_script = 
#include "./mrb/entry_mrb.rb"
//#include "./mrb/bitmap_test.rb"
;

//#define EDT_DEBUG(...)  printf(__VA_ARGS__)
#define EDT_DEBUG(...)


EditLine* EditLine::create_line(void)
{
  EditLine* line = (EditLine*)fmrb_spi_malloc(sizeof(EditLine));
  if(line){
    if(line->init(NULL) < 0){
      fmrb_free(line);
      return NULL;
    }    
  }
  return line;
}

EditLine* EditLine::create_line(char* input)
{
  EditLine* line = (EditLine*)fmrb_spi_malloc(sizeof(EditLine));
  if(NULL==line)return NULL;
  
  if(line->init(input) < 0){
    fmrb_free(line);
    return NULL;
  }
  return line;
}


int EditLine::init(char* input)
{
  if(NULL==input){
    text = (char*)fmrb_spi_malloc(EDITLINE_BLOCK_SIZE);
    if(text == NULL) return -1;
    memset(text,0,EDITLINE_BLOCK_SIZE);
    text[0] = '\0';
    length = 0;
    buff_size = EDITLINE_BLOCK_SIZE;
  }else{
    int input_len = strlen(input);
    int block_size = (input_len+1)/EDITLINE_BLOCK_SIZE + 1;
    buff_size = EDITLINE_BLOCK_SIZE * block_size;
    text = (char*)fmrb_spi_malloc(buff_size);
    if(text == NULL) return -1;
    memset(text,0,buff_size);
    strcpy(text,input);
    length = input_len;
  }
  flag = 0;
  lineno = 0;
  prev = NULL;
  next = NULL;
  return 0;
}


int EditLine::insert(uint16_t pos,char c)
{
  if(text==NULL) return -1;

  EDT_DEBUG("p:%d length=%d buffsize=%d\n",pos,length,buff_size);
  if( length+1+1 > buff_size) // Text lenght + null char + new char >= cuurent buff size
  {
    EDT_DEBUG("realloc block(text_p:%p new buff size:%d)\n",text,buff_size+EDITLINE_BLOCK_SIZE);
    text = (char*)fmrb_spi_realloc(text,buff_size+EDITLINE_BLOCK_SIZE);
    if (NULL==text)
    {
      return -1;
    }
    memset(text+buff_size,0,EDITLINE_BLOCK_SIZE);
    buff_size += EDITLINE_BLOCK_SIZE;
  }
  // 012345
  // ABC@      pos=2, length=3, buffsize=6
  // ABXC@            length=4, buffsize=6

  EDT_DEBUG("%p, %p, %d\n",text+pos+1,text+pos,length-pos+1);
  memmove(text+pos+1,text+pos,length-pos+1);
  text[pos]=c;
  length+=1;
  return 0;
}

int EditLine::backdelete(uint16_t pos)
{
  if(text==NULL) return -1;
  if(pos==0) return -1;
  // 012345
  // ABC@      pos=1, length=3, buffsize=6
  // BC@              length=2, buffsize=6

  EDT_DEBUG("%p, %p, %d\n",text+pos+1,text+pos,length-pos+1);
  memmove(text+pos-1,text+pos,length-pos+1);
  length-=1;

  EDT_DEBUG("p:%d length=%d buffsize=%d\n",pos,length,buff_size);
  if( length+1 <= buff_size - EDITLINE_BLOCK_SIZE) // Text lenght + null char < cuurent buff size - BLOCK
  {
    EDT_DEBUG("realloc block(text_p:%p new buff size:%d)\n",text,buff_size-EDITLINE_BLOCK_SIZE);
    text = (char*)fmrb_spi_realloc(text,buff_size-EDITLINE_BLOCK_SIZE);
    if (NULL==text)
    {
      return -1;
    }
    buff_size -= EDITLINE_BLOCK_SIZE;
  }

  return 0;
}


int EditLine::insert(char c)
{
  return 1;
}

char* EditLine::cut(uint16_t start_pos, uint16_t end_pos)
{
  // 0123456789
  // abcde@     // start:2 end:5
  // cde@       // buff
  // ab@        // text

  if(NULL==text)return NULL;
  if(end_pos < start_pos)return NULL;
  int copy_size = end_pos - start_pos + 1;
  char* buff = (char*)fmrb_spi_malloc(copy_size);
  memcpy(buff,&text[start_pos],copy_size);
  memmove(&text[start_pos],&text[end_pos],length-end_pos+1);
  length -= copy_size-1;
  
  EDT_DEBUG("start:%d end:%d length=%d buffsize=%d\n",start_pos,end_pos,length,buff_size);
  if( length+1 <= buff_size - EDITLINE_BLOCK_SIZE) // Text lenght + null char < cuurent buff size - BLOCK
  {
    EDT_DEBUG("realloc block(text_p:%p new buff size:%d)\n",text,buff_size-EDITLINE_BLOCK_SIZE);
    text = (char*)fmrb_spi_realloc(text,buff_size-EDITLINE_BLOCK_SIZE);
    if (NULL==text)
    {
      fmrb_free(buff);
      return NULL;
    }
    buff_size -= EDITLINE_BLOCK_SIZE;
  }
  return buff;
}


/***********************************
 * FmrbEditor
 *   An editor for mruby code
 ***********************************/
FmrbEditor::FmrbEditor():
  m_buff_head(NULL),
  m_lineno_shift(6),
  m_disp_head_line(1),
  m_term(NULL) 
{

}
int FmrbEditor::begin(fabgl::Terminal* terminal)
{
  m_term = terminal;
  return 0;
}

int FmrbEditor::release()
{
  return 0;
}

void FmrbEditor::wait_key(char target){
  while(true)
  {
    if (m_term->available())
    {
      char c = m_term->read();
      if(c == target){
        return;
      }
    }
  }
}

int FmrbEditor::run(char* input_script){
  if(!m_term) return -1;
  m_height = m_term->getRows();
  m_width  = m_term->getColumns();
  m_disp_height = m_height - 1;
  m_disp_width = m_width;

  FMRB_DEBUG(FMRB_LOG::INFO,"Editor begin\n");

  m_term->clear();
  move_cursor(m_lineno_shift+1,1);

  if(input_script)
  {
    load(input_script);
    fmrb_free(input_script);
  }else{
    load(null_script);
    //load_demo_file();
  }

  update();

  int escape = 0;
  char escape_c[4] = {0};
  while(true)
  {
    if (m_term->available())
    {
      char c = m_term->read();
      //printf("> %02x\n",c);

      if(!escape)
      {
        if(c>=0x20 && c<=0x7E){
          //Visible character
          insert_ch(c);
        }else{
          switch(c){
            case 0x7F: // BS
              EDT_DEBUG("BS\n");
              delete_ch();
              break;
            case 0x0D: // CR
              EDT_DEBUG("RETURN\n");
              insert_ret();
              break;
            case 0x1A: // Ctrl-z
              EDT_DEBUG("Ctrl-z\n");
              break;
            case 0x18: // Ctrl-x
              EDT_DEBUG("Ctrl-x\n");
              break;
            case 0x16: // Ctrl-v
              EDT_DEBUG("Ctrl-v\n");
              break;
            case 0x03: // Ctrl-c
              EDT_DEBUG("Ctrl-c\n");
              break;
            case 0x04: // Ctrl-d
              EDT_DEBUG("Ctrl-d\n");
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
                FMRB_DEBUG(FMRB_LOG::DEBUG,"F1\n");
                break;
              case 0x51: // ESC OP : F2
                FMRB_DEBUG(FMRB_LOG::DEBUG,"F2\n");
                save_file();
                break;
              case 0x52: // ESC OP : F3
                FMRB_DEBUG(FMRB_LOG::DEBUG,"F3\n");
                load_file();
                break;
              case 0x53: // ESC OP : F4
                EDT_DEBUG("F4\n");
                return 0;
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
                m_term->read();
                EDT_DEBUG("F5\n");
                load_demo_file();
                escape = 0;
                break;
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
              EDT_DEBUG("DEL\n");
              delete_ch();
            }
            escape=0;
          }else{
            escape=0;
          }
        }else if(escape==4){
          switch(c){
            case 0x7E: //  ESC[1*~ : FN
              EDT_DEBUG("FN\n");
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

EditLine* FmrbEditor::load_line(const char* in)
{
  EditLine* line_p = EditLine::create_line();
  if(NULL==line_p){
    m_error = EDIT_MEM_ERROR;
    return NULL;
  }
  int csr=0;
  bool end_flag=false;
  while(!end_flag)
  {
#if 1
    if( (csr+1) % EDITLINE_BLOCK_SIZE == 0){
      if (NULL==fmrb_spi_realloc(line_p->text,csr+EDITLINE_BLOCK_SIZE))
      {
        fmrb_free(line_p->text);
        fmrb_free(line_p);
        return NULL;
      }
      line_p->buff_size = line_p->buff_size+EDITLINE_BLOCK_SIZE;
    }
    line_p->text[csr] = in[csr];
#else
    line_p->insert(in[csr]);
#endif
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
  EditLine* fist_line = EditLine::create_line();
  if(NULL==fist_line){
    m_error = EDIT_MEM_ERROR;
    return;
  }
  fist_line->prev=NULL;
  EditLine* last_line = fist_line;
  while(buf[csr]!='\0')
  {
    EditLine* line = load_line(&buf[csr]);
    if(NULL==line){
      m_error = EDIT_MEM_ERROR;
      return;
    }
    //FMRB_DEBUG(FMRB_LOG::DEBUG,"load size=%04d : %s\n",line->length,line->text);
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
  EDT_DEBUG("(%02d,%02d) head=%d total=%d\n",m_x,m_y,m_disp_head_line,m_total_line);
}

void FmrbEditor::move_edit_cursor(int dir)
{
  int current_line_n = m_disp_head_line-1 + m_y;
  //FMRB_DEBUG(FMRB_LOG::DEBUG,"current_line_n=%d\n",current_line_n);
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
  m_term->write(buf);
}


EditLine* FmrbEditor::seek_line(int n)
{
  if(n<1 || n>m_total_line)
  {
    FMRB_DEBUG(FMRB_LOG::ERR,"seek line > total line\n");
    return NULL;
  }
  if(NULL==m_buff_head)
  {
    FMRB_DEBUG(FMRB_LOG::ERR,"m_buff_head is NULL\n");
    return NULL;
  }
  if(NULL == m_buff_head->next){
    return NULL; 
  } 
  EditLine* line = m_buff_head;
  for(int i=1;i<=n;i++){
    line = line->next;
    if(NULL==line){
      FMRB_DEBUG(FMRB_LOG::ERR,"line %d doesn't exist\n",n);
      return NULL;
    }
  }
  return line;
}

void FmrbEditor::draw_line(int disp_y,EditLine* line)
{
  move(1,disp_y);
  m_term->write("\e[2K"); // delete line

  m_term->printf("\e[34m%04d: \e[0m",line->lineno);
  m_term->write(line->text);
  //m_term->write("\r\n");
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
  m_term->write("\e[30m\e[46m UPDATE \e[0m");
  move(11,bottom);
  m_term->write("\e[30m\e[46m  SAVE  \e[0m");
  move(21,bottom);
  m_term->write("\e[30m\e[46m  LOAD  \e[0m");
  move(31,bottom);
  m_term->write("\e[30m\e[46m  RUN   \e[0m");
  move(41,bottom);
  m_term->write("\e[30m\e[46m  DEMO  \e[0m");

  move(m_x,m_y);
}

void FmrbEditor::update_lineno(void)
{
  if(NULL==m_buff_head) return;
  if(NULL==m_buff_head->next) return;
  EditLine* line = m_buff_head->next;
  int lineno = 0;
  while(line)
  {
    lineno += 1;
    line->lineno = lineno;
    line = line->next;
  }

}

void FmrbEditor::insert_ch(char c)
{
  EditLine* line = seek_line(m_disp_head_line+m_y-1);
  //FMRB_DEBUG(FMRB_LOG::DEBUG,"INSERT(%c):text:%s\n",c,line->text);
  line->insert(m_x-m_lineno_shift-1,c);
  draw_line(m_y,line);
  m_x += 1;
  move(m_x,m_y);
}

void FmrbEditor::insert_ret()
{
  EditLine* line = seek_line(m_disp_head_line+m_y-1);
  if(NULL==line)return;
  char* cut_test = line->cut(m_x-m_lineno_shift-1,line->length);
  if(NULL==cut_test)return;
  EditLine* new_line = EditLine::create_line(cut_test);
  if(NULL==new_line){
    fmrb_free(cut_test);
    return;
  }
  new_line->prev = line;
  new_line->next = line->next;
  line->next = new_line;
  m_total_line++;
  fmrb_free(cut_test);
  update_lineno();

  m_x = m_lineno_shift + 1;

  if(m_disp_height == m_y){
    //scroll
    if(m_disp_head_line-1+m_disp_height < m_total_line){
      m_disp_head_line += 1;
    }
  }else{
    m_y = m_y + 1;
  }

  update();
}

void FmrbEditor::delete_ch()
{
  EditLine* line = seek_line(m_disp_head_line+m_y-1);
  //FMRB_DEBUG(FMRB_LOG::DEBUG,"BS:text:%s\n",line->text);
  if(m_x-m_lineno_shift-1>0){
    line->backdelete(m_x-m_lineno_shift-1);
    draw_line(m_y,line);
    m_x -= 1;
    move(m_x,m_y);
  }
}



char* FmrbEditor::dump_script(void)
{
  if(NULL==m_buff_head) return NULL;
  if(NULL==m_buff_head->next) return NULL;
  EditLine* line = m_buff_head->next;
  int total_length = 0;
  while(line)
  {
    total_length += line->length + 1;
    line = line->next;
  }
  FMRB_DEBUG(FMRB_LOG::DEBUG,"total_length=%d\n",total_length);
  char* buff = (char*)fmrb_spi_malloc(total_length+1);
  if(NULL==buff){
    FMRB_DEBUG(FMRB_LOG::ERR,"cannot allocate memory!\n");
    return NULL;
  }
  int csr = 0;
  line = m_buff_head->next;
  while(line)
  {
    memcpy(&buff[csr],line->text,line->length);
    buff[csr+line->length]='\n';
    csr += line->length+1;
    line = line->next;
  }
  buff[csr]='\0';
  FMRB_DEBUG(FMRB_LOG::DEBUG,"csr=%d total_length=%d\n",csr,total_length);
  //FMRB_DEBUG(FMRB_LOG::DEBUG,"%s\n",buff);
  return buff;
}

void FmrbEditor::clear_buffer(){
  FMRB_DEBUG(FMRB_LOG::DEBUG,"clear_buffer\n");
  EditLine* line = m_buff_head;
  while(line)
  {
    if(line->text){
      fmrb_free(line->text);
    }
    EditLine* old = line;
    line = line->next;
    fmrb_free(old);
  }
  m_buff_head = NULL;
}

void FmrbEditor::load_file(){
  FMRB_DEBUG(FMRB_LOG::DEBUG,"load_file\n");
  //clear current buffer
  clear_buffer();
  uint32_t fsize;
  char* buff = FMRB_storage.load("/default.rb",fsize,true,false);
  move_cursor(m_lineno_shift+1,1);
  if(buff){
    load(buff);
  }else{
    FMRB_DEBUG(FMRB_LOG::ERR,"failed to load file\n");
    load(null_script);
  }
  update();
  FMRB_DEBUG(FMRB_LOG::DEBUG,"load_file done\n");
}

void FmrbEditor::load_demo_file(){
  //clear current buffer
  clear_buffer();
  move_cursor(m_lineno_shift+1,1);
  load(sample_script);
  update();
}

void FmrbEditor::save_file(){
  FMRB_DEBUG(FMRB_LOG::DEBUG,"save_file\n");
  char* buff = dump_script();
  if(buff){
    FMRB_storage.save(buff,"/default.rb");
    fmrb_free(buff);
  }else{
    FMRB_DEBUG(FMRB_LOG::ERR,"dump_script error\n");
  }
  FMRB_DEBUG(FMRB_LOG::DEBUG,"save_file done\n");
}