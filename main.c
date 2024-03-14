
#include "MSP430F2132.h"

//Port1
#define LATCH_595 BIT0
#define CLOCK_595_1 BIT1
#define DATA_595_1 BIT2
#define CLOCK_595_2 BIT4
#define DATA_595_2 BIT5
#define B_13_139 BIT6
#define A_13_139 BIT7
//Port2
#define B_24_139 BIT0
#define A_24_139 BIT1
#define EnableLED BIT2

int MODE;
int SWITCH_MODE = 0;
int MAX_TIME = 15;
int DO_CHENH_LECH = 0;
const int TIME_YELLOW = 3;
const int RED = 0;
const int YELLOW = 1;
const int GREEN = 2;
const int SLOW_MODE = 997;
const int NORMAL_MODE = 998;
const int ONLY_YELLOW_MODE = 999;

typedef struct {
  int type;
  int time;
} TrafficLight;

TrafficLight HUONG_13;
TrafficLight HUONG_24;

void LACTH_DATA()
{
  P1OUT &= ~LATCH_595;
  P1OUT |= LATCH_595;
}
void shiftOut(int IC, unsigned char hex)
{
  unsigned char ICs_Clock[] = {CLOCK_595_1, CLOCK_595_2};
  unsigned char ICs_Data[] = {DATA_595_1, DATA_595_2};
  for (int i = 0; i < 4; i++)
  {
    if ((hex & (1 << i)))
      P1OUT |= ICs_Data[IC];
    else 
      P1OUT &= ~ICs_Data[IC];
    P1OUT |= ICs_Clock[IC];
    P1OUT &= ~ICs_Clock[IC];
  }
}
void init()
{
   MODE = NORMAL_MODE; 
   HUONG_13.time = MAX_TIME;
   HUONG_13.type = GREEN;
   HUONG_24.time = MAX_TIME + (DO_CHENH_LECH < TIME_YELLOW ? TIME_YELLOW : DO_CHENH_LECH);
   HUONG_24.type = RED;
}
void renderLED()
{
  if (HUONG_13.type == RED)
  {
    P1OUT &= ~B_13_139; 
    P1OUT &= ~A_13_139; //00
  } else if (HUONG_13.type == YELLOW)
  {
    P1OUT &= ~B_13_139;
    P1OUT |= A_13_139; //01
  } else if (HUONG_13.type == GREEN)
  {
    P1OUT |= B_13_139;
    P1OUT &= ~A_13_139; //10
  } 
  
  if (HUONG_24.type == RED)
  {
    P2OUT &= ~B_24_139; 
    P2OUT &= ~A_24_139; //00
  } else if (HUONG_24.type == YELLOW)
  {
    P2OUT &= ~B_24_139;
    P2OUT |= A_24_139; //01
  } else if (HUONG_24.type == GREEN)
  {
    P2OUT |= B_24_139;
    P2OUT &= ~A_24_139; //10
  } 
}
void SET_HUONG_13()
{
  HUONG_13.time--;
  if (HUONG_13.time == 0)
  {
    if (HUONG_13.type == RED)
    {
      HUONG_13.time = MAX_TIME;
      HUONG_13.type = GREEN;
    } else if (HUONG_13.type == YELLOW)
    {
      HUONG_13.time = MAX_TIME + TIME_YELLOW + DO_CHENH_LECH;
      HUONG_13.type = RED;
    } else if (HUONG_13.type == GREEN)
    {
      HUONG_13.time = TIME_YELLOW;
      HUONG_13.type = YELLOW;
    } 
  }
}
void SET_HUONG_24()
{
  HUONG_24.time--;
  if (HUONG_24.time == 0)
  {
    if (HUONG_24.type == RED)
    {
      HUONG_24.time = MAX_TIME;
      HUONG_24.type = GREEN;
    } else if (HUONG_24.type == YELLOW)
    {
      HUONG_24.time = MAX_TIME + TIME_YELLOW + DO_CHENH_LECH;
      HUONG_24.type = RED;
    } else if (HUONG_24.type == GREEN)
    {
      HUONG_24.time = TIME_YELLOW;
      HUONG_24.type = YELLOW;
    } 
  }
}
void renderLED7Seg()
{
  int hang_don_vi_13 = HUONG_13.time % 10;
  int hang_chuc_13 = (HUONG_13.time - hang_don_vi_13) / 10;
  
  shiftOut(0, hang_don_vi_13);
  shiftOut(0, hang_chuc_13);

  int hang_don_vi_24 = HUONG_24.time % 10;
  int hang_chuc_24 = (HUONG_24.time - hang_don_vi_24) / 10;
  
  shiftOut(1, hang_don_vi_24);
  shiftOut(1, hang_chuc_24);
  LACTH_DATA();
}
void setEnableLED(int isEnable) {
  if (isEnable)
  {
    P2OUT |= EnableLED;
  } else P2OUT &= ~EnableLED;
}
void main( void )
{
  // Stop watchdog timer to prevent time out reset
  WDTCTL = WDTPW + WDTHOLD;
  
  P1DIR = 0xF7;
  P2DIR = 0xFF;
  P3DIR = 0xFF;
  
  //Set P1.3 la chan ngat
  P1IE = BIT3; 
  P1IES = BIT3;
  P1IFG &= ~BIT3;
  _BIS_SR(GIE);
  //khoi tao gia tri mac dinh
  init();
  
  while(1)
  {  
    if (HUONG_13.time == 1 && HUONG_24.time == 1 && SWITCH_MODE != 0)
    {
      int modes[3] = { NORMAL_MODE, SLOW_MODE, ONLY_YELLOW_MODE };
      int current_index_mode = 0;
      for (int i = 0 ; i < 3; i ++)
      {
        if (MODE == modes[i])
        {current_index_mode = i;}
      }
      MODE = modes[(current_index_mode + SWITCH_MODE) % 3];
      SWITCH_MODE = 0;
    }
    if (MODE == NORMAL_MODE || MODE == SLOW_MODE)
    {
      DO_CHENH_LECH = MODE == SLOW_MODE ? 10 : 0;
      setEnableLED(1);
      SET_HUONG_13();
      SET_HUONG_24();
      renderLED7Seg();
      renderLED();
    } 
    else if(MODE == ONLY_YELLOW_MODE)
    {
      setEnableLED(0);
      //BAT DEN VANG CHO HUONG 1-3
      P1OUT &= ~B_13_139;
      P1OUT |= A_13_139;

      //BAT DEN VANG CHO HUONG 2-4
      P2OUT &= ~B_24_139;
      P2OUT |= A_24_139;
      __delay_cycles(500000);

      //TAT DEN VANG CHO HUONG 1-3
      P1OUT |= B_13_139;
      P1OUT |= A_13_139;

      //TAT DEN VANG CHO HUONG 2-4
      P2OUT |= B_24_139;
      P2OUT |= A_24_139;

    }
    __delay_cycles(500000);
  }
}
#pragma vector = PORT1_VECTOR
__interrupt void Button_Click(void)
{
  if (P1IFG & BIT3)
  {
    SWITCH_MODE = SWITCH_MODE == 3 ? 0 : (SWITCH_MODE + 1);
    P1IFG &= ~BIT3;
  }
}