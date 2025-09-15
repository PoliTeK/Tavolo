
/*  CODICE OSC2 E OSC3 STM32
Questo codice implementa i primi due oscillatori medio basso e medio alto
Per ogni oscillatore è previsto un potenziometro per variare la nota nella scala 
di LA minore e un potenziometro per variare il cutoff del filtro.
La resonance di entrambi i filtri è costante con un valore calibrato ad orecchio.
Inoltre è implementato un controllo della ring modulation attivabile con un pulsante,
con un led pilotato in PWM che va a tempo con la modulazione.


*/


#include <MozziGuts.h>
// oscillator templates
#include <Oscil.h>
#include <tables/saw2048_int8.h> 
#include <tables/triangle_hermes_2048_int8.h>
#include <tables/square_no_alias_2048_int8.h>
#include <tables/sin2048_int8.h> 
// filtro
#include <ResonantFilter.h>
// libreria per map veloce
#include <IntMap.h> 

#include <EventDelay.h> // libreria ritardi di mozzi 

// libreria Random function
#include <mozzi_rand.h>



#define AUDIO_RATE 44100 // Hz   
// #define AUDIO_RATE 16384 // Hz
#define CONTROL_RATE 128 // Hz

// Definiamo i pin a cui colleghiamo potenziometri 
#define POT0_PIN A0 
#define POT1_PIN A1
#define POT2_PIN A2 
#define POT3_PIN A3
#define POT4_PIN A4 
#define POT5_PIN A5
#define P2_PIN 2      // Pulsante per attivare o disattivare la modulazione
#define P3_PIN 4      // Pulsante per attivare o disattivare la modulazione
#define LED_PIN 5    // Led per visualizzare la freq di modulazione


// Definizione note associate a frequenze
#define La0 27.5
#define Si0 30.87
#define Do1 32.7
#define Re1 36.71
#define Mi1 41.2
#define Fa1 43.65
#define Sol1 49
#define La1 La0*2
// vettore con le note della scala
const float note[8] = {La0, Si0, Do1, Re1, Mi1, Fa1, Sol1, La1};


// funzioni per il map
const IntMap GEN_map(0,4096,0,50);   // Per STM32 con potenziometri collegati a 3.3V
const IntMap NOTE_map(0,45,0,7);   
const IntMap MOD_map(0,45,0,50);
const IntMap CUTOFF_map(0,4000,200,40000);


//--------------------------------------------Pulsante-------------------------------------------------
bool Switch_RM3=false;
bool Switch_RM2=false;
unsigned long debounceDelay = 50; // Ritardo di debounce in millisecondi
bool buttonState3 = false; // Variabile booleana da invertire
bool buttonState2 = false; // Variabile booleana da invertire
bool lastButtonState3 = false; // Stato precedente del pulsante
bool lastButtonState2 = false; // Stato precedente del pulsante
unsigned long lastDebounceTime3 = 0; // Tempo dell'ultimo cambiamento di stato del pulsante  
unsigned long lastDebounceTime2 = 0; // Tempo dell'ultimo cambiamento di stato del pulsante  



//--------------------------------------------Oscillatori Voci-------------------------------------------
Oscil <SAW2048_NUM_CELLS, AUDIO_RATE> Osc3(SAW2048_DATA); // oscillatore medio basso
//Oscil <TRIANGLE_HERMES_2048_NUM_CELLS, AUDIO_RATE> Osc3(TRIANGLE_HERMES_2048_DATA); // oscillatore medio alto
Oscil <SQUARE_NO_ALIAS_2048_NUM_CELLS, AUDIO_RATE> Osc2(SQUARE_NO_ALIAS_2048_DATA);
int Sum = 0;
int ON2 = 0;
int ON3 = 0;

// Array di oscillatori di voci per poterci iterare sopra
Oscil <2048, AUDIO_RATE> oscillatori[] = {Osc2, Osc3};

//--------------------------------------------Oscillatori Modulazioni-------------------------------------------
Oscil <SIN2048_NUM_CELLS, AUDIO_RATE> Carrier2(SIN2048_DATA); // portante per Ring Modulatin(RM)  Osc1 
Oscil <SIN2048_NUM_CELLS, AUDIO_RATE> Carrier3(SIN2048_DATA);


//---------------------------------------Filtri-------------------------------------------------
ResonantFilter<LOWPASS, uint16_t> lpf2; 
ResonantFilter<LOWPASS, uint16_t> lpf3; 
int cutoff2 = 0;
int cutoff3 = 0;
int resonance=0;







//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void setup() {
  Serial.begin(9600);
  pinMode(P3_PIN,INPUT);
  pinMode(P2_PIN,INPUT);
  pinMode(LED_PIN,OUTPUT);

  startMozzi(CONTROL_RATE); // :)
  
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void updateControl() { 

//--------------------------------------------Debouncing--------------------------------------------------------
  int reading3 = digitalRead(P3_PIN);
  int reading2 = digitalRead(P2_PIN);
  
  // If the switch changed, due to noise or pressing:
  if (reading3 != lastButtonState3) {
    // reset the debouncing timer
    lastDebounceTime3 = millis();
  }
  if ((millis() - lastDebounceTime3) > debounceDelay) {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:
    // if the button state has changed:
    if (reading3 != buttonState3) {
      buttonState3 = reading3;
      // only toggle the LED if the new button state is HIGH
      if (buttonState3 == HIGH) {
        Switch_RM3 = !Switch_RM3;
      }
    }
  }
  //Serial.println(Switch_RM);
  // save the reading. Next time through the loop, it'll be the lastButtonState:
  lastButtonState3 = reading3;

  // If the switch changed, due to noise or pressing:
  if (reading2 != lastButtonState2) {
    // reset the debouncing timer
    lastDebounceTime2 = millis();
  }
  if ((millis() - lastDebounceTime2) > debounceDelay) {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:
    // if the button state has changed:
    if (reading2 != buttonState2) {
      buttonState2 = reading2;
      // only toggle the LED if the new button state is HIGH
      if (buttonState2 == HIGH) {
        Switch_RM2 = !Switch_RM2;
      }
    }
  }
  //Serial.println(Switch_RM);
  // save the reading. Next time through the loop, it'll be the lastButtonState:
  lastButtonState2 = reading2;

//----------------------------------------Note Voci----------------------------------------------------------- 
  // Vettore con tutti i valori letti dai potenziometri
  int letture[2] = { GEN_map(mozziAnalogRead(POT0_PIN)), GEN_map(mozziAnalogRead(POT1_PIN))};
  int valori_discreti[2];    // Variabili per il calcolo delle note e della frequenza dei vari oscillatori
  float frequenze_base[2];
  // Discretizzazione delle frequenze iterate per ogni oscillatore 
  for (int i = 0; i < 2; i++) 
  {
    valori_discreti[i] = NOTE_map(letture[i]);
    frequenze_base[i] = note[valori_discreti[i]];
  }
// setto le frequenze in base alle letture
  Osc2.setFreq(frequenze_base[0]*8);  
  Osc3.setFreq(frequenze_base[1]*16);

//-------------------------------------------------------RM-------------------------------------------------------------------------
  float Carrier2_Freq = MOD_map(GEN_map(mozziAnalogRead(POT4_PIN)));
  float Carrier3_Freq = MOD_map(GEN_map(mozziAnalogRead(POT5_PIN)));// leggo il potenziometro per determinare la frequenza aggiuntiva
  Carrier2.setFreq(frequenze_base[0]*48+Carrier2_Freq/8); // portante1: è un'ottava sotto il segnale modulato + la frequenza letta dal potenziometro(divido per 8 per avere più precisione)
  Carrier3.setFreq(frequenze_base[1]*24+Carrier3_Freq/10); // portante2: è alla stessa ottava del segnale modulato + la frequenza letta dal potenziometro(divido per 10 per avere una precisione diversa dall'altra portante)
//  modo simpatico per fare oscillare il led a tempo con la modulazione
  if(Switch_RM3==true || Switch_RM2==true){  
     analogWrite(LED_PIN,abs(Carrier2.next()));   
  }
  else analogWrite(LED_PIN,0);

  
  

//------------------------------------------------------------------------Filtri------------------------------------------------
  cutoff2 = CUTOFF_map(mozziAnalogRead(POT2_PIN));
  cutoff3 = CUTOFF_map(mozziAnalogRead(POT3_PIN));
  if (cutoff2 < 5000) ON2 = 0;
  else if (cutoff2 > 15000) ON2=1;
  if (cutoff3 < 5000) ON3 = 0;
  else if (cutoff3 > 15000) ON3=1;
  resonance = 1000;
  lpf2.setCutoffFreqAndResonance(cutoff2, resonance);
  lpf3.setCutoffFreqAndResonance(cutoff3, resonance);
  Serial.println(cutoff2);
  //Serial.println(cutoff2);

}




AudioOutput_t updateAudio() {
  Sum=0;
  if (Switch_RM3==true ){
    Sum +=(((lpf3.next(((Carrier3.next()>>9))*Osc3.next())*4 )))*ON3 ;  // ( ((lpf2.next((Carrier2.next()>>9)*Osc2.next()))*4) + 
  }
  else{
    Sum +=(lpf3.next(Osc3.next())*4)*ON3 ; // (lpf2.next(Osc2.next())*4 + 
  }
  if (Switch_RM2==true ){
    Sum +=(((lpf2.next((Carrier2.next()>>9)*Osc2.next()))*3))*ON2;  // ( ((lpf2.next((Carrier2.next()>>9)*Osc2.next()))*4) + 
  }
  else{
    Sum +=(lpf2.next(Osc2.next())*3)*ON2 ; // (lpf2.next(Osc2.next())*4 + 
  }
  return MonoOutput::fromNBit(13,Sum);
}

void loop() {
  audioHook(); // required here
}
