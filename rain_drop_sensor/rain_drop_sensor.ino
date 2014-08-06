#include <avr/sleep.h>
#include <avr/wdt.h>
//#include <toneAC.h>

#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

int cpt = 0;           // variable to store the value read
uint8_t nb_melody = 5;
uint16_t pause = 5000;
unsigned long get_time1 = millis();
volatile boolean f_wdt=1;

/* Coefficient diviseur du pont de résistance */
const float coeff_division = 2.0;

void setup()
{
  //Serial.begin(9600);          //  setup serial
  
  cbi( SMCR,SE );      // sleep enable, power down mode
  cbi( SMCR,SM0 );     // power down mode
  sbi( SMCR,SM1 );     // power down mode
  cbi( SMCR,SM2 );     // power down mode
  
  setup_watchdog(6);
}

void playConfirmMelody()
{
  for (int aa=900;aa<=2200;aa=aa+50){    
    playTone(aa);
  }
  for (int aa=2200;aa>=900;aa=aa-50){    
    playTone(aa);
  }
  for (int aa=900;aa<=2200;aa=aa+50){    
    playTone(aa);
  }
}

void playTone(int aa)
{
    tone(8, aa, 6);
    delay(7);
    noTone(8);
}

void loop()
{
  if (f_wdt==1) {  // wait for timed out watchdog / flag is set when a watchdog timeout occurs
  
    f_wdt=0;       // reset flag
   
    /* Mesure de la tension brute */
    unsigned int raw = analogRead(A0) * coeff_division;
    
    if (raw > 100 && cpt < nb_melody) {
      playConfirmMelody();
      
      cpt++;
      if (cpt == nb_melody)
        get_time1 = millis();
    }
    
    if (cpt >= nb_melody &&  millis() >= (get_time1 + pause)){
      cpt = 0;
    } 
    
    //Serial.println(raw);  
    //delay(2); // wait until the last serial character is send
    
    system_sleep();
    
  }
}

//****************************************************************  
// set system into the sleep state 
// system wakes up when wtchdog is timed out
void system_sleep() {

  cbi(ADCSRA,ADEN);                    // switch Analog to Digitalconverter OFF

  set_sleep_mode(SLEEP_MODE_IDLE); // sleep mode is set here
  sleep_enable();

  sleep_mode();                        // System sleeps here

  sleep_disable();                     // System continues execution here when watchdog timed out 
  sbi(ADCSRA,ADEN);                    // switch Analog to Digitalconverter ON
}

//****************************************************************
// 0=16ms, 1=32ms,2=64ms,3=128ms,4=250ms,5=500ms
// 6=1 sec,7=2 sec, 8=4 sec, 9= 8sec
void setup_watchdog(int ii) {

  byte bb;
  int ww;
  if (ii > 9 ) ii=9;
  bb=ii & 7;
  if (ii > 7) bb|= (1<<5);
  bb|= (1<<WDCE);
  ww=bb;
  //Serial.println(ww);

  MCUSR &= ~(1<<WDRF);
  // start timed sequence
  WDTCSR |= (1<<WDCE) | (1<<WDE);
  // set new watchdog timeout value
  WDTCSR = bb;
  WDTCSR |= _BV(WDIE);
}

//****************************************************************  
// Watchdog Interrupt Service / is executed when  watchdog timed out
ISR(WDT_vect) {
  f_wdt=1;  // set global flag
}


