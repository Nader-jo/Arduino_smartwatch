#include <SoftwareSerial.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <TFT_ILI9163C.h>

//PINS
#define __CS 10
#define __DC 9
#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define YELLOW  0xFFE0  
#define WHITE   0xFFFF
uint8_t MenuPosition = 1,lastMenuPosition=1,SettingsMenuPosition = 1,lastSettingsMenuPosition=1,UtililyMenuPosition = 1,lastUtililyMenuPosition=1;
uint8_t buttonUp = 5;
uint8_t buttonDown = 7;
uint8_t buttonSelect = 6;
uint8_t bluetoothTx = 4; //Bluetooth tx pin on arduino
uint8_t bluetoothRx = 3; //Bluetooth rx pin on arduino
long timeSet=0;
uint8_t LED_status =0,prev_LED_status =0,Chrono_status =0,prev_Chrono_status =0,TorchMenuPosition=1,lastTorchMenuPosition=1,ChronoMenuPosition=1,lastChronoMenuPosition=1;
SoftwareSerial bluetooth(bluetoothTx, bluetoothRx);
uint8_t is_menu = 0;

TFT_ILI9163C TFT = TFT_ILI9163C(__CS, __DC);
  const uint16_t GREY       =     TFT.Color565(64,64,64);
  const uint16_t LIGHTBLUE  =     TFT.Color565(64,64,255);
  const uint16_t LIGHTRED   =     TFT.Color565(255,64,64);
  const uint16_t DARKBLUE   =     TFT.Color565(0,0,128);
  const uint16_t DARKRED    =     TFT.Color565(128,0,0);
uint8_t ccenterx,ccentery;//center x,y of the clock
const uint8_t cradius  =   63;//radius of the clock
const float scosConst   =   0.0174532925;
float sx = 0, sy = 1, mx = 1, my = 0, hx = -1, hy = 0;
float sdeg=0, mdeg=0, hdeg=0;
uint8_t prev=0,face=1,prevFace=1,s_timer=0,ms_timer=0;
long previousMillis = 0,timerBegin=0,timerEnd=0;
uint8_t osx,osy,omx,omy,ohx,ohy;
uint8_t x0 = 0, x1 = 0, yy0 = 0, yy1 = 0;
uint8_t hh,mm,ss,last_ss,hSet,mSet,sSet,alarmH,alarmM,chronoTimer=0;  //containers for current time
static uint8_t conv2d(const char* p) {
  uint8_t v = 0;
  if ('0' <= *p && *p <= '9') v = *p - '0';
  return 10 * v + *++p - '0';
}
// instead of using TFT.width() and TFT.height() set constant values
// (we can change the size of the game easily that way)
#define TFTW            128     // screen width
#define TFTH            128     // screen height
#define TFTW2            64     // half screen width
#define TFTH2            64     // half screen height
// game constant
#define SPEED             1
#define GRAVITY         9.8
#define JUMP_FORCE     2.15
#define SKIP_TICKS     20.0     // 1000 / 50fps
#define MAX_FRAMESKIP     5
// bird size
#define BIRDW             8     // bird width
#define BIRDH             8     // bird height
#define BIRDW2            4     // half width
#define BIRDH2            4     // half height
// pipe size
#define PIPEW            12     // pipe width
#define GAPHEIGHT        36     // pipe gap height
// floor size
#define FLOORH           20     // floor height (from bottom of the screen)
// grass size
#define GRASSH            4     // grass height (inside floor, starts at floor y)

// background
const uint16_t BCKGRDCOL = TFT.Color565(138,235,244);
// bird
const uint16_t BIRDCOL = TFT.Color565(255,254,174);
// pipe
const uint16_t PIPECOL  = TFT.Color565(99,255,78);
// pipe highlight
const uint16_t PIPEHIGHCOL  = TFT.Color565(250,255,250);
// pipe seam
const uint16_t PIPESEAMCOL  = TFT.Color565(0,0,0);
// floor
const uint16_t FLOORCOL = TFT.Color565(246,240,163);
// grass (col2 is the stripe color)
const uint16_t GRASSCOL  = TFT.Color565(141,225,87);
const uint16_t GRASSCOL2 = TFT.Color565(156,239,88);

// bird sprite
// bird sprite colors (Cx name for values to keep the array readable)
#define C0 BCKGRDCOL
#define C1 TFT.Color565(195,165,75)
#define C2 BIRDCOL
#define C3 WHITE
#define C4 RED
#define C5 TFT.Color565(251,216,114)
static unsigned int birdcol[] =
{ C0, C0, C1, C1, C1, C1, C1, C0,
  C0, C1, C2, C2, C2, C1, C3, C1,
  C0, C2, C2, C2, C2, C1, C3, C1,
  C1, C1, C1, C2, C2, C3, C1, C1,
  C1, C2, C2, C2, C2, C2, C4, C4,
  C1, C2, C2, C2, C1, C5, C4, C0,
  C0, C1, C2, C1, C5, C5, C5, C0,
  C0, C0, C1, C5, C5, C5, C0, C0};

// bird structure
static struct BIRD {
  unsigned char x, y, old_y;
  unsigned int col;
  float vel_y;
} bird;
// pipe structure
static struct PIPE {
  char x, gap_y;
  unsigned int col;
} pipe;

// score
static short score;
// temporary x and y var
static short tmpx, tmpy;

// ---------------
// draw pixel
// ---------------
// faster drawPixel method by inlining calls and using setAddrWindow and pushColor
// using macro to force inlining
#define drawPixel(a, b, c) TFT.setAddrWindow(a, b, a, b); TFT.pushColor(c)

// ---------------
// initial setup
// ---------------
void setup(void) {
  Serial.begin(9600);
  DDRD &= ~(1<<PD5);
  DDRD &= ~(1<<PD6);
  DDRD &= ~(1<<PD7);
  pinMode(A0,OUTPUT);
  pinMode(A1,OUTPUT);
  pinMode(2,OUTPUT);
  digitalWrite(2, HIGH);
  TFT.begin();
  TFT.setRotation(2);
  BootText();
  delay(1000);
  ccenterx = TFT.width()/2;
  ccentery = TFT.height()/2;
  osx = ccenterx;
  osy = ccentery;
  omx = ccenterx;
  omy = ccentery;
  ohx = ccenterx;
  ohy = ccentery;
  
  //get current time from compiler
  hSet = conv2d(__TIME__);
  mSet = conv2d(__TIME__+3);
  sSet = conv2d(__TIME__+6)+20;
  hh = hSet;
  mm = mSet;
  ss = sSet;
  last_ss = ss;
  alarmH = 4;
  alarmM = 20;
vibrator(0);
}
void loop() { 
    if((is_menu == 0)&&(select()==1)){
       is_menu = 1;
       displayMenu(true,MenuPosition); 
      while (is_menu == 1){
        MenuPosition = correctMenuPosition( navigationMenu(is_menu)+ lastMenuPosition,4);
        if ((lastMenuPosition != MenuPosition)){
          //displayMenu(true,MenuPosition); 
          updateMenu(MenuPosition,lastMenuPosition);
          lastMenuPosition=MenuPosition;
          delay(120);
        }
        if ((select()==1)&&(MenuPosition==1)){
          timeTracking();
          watchFace(face,is_menu,0);
          is_menu = 0;
        }else if ((select()==1)&&(MenuPosition==4)){
          game_start();
          game_loop();
          game_over();
          is_menu = 4;
          while ((is_menu == 4)&&(select()==0)){
          }
          if ((is_menu == 4)&&(select()==1))is_menu = 0;
        }else if ((select()==1)&&(MenuPosition==2)){
          is_menu = 2;
          timeTracking();
          setTime();
          timeSet = millis()/1000;
          if ((is_menu == 2)&&(select()==1))is_menu = 0;
        }else if ((select()==1)&&(MenuPosition==3)){
          is_menu = 3;
          displaySettings(true,SettingsMenuPosition);
          while (is_menu == 3){
            SettingsMenuPosition = correctMenuPosition( navigationMenu(is_menu)+ lastSettingsMenuPosition,4);
            if ((lastSettingsMenuPosition != SettingsMenuPosition)){
              updateSettings(SettingsMenuPosition,lastSettingsMenuPosition); 
              lastSettingsMenuPosition=SettingsMenuPosition;
              delay(120);
            }if ((select()==1)&&(SettingsMenuPosition==4)){
              is_menu = 0;
            }if ((select()==1)&&(SettingsMenuPosition==3)){
              credit();
              while ((is_menu == 3)&&(select()==0)){
              }
              is_menu = 1;
            }if ((select()==1)&&(SettingsMenuPosition==1)){
              watchtype();
              is_menu = 5;
              tickBox(face);
              while ((is_menu == 5)&&(select()==0)){
                face = selectWatchType(face);
                if (face != prevFace) {
                  delay(200);
                  tickBox(face);
                  prevFace = face;
                }
              }
              is_menu = 1;
            }if ((select()==1)&&(SettingsMenuPosition==2)){
              is_menu = 6;
              displayUtilities(true,UtililyMenuPosition);
              while (is_menu == 6){
                UtililyMenuPosition = correctMenuPosition( navigationMenu(is_menu)+ lastUtililyMenuPosition,4);
                if ((lastUtililyMenuPosition != UtililyMenuPosition)){
                  updateUtilities(UtililyMenuPosition,lastUtililyMenuPosition); 
                  lastUtililyMenuPosition=UtililyMenuPosition;
                  delay(120);
                }if ((select()==1)&&(UtililyMenuPosition==4)){
                  is_menu = 1;
                }if ((select()==1)&&(UtililyMenuPosition==1)){
                  setAlarm();
                  while ((is_menu == 6)&&(select()==0)){
                  }
                  is_menu = 3;
                  displayUtilities(true,UtililyMenuPosition);
                  is_menu = 6;
                }if ((select()==1)&&(UtililyMenuPosition==2)){
                  torch();
                  is_menu = 7;
                  torchBox(LED_status);
                  while (is_menu == 7){
                    TorchMenuPosition = correctMenuPosition((TorchMenuPosition + navigationMenu(is_menu)),2) ;
                    if(TorchMenuPosition!=lastTorchMenuPosition){
                      updateTorch();
                      delay(120);
                      lastTorchMenuPosition =TorchMenuPosition;
                    }
                    if (TorchMenuPosition==1) LED_status =(LED_status+select())%2;
                    if ((LED_status != prev_LED_status)&&(select())) {
                      delay(200);
                      prev_LED_status = LED_status;
                      torchBox(LED_status);  
                    }
                    if ((select()==1)&&(TorchMenuPosition==2)){
                      is_menu = 3;
                    }
                  }
                }if ((select()==1)&&(UtililyMenuPosition==3)){
                  Chronometer();
                  is_menu = 8;
                  if (Chrono_status==1)timerEnd=millis();
                  printTimer();
                  while (is_menu == 8){
                    ChronoMenuPosition = correctMenuPosition((ChronoMenuPosition + navigationMenu(is_menu)),2) ;
                    runChrono(Chrono_status);
                    if ((select())&&(Chrono_status==0)&&((timerEnd-timerBegin)==0)) {
                      Chrono_status = 1;
                      timerBegin=millis();
                      runChrono(Chrono_status);
                      delay(200);
                    }
                    if ((select())&&(Chrono_status==1)&&((timerEnd-timerBegin)!=0)) {
                      Chrono_status = 0;
                      timerEnd=millis();
                      runChrono(Chrono_status);
                      delay(200);
                    }
                    if ((select())&&(Chrono_status==0)&&((timerEnd-timerBegin)!=0)) {
                      Chrono_status = 2;
                      runChrono(Chrono_status);
                      Chrono_status = 0;
                      delay(200);
                    }
                    if(ChronoMenuPosition!=lastChronoMenuPosition){
                      updateChrono();
                      delay(50);
                      lastChronoMenuPosition =ChronoMenuPosition;
                    }
                    printTimer();
                    if ((select()==1)&&(ChronoMenuPosition==2)){
                      is_menu = 3;
                    }
                  }
                }
              }
            }
          }
        }
      }
    }else if ((is_menu == 0)&&(select()==0)){
        timeTracking();
        if (last_ss!=ss) {
          prev = watchFace(face,is_menu,0);
          last_ss = ss;
        }
        if ((hh==alarmH)&&(mm==alarmM))vibrator(ss%2);else vibrator(0);
        is_menu = 0;
      }  
 }
void torchBox(uint8_t n){
  if(n==1) digitalWrite(A1, HIGH);
  else digitalWrite(A1, LOW);
}
 void torch(void){
  TFT.fillScreen(BLACK); 
  TFT.setCursor(3*TFT.width()/12, TFT.height()/12);
  TFT.setTextColor(GREEN); 
  TFT.setTextSize(2);
  TFT.println("Torch");
  updateTorch();
 }
 void updateTorch(void){
  uint8_t k,c;
  for (k = 1; k < 3; k++) {
      uint8_t cx, cy, x, y, w, h;
      //  center
      cx = TFT.width()/2;
      cy = (2*k+1)*TFT.height()/8;
      //  size
      w = TFT.width()-20;
      h = (TFT.height()-20)/4;
      x = cx - w / 2;
      y = cy - h / 2;
      if (TorchMenuPosition==k) Button(x, y, w, h,RED);
      else Button(x, y, w, h,BLUE);
      TFT.setTextColor(WHITE);
      TFT.setTextSize(1);
      switch (k){
         case 1:{
            TFT.setCursor(5.5*TFT.width()/15,cy-2);
            TFT.println("On/Off");
            break;}
         case 2:{
            TFT.setCursor(6*TFT.width()/15,cy-2);
            TFT.println("Back");
            break;}
      }
  }
}
void updateChrono(void){
  uint8_t k,c;
  for (k = 2; k < 4; k++) {
      uint8_t cy=StandardGUIMenu(k, ChronoMenuPosition+1);
      TFT.setTextColor(WHITE);
      TFT.setTextSize(1);
      switch (k-1){
         case 1:{
            TFT.setCursor(3*TFT.width()/15,cy-2);
            TFT.println("Go/Stop/Reset");
            break;}
         case 2:{
            TFT.setCursor(6*TFT.width()/15,cy-2);
            TFT.println("Back");
            break;}
      }
  }
}
void printTimer(void){
   TFT.fillRect(32,28,108,21,BLACK);
  TFT.setCursor(32, 28);
  TFT.setTextColor(WHITE); 
  TFT.setTextSize(2);
  printDigits(int(s_timer));
  TFT.print(".");
  printDigits(int(ms_timer));
}
void runChrono(uint8_t k){
  switch (k){
    case 0:{
      //s_timer=(timerEnd-timerBegin)/1000;
      //ms_timer=(timerEnd-timerBegin)%1000;
    break;}
    case 1:{ 
      s_timer=(millis()-timerBegin)/1000;
      ms_timer=(millis()-timerBegin)%1000;
    break;}
    case 2:{ 
      s_timer=0;
      ms_timer=0;
      timerBegin=0;
      timerEnd=0;
    break;}
  }
}
 void Chronometer(void){
  TFT.fillScreen(BLACK); 
  TFT.setCursor(3*TFT.width()/12, TFT.height()/12);
  TFT.setTextColor(GREEN); 
  TFT.setTextSize(2);
  TFT.println("Chrono");
  runChrono(0);
  updateChrono();
 }
  void timeTracking(void){
    ss=(sSet+(millis())/1000-timeSet)%60;
    mm=(mSet+(sSet+(millis())/1000-timeSet)/60)%60;
    hh=(hSet+(mSet*60+sSet+ (millis())/1000-timeSet)/3600)%24;
  }
  
  void setAlarm(void){ 
    uint8_t prevH =alarmH,prevM=alarmM;
    TFT.fillScreen(BLACK); 
    TFT.setCursor(TFT.width()/9, TFT.height()/12);
    TFT.setTextColor(GREEN); 
    TFT.setTextSize(2);
    TFT.println("Set Alarm");
    uint8_t h_local=alarmH;
    uint8_t m_local=alarmM;
    TFT.setCursor(20, 34);
    TFT.setTextColor(YELLOW); 
    TFT.setTextSize(3);
    printDigits(int(h_local));
    TFT.setTextColor(WHITE); 
    TFT.print(":");
    printDigits(int(m_local));
    delay(100);
    while(select()==0){
      if (h_local!=prevH){
        TFT.fillRect(20, 34,40,30 , BLACK);
        TFT.setCursor(20, 34);
        TFT.setTextColor(YELLOW); 
        TFT.setTextSize(3);
        printDigits(int(h_local));
        prevH=h_local;
      }
      h_local = h_local - navigationMenu(2);
      if(h_local<0) h_local= 23; else if (h_local>23) h_local=0;
      delay(120); 
    }
    delay(100);
    TFT.fillRect(20, 34,40,30 , BLACK);
    TFT.setCursor(20, 34);
    TFT.setTextColor(WHITE); 
    TFT.setTextSize(3);
    printDigits(int(h_local));
    prevM=m_local-1;
    while(select()==0){
      if (m_local!=prevM){
        TFT.fillRect(74, 34,40,30, BLACK);
        TFT.setCursor(74, 34);
        TFT.setTextColor(YELLOW); 
        TFT.setTextSize(3);
        printDigits(int(m_local));
        prevM=m_local;
      }
      m_local = m_local - navigationMenu(2);
      if(m_local<0) m_local= 59; else if (m_local>59) m_local=0;
      delay(120); 
    }
    TFT.fillRect(74, 34,40,30, BLACK);
    TFT.setCursor(74, 34);
    TFT.setTextColor(WHITE); 
    TFT.setTextSize(3);
    printDigits(int(m_local));
    alarmH=h_local;
    alarmM=m_local;
  }
  
  void setTime(void){
    uint8_t prevH =hh,prevM=mm,prevS=ss;
    TFT.fillScreen(BLACK); 
    TFT.setCursor(TFT.width()/8, TFT.height()/12);
    TFT.setTextColor(GREEN); 
    TFT.setTextSize(2);
    TFT.println("Set Time");
    uint8_t h_local=hh;
    uint8_t m_local=mm;
    uint8_t s_local=ss;
    TFT.setCursor(20, 34);
    TFT.setTextColor(YELLOW); 
    TFT.setTextSize(3);
    printDigits(int(h_local));
    TFT.setTextColor(WHITE); 
    TFT.print(":");
    printDigits(int(m_local));
    TFT.setCursor(53, 68);
    TFT.setTextSize(2);
    printDigits(int(s_local));
    delay(100);
    while(select()==0){
      if (h_local!=prevH){
        TFT.fillRect(20, 34,40,30 , BLACK);
        TFT.setCursor(20, 34);
        TFT.setTextColor(YELLOW); 
        TFT.setTextSize(3);
        printDigits(int(h_local));
        prevH=h_local;
      }
      h_local = h_local - navigationMenu(2);
      if(h_local<0) h_local= 23; else if (h_local>23) h_local=0;
      delay(120); 
    }
    delay(100);
    TFT.fillRect(20, 34,40,30 , BLACK);
    TFT.setCursor(20, 34);
    TFT.setTextColor(WHITE); 
    TFT.setTextSize(3);
    printDigits(int(h_local));
    prevM=m_local-1;
    while(select()==0){
      if (m_local!=prevM){
        TFT.fillRect(74, 34,40,30, BLACK);
        TFT.setCursor(74, 34);
        TFT.setTextColor(YELLOW); 
        TFT.setTextSize(3);
        printDigits(int(m_local));
        prevM=m_local;
      }
      m_local = m_local - navigationMenu(2);
      if(m_local<0) m_local= 59; else if (m_local>59) m_local=0;
      delay(120); 
    }
    delay(100);
    TFT.fillRect(74, 34,40,30, BLACK);
    TFT.setCursor(74, 34);
    TFT.setTextColor(WHITE); 
    TFT.setTextSize(3);
    printDigits(int(m_local));
    prevS=s_local+1;
    while(select()==0){
      if (s_local!=prevS){
        TFT.fillRect(53, 68,40,30, BLACK);
        TFT.setCursor(53, 68);
        TFT.setTextColor(YELLOW); 
        TFT.setTextSize(2);
        printDigits(int(s_local));
        prevS=s_local;
      }
      s_local = s_local - navigationMenu(2);
      if(s_local<0) s_local= 59; else if (s_local>59) s_local=0;
      delay(120); 
    }
    TFT.fillRect(53, 68,40,30, BLACK);
    TFT.setCursor(53, 68);
    TFT.setTextColor(WHITE); 
    TFT.setTextSize(2);
    printDigits(int(s_local));
    hSet=h_local;
    mSet=m_local;
    sSet=s_local;
  }
  
uint8_t select(void){
  uint8_t BS = digitalRead(buttonSelect);
  if((BS==1)){
    return(1);
  }else{
  return(0);
  }
}
uint8_t correctMenuPosition(uint8_t MenuPosition,uint8_t k){
  if (MenuPosition<1){
    MenuPosition=k;
  }if (MenuPosition>k){
    MenuPosition=1;
  }
  return MenuPosition;
}
uint8_t selectWatchType(uint8_t face){
  MenuPosition = navigationMenu(5)+face;
  if (MenuPosition<0)MenuPosition=1;
  if (MenuPosition>1)MenuPosition=0;
  return(MenuPosition);
}
void tickBox(uint8_t MenuPosition){
  uint8_t y1=3*TFT.height()/8-((TFT.height()-50)/4)/2;
  uint8_t y2=5*TFT.height()/8-((TFT.height()-50)/4)/2;
  uint8_t x= TFT.width()/2 +(TFT.width()-70)/2;
  if (MenuPosition==1){
  TFT.fillRect(x, y1, 20, 20,BLUE);
  TFT.fillRect(x, y2, 20, 20,BLUE);
  TFT.drawRect(x, y2, 20, 20,RED);
  TFT.fillRect(x, y1, 20, 20,RED);
  }else{
  TFT.fillRect(x, y1, 20, 20,BLUE);
  TFT.fillRect(x, y2, 20, 20,BLUE);
  TFT.drawRect(x, y1, 20, 20,RED);
  TFT.fillRect(x, y2, 20, 20,RED);
  }
}
uint8_t navigationMenu(uint8_t is_menu){
  uint8_t BU= digitalRead(buttonUp);
  uint8_t BD= digitalRead(buttonDown);
  if ((BU>0)&&(is_menu!=0)){
    return (-1);
  }else if ((BD>0)&&(is_menu=1)){
    return (1);
  }else {
    return (0);
  }
}
void updateMenu(uint8_t MenuPosition,uint8_t lastMenuPosition){
  uint8_t k,c;
  char str1[12]="Watch",str2[12]="Set Time",str3[12]="Settings",str4[12]="Game";
  k = MenuPosition;
  uint8_t cx, cy, x, y, w, h;
  //  center
  cx = TFT.width()/2;
  cy = (2*k+1)*TFT.height()/10;
  //  size
  w = TFT.width()-20;
  h = (TFT.height()-20)/5;
  x = cx - w / 2;
  y = cy - h / 2;
  Button(x, y, w, h,RED);
      //TFT.fillRect(x, y, w, h,RED);
  writeLabels(k,cy,str1,str2,str3,str4);
  k = lastMenuPosition;
  //  center
  cy = (2*k+1)*TFT.height()/10;
  y = cy - h / 2;
  Button(x, y, w, h,BLUE);
  writeLabels(k,cy,str1,str2,str3,str4);
}
void Button(uint8_t x,uint8_t y,uint8_t w,uint8_t h,uint16_t COLOR){
  TFT.fillRect(x, y, w, h,COLOR);
  if(COLOR == BLUE){
    TFT.fillTriangle(x, y, x+w, y, x, y+h/3, LIGHTBLUE);
    TFT.fillTriangle(x, y+h, x+w, y+h, x+w, y+3*h/4, DARKBLUE);
  }else if(COLOR == RED){
    TFT.fillTriangle(x, y, x+w, y, x, y+h/3, LIGHTRED);
    TFT.fillTriangle(x, y+h, x+w, y+h, x+w, y+3*h/4, DARKRED);
  }
}
void displayMenu(bool fill,uint8_t MenuPosition){
  TFT.fillScreen(BLACK); 
  TFT.setCursor(4*TFT.width()/12, TFT.height()/12);
  TFT.setTextColor(GREEN); 
  TFT.setTextSize(2);
  TFT.println("Home");
  uint8_t k,c;
  for (k = 1; k < 5; k++) {
      uint8_t cy=StandardGUIMenu(k,MenuPosition);
      writeLabels(k,cy,"Watch","Set Time","Settings","Game");
  }
}

void writeLabels(uint8_t k,uint8_t cy,char str1[12],char str2[12],char str3[12],char str4[12]){
      TFT.setTextColor(WHITE);
      TFT.setTextSize(1);
      switch (k){
         case 1:{
            TFT.setCursor((TFT.width()-5*strlen(str1))/2,cy-2);
            TFT.println(str1);
            break;}
         case 2:{
            TFT.setCursor((TFT.width()-5*strlen(str2))/2,cy-2);
            TFT.println(str2);
            break;}
         case 3:{
             TFT.setCursor((TFT.width()-5*strlen(str3))/2,cy-2);
             TFT.println(str3);
             break;} 
         case 4:{
             TFT.setCursor((TFT.width()-5*strlen(str4))/2,cy-2);
             TFT.println(str4);
             break;}
      }
}

void watchtype(void){
  TFT.fillScreen(BLACK); 
  TFT.setCursor(TFT.width()/24, TFT.height()/12);
  TFT.setTextColor(GREEN); 
  TFT.setTextSize(2);
  TFT.println("Watch Type");
  uint8_t k,c;
  for (k = 1; k < 3; k++) {
      uint8_t cx, cy, x, y, w, h;
      //  center
      cx = TFT.width()/2;
      cy = (2*k+1)*TFT.height()/8;
      //  size
      w = TFT.width()-20;
      h = (TFT.height()-20)/4;
      x = cx - w / 2;
      y = cy - h / 2;
        if (k==MenuPosition){
         Button(x, y, w, h,RED);
          }else Button(x, y, w, h,BLUE);
      TFT.setTextColor(WHITE);
      TFT.setTextSize(1);
      switch (k){
         case 1:{
            TFT.setCursor(2*TFT.width()/14,cy-2);
            TFT.println("Digital");
            break;}
         case 2:{
            TFT.setCursor(2*TFT.width()/14,cy-2);
            TFT.println("Analog");
            break;}
      }
  }
}
void updateSettings(uint8_t MenuPosition, uint8_t lastMenuPosition){
  uint8_t k,c;
  char str1[12]="Watch type",str2[12]="Utilities",str3[12]="About",str4[12]="Back";
  k=MenuPosition;
      uint8_t cx, cy, x, y, w, h;
      //  center
      cx = TFT.width()/2;
      cy = (2*k+1)*TFT.height()/10;
      //  size
      w = TFT.width()-20;
      h = (TFT.height()-20)/5;
      x = cx - w / 2;
      y = cy - h / 2;
     Button(x, y, w, h,RED);
      writeLabels(k,cy,str1,str2,str3,str4);
      k=lastMenuPosition;
      cy = (2*k+1)*TFT.height()/10;
      y = cy - h / 2;
     Button(x, y, w, h,BLUE);
      writeLabels(k,cy,str1,str2,str3,str4);
}

void displaySettings(bool fill,uint8_t MenuPosition){
  TFT.fillScreen(BLACK); 
  TFT.setCursor(1.5*TFT.width()/12, TFT.height()/12);
  TFT.setTextColor(GREEN); 
  TFT.setTextSize(2);
  TFT.println("Settings");
  uint8_t k,c;
  for (k = 1; k < 5; k++) {
      uint8_t cy=StandardGUIMenu(k, MenuPosition);
      writeLabels(k,cy,"Watch type","Utilities","About","Back");
  }
}

void updateUtilities(uint8_t MenuPosition, uint8_t lastMenuPosition){
  uint8_t k,c;
  char str1[12]="Set Alarm",str2[12]="Light",str3[12]="Chronometer",str4[12]="Back";
  k=MenuPosition;
      uint8_t cx, cy, x, y, w, h;
      //  center
      cx = TFT.width()/2;
      cy = (2*k+1)*TFT.height()/10;
      //  size
      w = TFT.width()-20;
      h = (TFT.height()-20)/5;
      x = cx - w / 2;
      y = cy - h / 2;
     Button(x, y, w, h,RED);
      writeLabels(k,cy,str1,str2,str3,str4);
      k=lastMenuPosition;
      cy = (2*k+1)*TFT.height()/10;
      y = cy - h / 2;
     Button(x, y, w, h,BLUE);
      writeLabels(k,cy,str1,str2,str3,str4);
}

uint8_t StandardGUIMenu(uint8_t k,uint8_t MenuPosition){
uint8_t cx, cy, x, y, w, h;
      //  center
      cx = TFT.width()/2;
      cy = (2*k+1)*TFT.height()/10;
      //  size
      w = TFT.width()-20;
      h = (TFT.height()-20)/5;
      x = cx - w / 2;
      y = cy - h / 2;
        if (k==MenuPosition){
           Button(x, y, w, h,RED);
          }else {
           Button(x, y, w, h,BLUE);
          }
      return(cy);
}
void displayUtilities(bool fill,uint8_t MenuPosition){
  TFT.fillScreen(BLACK); 
  TFT.setCursor(TFT.width()/12, TFT.height()/12);
  TFT.setTextColor(GREEN); 
  TFT.setTextSize(2);
  TFT.println("Utilities");
  uint8_t k,c;
  for (k = 1; k < 5; k++) {
      uint8_t cy = StandardGUIMenu(k,MenuPosition);
      writeLabels(k,cy,"Set Alarm","Light","Chronometer","Back");
  }
}
/*int blutoothChat(char c){
    char r;
    char toSend;
    
    if(bluetooth.available()){
//        correctScreen();
        r = bluetooth.read();  
        TFT.setTextColor(GREEN); 
        TFT.setTextSize(1);
        TFT.print(r);
        c=1;
     } else if(Serial.available()){
//        correctScreen();
        toSend = Serial.read();
        TFT.setTextColor(BLUE); 
        TFT.setTextSize(1);
        TFT.print(toSend);
        c=1;
   }else if(c==1){
        TFT.println();
        //vibrator();
        c=0;
  } else if(c==-1){
        TFT.setCursor(4*TFT.width()/18, TFT.height()/15);
        TFT.setTextColor(RED); 
        TFT.setTextSize(2);
        TFT.println("BTchat");
        //vibrator();
        c=0;
  }return c;
}*/
/*void correctScreen(void){
  if (TFT.getCursorY()>113){
    TFT.fillScreen(BLACK); 
    TFT.setCursor(4*TFT.width()/18, TFT.height()/15);
    TFT.setTextColor(RED); 
    TFT.setTextSize(2);
    TFT.println("BTchat");
  }
}*/
void vibrator(uint8_t n){
  if(n==1) digitalWrite(A0, HIGH);
  else digitalWrite(A0, LOW);
}
uint8_t watchFace(uint8_t screen,uint8_t prev,uint8_t LastPrev) { 
   if (prev!= LastPrev)
   {TFT.fillScreen(BLACK); 
   LastPrev = prev;
   if (screen==0) drawClockFace();
   } 
   if (screen==0){
    watch(); prev=screen;
  }else if(screen==1){
    numWatch();prev=screen;
  }return(prev);
}



void numWatch(void){
    TFT.setCursor(6, 24);
    TFT.fillRect(0,0,(TFT.width()-1),75, BLACK);
    //TFT.setCursor(6, 24);
    TFT.setCursor(20, 28);
    TFT.setTextColor(WHITE); 
    //TFT.setTextSize(4);
    TFT.setTextSize(3);
    printDigits(int(hh));
    TFT.print(":");
    printDigits(int(mm));
    //TFT.setCursor(98, 61);
    //TFT.setTextSize(2);
    TFT.setCursor(84, 61);
    TFT.setTextSize(2);
    printDigits(int(ss));
    AlarmIcon(7,90,true);
    messageIcon(37,90,true);
    callIcon(67,90,true);
    bluetoothIcon(97,90,true);
//    TFT.setCursor(31, 112);
//    TFT.setTextColor(WHITE); 
//    TFT.setTextSize(1);
//    TFT.print(__DATE__);
}

void printDigits(uint8_t digits){
  // utility function for digital clock display: prints leading 0
   if(digits < 10)
    TFT.print('0');
   TFT.print(digits);
 }
 
void watch(void){
    eraseClockHands(hh,mm,ss);
    drawClockHands(hh,mm,ss);
    /*AlarmIcon(0,0,true);
    messageIcon(104,0,true);
    callIcon(0,104,true);
    bluetoothIcon(104,104,true);*/
}
void drawClockFace(){
  TFT.fillCircle(ccenterx, ccentery, cradius, WHITE);
  TFT.fillCircle(ccenterx, ccentery, cradius-4, BLACK);
  // Draw 12 lines
  for(uint8_t i = 0; i<360; i+= 30) {
    sx = cos((i-90)*scosConst);
    sy = sin((i-90)*scosConst);
    x0 = sx*(cradius-4)+ccenterx;
    yy0 = sy*(cradius-4)+ccentery;
    x1 = sx*(cradius-11)+ccenterx;
    yy1 = sy*(cradius-11)+ccentery;
    TFT.drawLine(x0, yy0, x1, yy1, YELLOW);
  }
}
void eraseClockHands(uint8_t h,uint8_t m,uint8_t s){
  // Erase just old hand positions
  TFT.drawLine(ohx, ohy, ccenterx+1, ccentery+1, BLACK);  
  TFT.drawLine(omx, omy, ccenterx+1, ccentery+1, BLACK);  
  TFT.drawLine(osx, osy, ccenterx+1, ccentery+1, BLACK);
}
void drawClockHands(uint8_t h,uint8_t m,uint8_t s){
  // Pre-compute hand degrees, x & y coords for a fast screen update
  sdeg = s * 6;                  // 0-59 -> 0-354
  mdeg = m * 6 + sdeg * 0.01666667;  // 0-59 -> 0-360 - includes seconds
  hdeg = h * 30 + mdeg * 0.0833333;  // 0-11 -> 0-360 - includes minutes and seconds
  hx = cos((hdeg-90)*scosConst);    
  hy = sin((hdeg-90)*scosConst);
  mx = cos((mdeg-90)*scosConst);    
  my = sin((mdeg-90)*scosConst);
  sx = cos((sdeg-90)*scosConst);    
  sy = sin((sdeg-90)*scosConst); 
  // Draw new hand positions  
  TFT.drawLine(hx*(cradius-28)+ccenterx+1, hy*(cradius-28)+ccentery+1, ccenterx+1, ccentery+1, WHITE);
  TFT.drawLine(mx*(cradius-17)+ccenterx+1, my*(cradius-17)+ccentery+1, ccenterx+1, ccentery+1, WHITE);
  TFT.drawLine(sx*(cradius-14)+ccenterx+1, sy*(cradius-14)+ccentery+1, ccenterx+1, ccentery+1, RED);
  TFT.fillCircle(ccenterx+1, ccentery+1, 3, RED);  
    // Update old x&y coords
  osx = sx*(cradius-14)+ccenterx+1;
  osy = sy*(cradius-14)+ccentery+1;
  omx = mx*(cradius-17)+ccenterx+1;
  omy = my*(cradius-17)+ccentery+1;
  ohx = hx*(cradius-28)+ccenterx+1;
  ohy = hy*(cradius-28)+ccentery+1;
}

unsigned long BootText() {
  unsigned long start = micros();
    TFT.fillScreen(BLACK); 
  TFT.setCursor(2, 2);
  TFT.setTextColor(GREEN); 
  TFT.setTextSize(1);
  TFT.println("Nader Industries. Inc");
  TFT.println("  ");
  delay(400);
  TFT.setTextColor(RED);
  TFT.println(" SmartWatch 0.7");
  TFT.println(" Booting ...");
  delay(1000);
  TFT.setTextColor(BLUE);
  TFT.println("  ");
  TFT.println("                ");
  TFT.println("      |\\\\  [ |");
  TFT.println("      ||\\\\  ||");
  TFT.println("      || \\\\ ||");
  TFT.println("      | ] \\\\||");
  TFT.println("  ");
  TFT.println("  ");
  delay(400);
  TFT.setTextColor(WHITE);
    TFT.println(" Feb 2017"); 
    TFT.println(" Tunis, Tunisia");
    TFT.fillScreen(BLACK); 
  return micros() - start;
}
void credit(void){
  TFT.fillScreen(BLACK); 
  TFT.setCursor(1, 1);
  TFT.setTextColor(CYAN); 
  TFT.setTextSize(1);
  TFT.println("Nader Industries. Inc");
  TFT.setTextColor(WHITE); 
  TFT.println("This Smartwatch was  designed and built byNader Jomaa Software engineer.");
  TFT.println(" ");
  TFT.println("nader.jomaa@yahoo.fr");
  TFT.println(" ");
  TFT.println("Made in Tunisia.");
  TFT.println(" ");
  TFT.println("A special thanks to  Themistokle Benetatoswho made the floppy  bird game.");
}
// ---------------
// game loop
// ---------------
void game_loop() {
  // ===============
  // prepare game variables
  // draw floor
  // ===============
  // instead of calculating the distance of the floor from the screen height each time store it in a variable
  unsigned char GAMEH = TFTH - FLOORH;
  // draw the floor once, we will not overwrite on this area in-game
  // black line
  TFT.drawFastHLine(0, GAMEH, TFTW, BLACK);
  // grass and stripe
  TFT.fillRect(0, GAMEH+1, TFTW2, GRASSH, GRASSCOL);
  TFT.fillRect(TFTW2, GAMEH+1, TFTW2, GRASSH, GRASSCOL2);
  // black line
  TFT.drawFastHLine(0, GAMEH+GRASSH, TFTW, BLACK);
  // mud
  TFT.fillRect(0, GAMEH+GRASSH+1, TFTW, FLOORH-GRASSH, FLOORCOL);
  // grass x position (for stripe animation)
  char grassx = TFTW;
  // game loop time variables
  double delta, old_time, next_game_tick, current_time;
  next_game_tick = current_time = millis();
  int loops;
  // passed pipe flag to count score
  bool passed_pipe = false;
  // temp var for setAddrWindow
  unsigned char px;
  
  while (1) {
    loops = 0;
    while( millis() > next_game_tick && loops < MAX_FRAMESKIP) {
      // ===============
      // input
      // ===============
      if ( !(PIND & (select()==0)) ) {
        // if the bird is not too close to the top of the screen apply jump force
        if (bird.y > BIRDH2*0.5) bird.vel_y = -JUMP_FORCE;
        // else zero velocity
        else bird.vel_y = 0;
      }
      
      // ===============
      // update
      // ===============
      // calculate delta time
      // ---------------
      old_time = current_time;
      current_time = millis();
      delta = (current_time-old_time)/1000;

      // bird
      // ---------------
      bird.vel_y += GRAVITY * delta;
      bird.y += bird.vel_y;

      // pipe
      // ---------------
      pipe.x -= SPEED;
      // if pipe reached edge of the screen reset its position and gap
      if (pipe.x < -PIPEW) {
        pipe.x = TFTW;
        pipe.gap_y = random(10, GAMEH-(10+GAPHEIGHT));
      }

      // ---------------
      next_game_tick += SKIP_TICKS;
      loops++;
    }

    // ===============
    // draw
    // ===============
    // pipe
    // ---------------
    // we save cycles if we avoid drawing the pipe when outside the screen
    if (pipe.x >= 0 && pipe.x < TFTW) {
      // pipe color
      TFT.drawFastVLine(pipe.x+3, 0, pipe.gap_y, PIPECOL);
      TFT.drawFastVLine(pipe.x+3, pipe.gap_y+GAPHEIGHT+1, GAMEH-(pipe.gap_y+GAPHEIGHT+1), PIPECOL);
      // highlight
      TFT.drawFastVLine(pipe.x, 0, pipe.gap_y, PIPEHIGHCOL);
      TFT.drawFastVLine(pipe.x, pipe.gap_y+GAPHEIGHT+1, GAMEH-(pipe.gap_y+GAPHEIGHT+1), PIPEHIGHCOL);
      // bottom and top border of pipe
      drawPixel(pipe.x, pipe.gap_y, PIPESEAMCOL);
      drawPixel(pipe.x, pipe.gap_y+GAPHEIGHT, PIPESEAMCOL);
      // pipe seam
      drawPixel(pipe.x, pipe.gap_y-6, PIPESEAMCOL);
      drawPixel(pipe.x, pipe.gap_y+GAPHEIGHT+6, PIPESEAMCOL);
      drawPixel(pipe.x+3, pipe.gap_y-6, PIPESEAMCOL);
      drawPixel(pipe.x+3, pipe.gap_y+GAPHEIGHT+6, PIPESEAMCOL);
    }
    // erase behind pipe
    if (pipe.x <= TFTW) TFT.drawFastVLine(pipe.x+PIPEW, 0, GAMEH, BCKGRDCOL);

    // bird
    // ---------------
    tmpx = BIRDW-1;
    do {
          px = bird.x+tmpx+BIRDW;
          // clear bird at previous position stored in old_y
          // we can't just erase the pixels before and after current position
          // because of the non-linear bird movement (it would leave 'dirty' pixels)
          tmpy = BIRDH - 1;
          do {
            drawPixel(px, bird.old_y + tmpy, BCKGRDCOL);
          } while (tmpy--);
          // draw bird sprite at new position
          tmpy = BIRDH - 1;
          do {
            drawPixel(px, bird.y + tmpy, birdcol[tmpx + (tmpy * BIRDW)]);
          } while (tmpy--);
    } while (tmpx--);
    // save position to erase bird on next draw
    bird.old_y = bird.y;

    // grass stripes
    // ---------------
    grassx -= SPEED;
    if (grassx < 0) grassx = TFTW;
    TFT.drawFastVLine( grassx    %TFTW, GAMEH+1, GRASSH-1, GRASSCOL);
    TFT.drawFastVLine((grassx+64)%TFTW, GAMEH+1, GRASSH-1, GRASSCOL2);

    // ===============
    // collision
    // ===============
    // if the bird hit the ground game over
    if (bird.y > GAMEH-BIRDH) break;
    // checking for bird collision with pipe
    if (bird.x+BIRDW >= pipe.x-BIRDW2 && bird.x <= pipe.x+PIPEW-BIRDW) {
      // bird entered a pipe, check for collision
      if (bird.y < pipe.gap_y || bird.y+BIRDH > pipe.gap_y+GAPHEIGHT) break;
      else passed_pipe = true;
    }
    // if bird has passed the pipe increase score
    else if (bird.x > pipe.x+PIPEW-BIRDW && passed_pipe) {
      passed_pipe = false;
      // erase score with background color
      TFT.setTextColor(BCKGRDCOL);
      TFT.setCursor( TFTW2, 4);
      TFT.print(score);
      // set text color back to white for new score
      TFT.setTextColor(WHITE);
      // increase score since we successfully passed a pipe
      score++;
    }

    // update score
    // ---------------
    TFT.setCursor( TFTW2, 4);
    TFT.print(score);
  }
  
  // add a small delay to show how the player lost
  vibrator(1);
  delay(1200);
  vibrator(0);
}

// ---------------
// game start
// ---------------
void game_start() {
  TFT.fillScreen(BLACK);
  TFT.fillRect(10, TFTH2 - 20, TFTW-20, 1, WHITE);
  TFT.fillRect(10, TFTH2 + 32, TFTW-20, 1, WHITE);
  TFT.setTextColor(WHITE);
  TFT.setTextSize(3);
  // half width - num char * char width in pixels
  TFT.setCursor( TFTW2-(6*9), TFTH2 - 16);
  TFT.println("FLAPPY");
  TFT.setTextSize(3);
  TFT.setCursor( TFTW2-(6*9), TFTH2 + 8);
  TFT.println("-BIRD-");
  TFT.setTextSize(0);
  TFT.setCursor( 10, TFTH2 - 28);
  TFT.println("ATMEGA328");
  TFT.setCursor( TFTW2 - (12*3) - 1, TFTH2 + 34);
  TFT.println("press button");
  while (1) {
    // wait for push button
    if ( !(PIND & (select()==0)) ) break;
  }
  
  // init game settings
  game_init();
}

// ---------------
// game init
// ---------------
void game_init() {
  // clear screen
  TFT.fillScreen(BCKGRDCOL);
  // reset score
  score = 0;
  // init bird
  bird.x = 20;
  bird.y = bird.old_y = TFTH2 - BIRDH;
  bird.vel_y = -JUMP_FORCE;
  tmpx = tmpy = 0;
  // generate new random seed for the pipe gape
  randomSeed(analogRead(0));
  // init pipe
  pipe.x = TFTW;
  pipe.gap_y = random(20, TFTH-60);
}

// ---------------
// game over
// ---------------
void game_over() {
  TFT.fillScreen(BLACK);
  TFT.setTextColor(WHITE);
  TFT.setTextSize(2);
  // half width - num char * char width in pixels
  TFT.setCursor( TFTW2 - (9*6), TFTH2 - 4);
  TFT.println("GAME OVER");
  TFT.setTextSize(0);
  TFT.setCursor( 10, TFTH2 - 14);
  TFT.print("score: ");
  TFT.print(score);
  TFT.setCursor( TFTW2 - (12*3), TFTH2 + 12);
  TFT.println("press button");
  while (1) {
    // wait for push button
    if ( !(PIND & (select()==0)) ) break;
  }
}
void messageIcon(uint8_t x,uint8_t y,boolean test){
  if(test==true){
    TFT.fillRect(x+2,y+5,20,14,GREEN);
    TFT.fillRect(x+3,y+6,18,12,BLACK);
    TFT.drawLine(x+2,y+5,x+11,y+11,GREEN);
    TFT.drawLine(x+2,y+6,x+11,y+12,GREEN);
    TFT.drawLine(x+12,y+11,x+21,y+5,GREEN);
    TFT.drawLine(x+12,y+12,x+21,y+6,GREEN);
  }else {
    TFT.fillRect(x+2,y+5,20,14,GREY);
    TFT.fillRect(x+3,y+6,18,12,BLACK);
    TFT.drawLine(x+2,y+5,x+11,y+11,GREY);
    TFT.drawLine(x+2,y+6,x+11,y+12,GREY);
    TFT.drawLine(x+12,y+11,x+21,y+5,GREY);
    TFT.drawLine(x+12,y+12,x+21,y+6,GREY);
  }
}
void callIcon(uint8_t x,uint8_t y, boolean test){
if(test==true){
  // designing the phone icon
  TFT.fillRect(x+1,y+2,11,19,WHITE);
  TFT.fillRect(x+2,y+3,9,17,BLACK);
  TFT.fillRect(x+3,y+4,7,13,BLUE);
  TFT.drawLine(x+3,y+18,x+9,y+18,WHITE);
  // designing the arrow 
  TFT.fillRect(x+16,y+14,6,6,RED);
  TFT.fillRect(x+16,y+14,5,5,BLACK);
  TFT.drawLine(x+21,y+4,x+14,y+11,RED);
  TFT.drawLine(x+21,y+5,x+14,y+12,RED);
  TFT.drawLine(x+21,y+6,x+14,y+13,RED);
  TFT.drawLine(x+14,y+11,x+20,y+17,RED);
  TFT.drawLine(x+14,y+12,x+20,y+18,RED);
  TFT.drawLine(x+14,y+13,x+20,y+19,RED);
    }else {
      // designing the phone icon
  TFT.fillRect(x+1,y+2,11,19,GREY);
  TFT.fillRect(x+2,y+3,9,17,BLACK);
  TFT.fillRect(x+3,y+4,7,13,GREY);
  TFT.drawLine(x+3,y+18,x+9,y+18,GREY);
  // designing the arrow 
  TFT.fillRect(x+16,y+14,6,6,GREY);
  TFT.fillRect(x+16,y+14,5,5,BLACK);
  TFT.drawLine(x+21,y+4,x+14,y+11,GREY);
  TFT.drawLine(x+21,y+5,x+14,y+12,GREY);
  TFT.drawLine(x+21,y+6,x+14,y+13,GREY);
  TFT.drawLine(x+14,y+11,x+20,y+17,GREY);
  TFT.drawLine(x+14,y+12,x+20,y+18,GREY);
  TFT.drawLine(x+14,y+13,x+20,y+19,GREY);
    }
}
void bluetoothIcon(uint8_t x,uint8_t y,boolean test){
  if(test==true){
    TFT.fillCircle(x+11, y+11, 12, BLUE);
    
    TFT.drawLine(x+11,y+2,x+11,y+21,WHITE);
    TFT.drawLine(x+12,y+2,x+12,y+21,WHITE);
    TFT.drawLine(x+11,y+2,x+16,y+7,WHITE);
    TFT.drawLine(x+12,y+2,x+17,y+7,WHITE);
    TFT.drawLine(x+16,y+7,x+7,y+16,WHITE);
    TFT.drawLine(x+17,y+7,x+8,y+16,WHITE);
    TFT.drawLine(x+11,y+21,x+16,y+16,WHITE);
    TFT.drawLine(x+12,y+21,x+17,y+16,WHITE);
    TFT.drawLine(x+16,y+16,x+7,y+7,WHITE);
    TFT.drawLine(x+17,y+16,x+8,y+7,WHITE);
  }else {
    TFT.drawCircle(x+11, y+11, 12, GREY);
    
    TFT.drawLine(x+11,y+2,x+11,y+21,GREY);
    TFT.drawLine(x+12,y+2,x+12,y+21,GREY);
    TFT.drawLine(x+11,y+2,x+16,y+7,GREY);
    TFT.drawLine(x+12,y+2,x+17,y+7,GREY);
    TFT.drawLine(x+16,y+7,x+7,y+16,GREY);
    TFT.drawLine(x+17,y+7,x+8,y+16,GREY);
    TFT.drawLine(x+11,y+21,x+16,y+16,GREY);
    TFT.drawLine(x+12,y+21,x+17,y+16,GREY);
    TFT.drawLine(x+16,y+16,x+7,y+7,GREY);
    TFT.drawLine(x+17,y+16,x+8,y+7,GREY);
  }
}
void AlarmIcon(uint8_t x,uint8_t y,boolean test){
  if(test==true){
    TFT.fillCircle(x+12, y+12, 12, YELLOW);
    TFT.fillCircle(x+12, y+12, 10, BLACK);
    TFT.drawLine(x+12,y+12,x+6,y+12,YELLOW);
    TFT.drawLine(x+12,y+12,x+12,y+4,YELLOW);
  }else {
    TFT.fillCircle(x+11, y+11, 12, GREY);
    TFT.fillCircle(x+11, y+11, 10, BLACK);
    TFT.drawLine(x+12,y+12,x+6,y+12,GREY);
    TFT.drawLine(x+12,y+12,x+12,y+4,GREY);
  }
}
