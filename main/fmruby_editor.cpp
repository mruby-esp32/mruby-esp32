
#include <stdio.h>
#include "fabgl.h"
#include "fmruby.h"
#include "fmruby_app.h"
#include "fmruby_editor.h"

#include "FS.h"
#include "SPIFFS.h"

const char* null_script = "\n";

const char* sample_script = 
#include "./mrb/entry_mrb.rb"


FmrbFileService file_service;

#define DEFAULT_TEST_PATH "/default.rb"
FmrbFileService::FmrbFileService(){
  m_opened=false;
}

#define FORMAT_SPIFFS_IF_FAILED false

int FmrbFileService::init(){
  VGAController.suspendBackgroundPrimitiveExecution();
  bool ret = SPIFFS.begin();
  VGAController.resumeBackgroundPrimitiveExecution();
  if(!ret){
    printf("SPIFFS Mount Failed\n");
    return -1;
  }
  printf("SPIFFS Mount OK\n");
  m_opened=true;
  return 0;
}

char* FmrbFileService::load(){
  printf("Reading file: %s\r\n", DEFAULT_TEST_PATH);
  if(!m_opened) return NULL;

  VGAController.suspendBackgroundPrimitiveExecution();
  File file = SPIFFS.open(DEFAULT_TEST_PATH);
  if(!file || file.isDirectory()){
    printf("- failed to open file for reading\n");
    VGAController.resumeBackgroundPrimitiveExecution();
    return NULL;
  }
  printf("- read from file: size=%d\n",(int)file.size());
  char* buff = (char*)fmrb_spi_malloc((int)file.size()+1);
  if(!buff){
    printf("malloc error\n");
    VGAController.resumeBackgroundPrimitiveExecution();
    return NULL;
  }
  VGAController.resumeBackgroundPrimitiveExecution();
  file.read((uint8_t*)buff,(size_t)file.size());
  return buff;
}
int FmrbFileService::save(char* buff){
  printf("Writing file: %s\r\n", DEFAULT_TEST_PATH);
  if(!m_opened) return -1;

  VGAController.suspendBackgroundPrimitiveExecution();
  File file = SPIFFS.open(DEFAULT_TEST_PATH, FILE_WRITE);
  if(!file){
    printf("- failed to open file for writing\n");
    VGAController.resumeBackgroundPrimitiveExecution();
    return -1;
  }
  if(file.print(buff)){
    printf("- file written\n");
  } else {
    printf("- write failed\n");
    VGAController.resumeBackgroundPrimitiveExecution();
    return -1;
  }
  VGAController.resumeBackgroundPrimitiveExecution();
  return 0;
}

EditLine* EditLine::create_line(void)
{
  EditLine* line = (EditLine*)fmrb_spi_malloc(sizeof(EditLine));
  if(line){
    if(line->init(NULL) < 0){
      free(line);
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
    free(line);
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

  printf("p:%d length=%d buffsize=%d\n",pos,length,buff_size);
  if( length+1+1 > buff_size) // Text lenght + null char + new char >= cuurent buff size
  {
    printf("realloc block(text_p:%p new buff size:%d)\n",text,buff_size+EDITLINE_BLOCK_SIZE);
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

  printf("%p, %p, %d\n",text+pos+1,text+pos,length-pos+1);
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

  printf("%p, %p, %d\n",text+pos+1,text+pos,length-pos+1);
  memmove(text+pos-1,text+pos,length-pos+1);
  length-=1;

  printf("p:%d length=%d buffsize=%d\n",pos,length,buff_size);
  if( length+1 <= buff_size - EDITLINE_BLOCK_SIZE) // Text lenght + null char < cuurent buff size - BLOCK
  {
    printf("realloc block(text_p:%p new buff size:%d)\n",text,buff_size-EDITLINE_BLOCK_SIZE);
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
  
  printf("start:%d end:%d length=%d buffsize=%d\n",start_pos,end_pos,length,buff_size);
  if( length+1 <= buff_size - EDITLINE_BLOCK_SIZE) // Text lenght + null char < cuurent buff size - BLOCK
  {
    printf("realloc block(text_p:%p new buff size:%d)\n",text,buff_size-EDITLINE_BLOCK_SIZE);
    text = (char*)fmrb_spi_realloc(text,buff_size-EDITLINE_BLOCK_SIZE);
    if (NULL==text)
    {
      free(buff);
      return NULL;
    }
    buff_size -= EDITLINE_BLOCK_SIZE;
  }
  return buff;
}


/*********************
 * FmrbEditor
 * 
 */
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

  printf("File service init\n");
  //file_service.init();
  printf("Editor begin\n");

  wait_key(0x0D);

  Terminal.clear();
  move_cursor(m_lineno_shift+1,1);

  load(null_script);
  //load_demo_file();

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
              insert_ret();
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
                //save_file();
                break;
              case 0x52: // ESC OP : F3
                printf("F3\n");
                //load_file();
                break;
              case 0x53: // ESC OP : F4
                printf("F4\n");
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
                Terminal.read();
                printf("F5\n");
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
        free(line_p->text);
        free(line_p);
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
  Terminal.write("\e[30m\e[46m  SAVE  \e[0m");
  move(21,bottom);
  Terminal.write("\e[30m\e[46m  LOAD  \e[0m");
  move(31,bottom);
  Terminal.write("\e[30m\e[46m  RUN   \e[0m");
  move(41,bottom);
  Terminal.write("\e[30m\e[46m  DEMO  \e[0m");

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
  //printf("INSERT(%c):text:%s\n",c,line->text);
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
    free(cut_test);
    return;
  }
  new_line->prev = line;
  new_line->next = line->next;
  line->next = new_line;
  m_total_line++;
  free(cut_test);
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
  //printf("BS:text:%s\n",line->text);
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
  printf("total_length=%d\n",total_length);
  char* buff = (char*)fmrb_spi_malloc(total_length);
  if(NULL==buff){
    printf("cannot allocate memory!\n");
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
  printf("%s\n",buff);
  return buff;
}

void FmrbEditor::clear_buffer(){
  printf("clear_buffer\n");
  EditLine* line = m_buff_head;
  while(line)
  {
    if(line->text){
      free(line->text);
    }
    EditLine* old = line;
    line = line->next;
    free(old);
  }
  m_buff_head = NULL;
}

void FmrbEditor::load_file(){
  printf("load_file\n");
  //clear current buffer
  clear_buffer();
  char* buff = file_service.load();
  move_cursor(m_lineno_shift+1,1);
  load(buff);
  update();
  printf("load_file done\n");
}

void FmrbEditor::load_demo_file(){
  //clear current buffer
  clear_buffer();
  move_cursor(m_lineno_shift+1,1);
  load(sample_script);
  update();
}

void FmrbEditor::save_file(){
  printf("save_file\n");
  char* buff = dump_script();
  if(buff){
    file_service.save(buff);
    free(buff);
  }else{
    printf("dump_script error\n");
  }
  printf("save_file done\n");
}