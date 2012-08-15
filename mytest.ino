#include <ADK.h>

#define BTN_SLIDER_0   0x00
#define BTN_SLIDER_1   0x01
#define BTN_SLIDER_2   0x02

#define BTN_MASK_PRESS     0x8000
#define BTN_MASK_HOLD      0x4000
#define BTN_MASK_RELEASE   0x2000
#define BTN_MASK_ID        0x003F

#define BTN_INITIAL_DELAY  10  //10ms before a click registers
#define BTN_HOLD_DELAY     400 //400mas before a click becomes a hold
#define BTN_AUTOREPEAT     100 //auto-repeat every 100ms

ADK L;
static uint8_t ledrgb[6][3]={
  0,0,255,
  255,0,0,
  255,255,0,
  0,0,255,
  0,255,0,
  255,0,0 
};
String string[3]={
  "Google","is","GOD"};

static int slider=100;

static uint16_t btnProcess(){

  static uint64_t lastActionTime[32] = {
    0,     }; 
  static uint32_t clickSent = 0;
  static uint32_t holdSent = 0;
  static uint32_t lastStates = 0;
  uint32_t curState, t, i, mask;

  curState = (((uint32_t)L.capSenseButtons()) << 16) | L.capSenseIcons();
  t = lastStates ^ curState;
  lastStates = curState;

  //update states for all buttons
  for(mask = 1, i = 0; i < 32; i++, mask <<= 1) if(t & mask){
    lastActionTime[i] = L.getUptime();
  }

  for(mask = 1, i = 0; i < 32; i++, mask <<= 1){
    if(curState & mask){
      uint64_t time = L.getUptime();
      uint64_t lapsed = time - lastActionTime[i];
      if(holdSent & mask){  
        if(lapsed > BTN_AUTOREPEAT){
          lastActionTime[i] = time;
          return i | BTN_MASK_HOLD;
        }
      }
      else if(clickSent & mask){  
        if(lapsed > BTN_HOLD_DELAY){
          holdSent |= mask;
          lastActionTime[i] = time;
          return i | BTN_MASK_HOLD;
        }
      }
      else{  //maybe time to click
        if(lapsed > BTN_INITIAL_DELAY){
          clickSent |= mask;
          lastActionTime[i] = time;
          return i | BTN_MASK_PRESS;
        }
      }
    }
    else if(clickSent & mask){  //release
      clickSent &=~ mask;
      holdSent &=~ mask;
      return i | BTN_MASK_RELEASE;
    }
  }
  return 0;
}

void adkPutchar(char c){
  Serial.write(c);
}

extern "C" void dbgPrintf(const char *, ... );

void setup(void)
{
  Serial.begin(115200);
  L.adkSetPutchar(adkPutchar);
  L.adkInit();
}

void loop(){

  uint8_t slider = L.capSenseSlider();
  slider = (((uint32_t)slider) >> 3) + 16;
  uint16_t button = btnProcess();
   
   dbgPrintf("ADK: Value of Slider:%d\n",slider);
    
    L.adkEventProcess();
   
    for(int j=0;j<3;j++){
      for(int i=0;i<6;i++){

        L.ledDrawLetter(i,string[j].charAt(i),ledrgb[i][0],ledrgb[i][1],ledrgb[i][2]);
        delay(slider);

      }
      delay(slider);
      for(int x=0;x<6;x++){
        L.ledDrawLetter(x,' ',0,0, 0);
        delay(slider);
      }
    }
  

}







