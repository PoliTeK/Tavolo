#include <MozziGuts.h>
// oscillator templates
#include <Oscil.h>
#include <tables/saw2048_int8.h> 
#include <tables/triangle_hermes_2048_int8.h>
#include <tables/square_no_alias_2048_int8.h>
// filtro
#include <ResonantFilter.h>

#include <tables/sin2048_int8.h> 
// libreria per map veloce
#include <IntMap.h> 

#include <EventDelay.h> // libreria ritardi di mozzi 
// libreria Random function
#include <mozzi_rand.h>



#define AUDIO_RATE 44100 // Hz   
// #define AUDIO_RATE 16384 // Hz

// Definiamo i pin a cui colleghiamo potenziometri 
#define POT0_PIN A0 
#define POT1_PIN A1
#define POT2_PIN A2 
#define POT3_PIN A3
#define POT4_PIN A4 
#define POT5_PIN A5


// Definizione note associate a frequenze
#define La0 27.5
#define Si0 30.87
#define Do1 32.7
#define Re1 36.71
#define Mi1 41.2
#define Fa1 43.65
#define Sol1 49
#define La1 La0*2


// funzioni per il map
const IntMap GEN_map(0,4096,0,50);   // Per STM32 con potenziometri collegati a 3.3V
const IntMap NOTE_map(0,45,0,7);   
const IntMap CUTOFF_map(20,4096,500,50000);

 


//--------------------------------------------Oscillatori Voci-------------------------------------------
Oscil <SQUARE_NO_ALIAS_2048_NUM_CELLS, AUDIO_RATE> Osc0(SQUARE_NO_ALIAS_2048_DATA); // oscillatore SubBass
Oscil <SAW2048_NUM_CELLS, AUDIO_RATE> Osc1(SAW2048_DATA); // Oscillatore  basso


//---------------------------------------Filtri-------------------------------------------------
ResonantFilter<LOWPASS, uint16_t> lpf1; 
ResonantFilter<LOWPASS, uint16_t> lpf2; 
int cutoff1 = 0;
int cutoff2 = 0;
int resonance=0;


//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void setup() {
  Serial.begin(9600);
  startMozzi(); // :)
  
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void updateControl() { 

//----------------------------------------Note Voci----------------------------------------------------------- 
  // Vettore con tutti i valori letti dai potenziometri
  int letture[5] = { GEN_map(mozziAnalogRead(POT0_PIN)), GEN_map(mozziAnalogRead(POT1_PIN))};
  int valori_discreti[2];    // Variabili per il calcolo delle note e della frequenza dei vari oscillatori
  float frequenze_base[2];
  // Discretizzazione delle frequenze iterate per ogni oscillatore 
  for (int i = 0; i < 2; i++) 
  {
    valori_discreti[i] = NOTE_map(letture[i]);
    switch(valori_discreti[i]) {
      case 0:
        frequenze_base[i] = La0;
        break;
      case 1:
        frequenze_base[i] = Si0;
        break;
      case 2:
        frequenze_base[i] = Do1;
        break;
      case 3:
        frequenze_base[i] = Re1;
        break;
      case 4:
        frequenze_base[i] = Mi1;
        break;
      case 5:
        frequenze_base[i] = Fa1;
        break;
      case 6:
        frequenze_base[i] = Sol1;
        break;
      case 7:
        frequenze_base[i] = La1;
        break;
    }
  }
// setto le frequenze in base alle letture
  Osc0.setFreq(frequenze_base[0]*2);
  Osc1.setFreq(frequenze_base[1]*4);

//------------------------------------------------------------------------Filtri------------------------------------------------
  cutoff1 = CUTOFF_map(mozziAnalogRead(POT2_PIN));
  cutoff2 = CUTOFF_map(mozziAnalogRead(POT3_PIN));
  resonance = 200;
  lpf1.setCutoffFreqAndResonance(cutoff1, resonance);
  lpf2.setCutoffFreqAndResonance(cutoff2, resonance);
  //Serial.println(cutoff1);
  //Serial.println(cutoff2);


}

int Sum = 0;


AudioOutput_t updateAudio() {
  Sum = (lpf1.next(Osc0.next())*2 + lpf2.next(Osc1.next())*4 )>>3;
  return MonoOutput::from8Bit(Sum);
}

void loop() {
  audioHook(); // required here
}

