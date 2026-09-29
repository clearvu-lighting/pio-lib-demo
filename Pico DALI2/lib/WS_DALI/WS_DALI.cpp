#include "WS_DALI.h"

class DaliMaster : public Dali
{
  protected:

		gpio_num_t rx_pin, tx_pin;

	public:

		DaliMaster( gpio_num_t rx_pin, gpio_num_t tx_pin );

  protected:

  /*
    virtual uint8_t bus_is_high(void) override; //returns !=0 if DALI bus is in high (non-asserted) state
    virtual void bus_set_low(void) override; //set DALI bus in low (asserted) state
    virtual void bus_set_high(void) override; //set DALI bus in high (released) state
  */
};

DaliMaster::DaliMaster( gpio_num_t rx_pin_, gpio_num_t tx_pin_ )
  : rx_pin( rx_pin_ )
  , tx_pin( tx_pin_ )
{
}

DaliMaster dali( (gpio_num_t)RX_PIN, (gpio_num_t)TX_PIN );
uint8_t DALI_Addr[64] = {0};
uint8_t DALI_NUM = 0;

uint8_t bus_is_high(void)
{
  return digitalRead(RX_PIN); //slow version
}

//use bus
void bus_set_low() {
  digitalWrite(TX_PIN,LOW); //opto slow version
}

//release bus
void bus_set_high() {
  digitalWrite(TX_PIN,HIGH); //opto slow version
}

void ARDUINO_ISR_ATTR onTimer() {
  dali.timer();
}

void DALI_Init() {
  printf("This is Wavshare's DALI board \r\n");
  //setup RX/TX pin
  pinMode(RX_PIN, INPUT);
  pinMode(TX_PIN, OUTPUT);
  dali.begin( bus_is_high, bus_set_high, bus_set_low );
  InitTimer();
  Scan_DALI_addr_ALL();
  if(DALI_NUM == 0)
    Assign_new_address_ALL();
}

#if ESP_ARDUINO_VERSION_MAJOR > 2
void InitTimer(void)
{
	/*
	 * If new Arduino environment, use this...
	 */
	hw_timer_t* hw_timer = timerBegin(9600000);
	timerAttachInterrupt( hw_timer, &onTimer );
	timerAlarm( hw_timer, 1000, true, 0 );
}
#else
void InitTimer(void)
{
	/*
	 * If the older Arduino environment, this is the equivalent...
	 */
	printf( "Setting up the timers using Arduino V2 method.\r\n" );

	hw_timer_t* hw_timer = timerBegin( 0, 80, true );   // Timer 1; 80 MHz / 8 = 10 MHz
	timerAttachInterrupt( hw_timer, &onTimer, false );  // Note Edge-triggered ISR
	timerAlarmWrite( hw_timer, 104, true );             // 104.2 us, repeating
	
	printf( "Enable the timer now.\r\n" );
	timerAlarmEnable( hw_timer );
	printf( "Timer enabled.\r\n" );
}
#endif

void Blinking_ALL() {                                                        
  printf("Running: Blinking all lamps\r\n");
  dali.set_level(254);
  delay(500);
  dali.set_level(0);
  delay(500);
}
void Luminaire_Brightness(uint8_t Light, uint8_t addr) {                     
  // printf("Running: Set the brightness of the fixture at address %d to %d %%\r\n", addr, Light);
  uint8_t Light_practical = (uint8_t)(2.55*Light);
  dali.set_level(Light_practical,addr);
}

void Lighten_ALL() {                                                          
  printf("Running: Turn on all lights on the DALI\r\n");
  dali.set_level(200);
}
void Extinguish_ALL() {                                                      
  printf("Running: Turn off all lights on the DALI\r\n");
  dali.set_level(0);
}

const char *DALI_ERR_TEXT[]
{
  "DALI_ERROR_UNKNOWN",
  "DALI_RESULT_NO_REPLY",      // 101 cmd() did not receive a reply (i.e. received a 'NO' Backward Frame)
  "DALI_RESULT_TIMEOUT",       // 102 Timeout waiting for DALI bus
  "DALI_RESULT_DATA_TOO_LONG", // 103 Trying to send too many bytes (max 3)
  "DALI_RESULT_INVALID_CMD",   // 104 The cmd argument in the call to cmd() was invalid
  "DALI_RESULT_INVALID_REPLY"  // 105 cmd() received an invalid reply (not 8 bits)
};

void Scan_DALI_addr_ALL() {                                                  
  printf("Running: Scan all addresses\r\n");
  uint8_t addr;
  for(addr = 0; addr<64; addr++) {
    int16_t rv = dali.cmd(DALI_QUERY_STATUS,addr);
    if(rv>=0) {
      DALI_Addr[DALI_NUM] = addr;
      DALI_NUM ++;
      printf("Address %d  status=0x%x  minLevel= %d \r\n", addr, rv, dali.cmd(DALI_QUERY_MIN_LEVEL,addr));
      dali.set_level(254,addr);
      delay(500);
      dali.set_level(0,addr);
    }
    else if ( -rv != DALI_RESULT_NO_REPLY ) 
    {
      int i = -rv;
      if( ( i >= DALI_RESULT_NO_REPLY ) && ( i <= DALI_RESULT_INVALID_REPLY ) ) i = i - DALI_RESULT_NO_REPLY + 1;
      else i = 0;
      printf("short address= %d ERROR= %d: %s\r\n", addr,-rv, DALI_ERR_TEXT[i] );
    }
  }  
  printf("End scan,%d devices were scanned\r\n",DALI_NUM);
}
void Delete_DALI_addr_ALL() {                                                           
  printf("Running: Delete all short addresses\r\n");
  //remove all short addresses
  dali.cmd(DALI_DATA_TRANSFER_REGISTER0,0xFF);
  dali.cmd(DALI_SET_SHORT_ADDRESS, 0xFF);
  printf("DONE delete \r\n");
}

void Assign_new_address_ALL(){                                                  
  printf("Running: Assign new addresses to all devices\r\n");   
  printf("Might need a couple of runs to find all lamps ...\r\n");
  printf("Be patient, this takes a while ...\r\n");
  uint8_t cnt = dali.commission(0xff); //init_arg=0b11111111 : all without short address  
  printf("DONE, assigned %d new short addresses\r\n",cnt);
  Scan_DALI_addr_ALL();
}

// 设置亮度的函数（需要实现具体逻辑）
void setBrightness(int sliderId, int value) {
    // 这里根据 sliderId 设置亮度
    Luminaire_Brightness(value, DALI_Addr[sliderId-1]);
    //printf("Slider %d value: %d\n", sliderId, value);
}
