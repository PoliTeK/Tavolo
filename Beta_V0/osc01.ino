/*  CODICE OSC0 E OSC1 STM32
Questo codice implementa i primi due oscillatori Sub Bass e Bass
Per ogni oscillatore è previsto un potenziometro per variare la nota nella scala 
di LA minore e un potenziometro per variare il cutoff del filtro.
La resonance di entrambi i filtri è costante con un valore calibrato ad orecchio.

Distorsione gain clip implementata sulla somma dei due oscillatori

*/


#include <MozziGuts.h>
// oscillator templates
#include <Oscil.h>
#include <tables/saw2048_int8.h> 
#include <tables/square_no_alias_2048_int8.h>
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
#define LEDCLIP_PIN 5 // Led per visualizzare la quantità di distorsione


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
const IntMap CUTOFF_map(0,4096,200,5000);
const IntMap CLIP_map(0,4096,70,255);
const IntMap LEDCLIP_map(50,255,255,0);
const IntMap GAIN_map(70,255,65,20);


//--------------------------------------------Pulsante-------------------------------------------------
bool Switch_RM=false;
unsigned long debounceDelay = 50; // Ritardo di debounce in millisecondi
bool buttonState = false; // Variabile booleana da invertire
bool lastButtonState = false; // Stato precedente del pulsante
unsigned long lastDebounceTime = 0; // Tempo dell'ultimo cambiamento di stato del pulsante


//--------------------------------------------Oscillatori Voci-------------------------------------------
Oscil <SQUARE_NO_ALIAS_2048_NUM_CELLS, AUDIO_RATE> Osc0(SQUARE_NO_ALIAS_2048_DATA); // oscillatore SubBass
Oscil <SAW2048_NUM_CELLS, AUDIO_RATE> Osc1(SAW2048_DATA); // Oscillatore  basso
// Array di oscillatori di voci per poterci iterare sopra
Oscil <2048, AUDIO_RATE> oscillatori[] = {Osc0, Osc1};
int Sum = 0;


//--------------------------------------------Oscillatori Modulazioni-------------------------------------------
Oscil <SIN2048_NUM_CELLS, AUDIO_RATE> Carrier2(SIN2048_DATA); // portante per Ring Modulatin(RM)  Osc1 
Oscil <SIN2048_NUM_CELLS, AUDIO_RATE> Carrier1(SIN2048_DATA);


//---------------------------------------Filtri-------------------------------------------------
ResonantFilter<LOWPASS, uint16_t> lpf1; 
ResonantFilter<LOWPASS, uint16_t> lpf2; 
int cutoff1 = 0;
int cutoff2 = 0;
int resonance=0;


//---------------------------------------Distorsione-------------------------------------------------
byte gain=255;
int clip=255;


//------------------------------------------------------------------------------------------------------------------
void setup() {
  //Serial.begin(9600);
  pinMode(LEDCLIP_PIN,OUTPUT);
  startMozzi(CONTROL_RATE); // :)
  
}

//-------------------------------------------------------------------------------------------------------------------
void updateControl() {

//--------------------------------------------Debouncing--------------------------------------------------------
  int reading = digitalRead(P0_PIN); // Legge lo stato attuale del pulsante
  // If the switch changed, due to noise or pressing:
  if (reading != lastButtonState) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }
  if ((millis() - lastDebounceTime) > debounceDelay) {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:
    // if the button state has changed:
    if (reading != buttonState) {
      buttonState = reading;
      // only toggle the LED if the new button state is HIGH
      if (buttonState == HIGH) {
        Switch_RM = !Switch_RM;
      }
    }
  }
  //Serial.println(Switch_RM);
  // save the reading. Next time through the loop, it'll be the lastButtonState:
  lastButtonState = reading;
  // Pippone finito ;)

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
 //setto le frequenze in base alle letture
  Osc0.setFreq(frequenze_base[0]*2);
  Osc1.setFreq(frequenze_base[1]*4);

//-------------------------------------------------------RM-------------------------------------------------------------------------
  float Carrier2_Freq = MOD_map(GEN_map(mozziAnalogRead(POT5_PIN))); // leggo il potenziometro per determinare la frequenza aggiuntiva
  
  // portante1: è un'ottava sotto il segnale modulato + la frequenza letta dal potenziometro
  // (divido per 8 per avere più precisione)
  Carrier1.setFreq(frequenze_base[1]*48+Carrier1_Freq/8);
  
  // portante2: è alla stessa ottava del segnale modulato + la frequenza letta dal potenziometro
  // (divido per 10 per avere una precisione diversa dall'altra portante)
  Carrier2.setFreq(frequenze_base[0]*24+Carrier2_Freq/10);
  
  // modo simpatico per fare oscillare il led a tempo con la modulazione
  if(Switch_RM==true){  
    if(Carrier1.next()<0){
      analogWrite(LEDM_PIN,0);
    }
    if(Carrier2.next()<0){
      analogWrite(LEDM_PIN,0);
    }
    else analogWrite(LEDM_PIN,Carrier1.next());   
  }
  else analogWrite(LEDM_PIN,0);

//------------------------------------------------------------------------Filtri------------------------------------------------
  cutoff1 = CUTOFF_map(mozziAnalogRead(POT2_PIN));
  cutoff2 = CUTOFF_map(mozziAnalogRead(POT3_PIN));
  resonance = 2000;
  lpf1.setCutoffFreqAndResonance(cutoff1, resonance);
  lpf2.setCutoffFreqAndResonance(cutoff2, resonance);
  //Serial.println(cutoff1);
  //Serial.println(cutoff2);

//------------------------------------------------------------------------Distorsione------------------------------------------------
  clip = CLIP_map(mozziAnalogRead(POT5_PIN));
  analogWrite(LEDCLIP_PIN,LEDCLIP_map(clip)); // Da quello che ho capito analogWrite non si può usare
  gain = GAIN_map(clip);


}



AudioOutput_t updateAudio() {
  if (Switch_RM==true){
    Sum = ( ((lpf1.next((Carrier2.next()>>9)*Osc0.next()))*4) + ((lpf2.next(((Carrier1.next()>>9))*Osc1.next())*3 )) )>>5;
    // moltiplico il segnale per le 2 portanti divise (di molto) secondo il gusto per rendere la modulazione il giusto invasiva
  }
  else{
}
  return MonoOutput::fromAlmostNBit(8,Sum);
}

void loop() {
  audioHook(); // required here
}
