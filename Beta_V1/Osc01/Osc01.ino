/*  CODICE OSC0 E OSC1 STM32
Questo codice implementa i primi due oscillatori Sub Bass e Bass
Per ogni oscillatore è previsto un potenziometro per variare la nota nella scala 
di LA minore e un potenziometro per variare il cutoff del filtro.
La resonance di entrambi i filtri è costante con un valore calibrato ad orecchio.

Distorsione gain clip implementata sulla somma dei due oscillatori

(FIXATO) ATTENZIONE! SENORE DISTANZA PER FILTRO OSC1 ESCLUSO VIA SOFTWARE PERCHè DAVA PROBLEMI

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
const IntMap CUTOFF_map(30,4000,200,5000);
const IntMap A3_map(1000,4000,50,3900);
const IntMap CUT3_map(500,4500,200,5000);
const IntMap CLIP_map(3900,30,255,70);
const IntMap LEDCLIP_map(200,60,0,40);
const IntMap GAIN_map(255,70,20,35);

 


//--------------------------------------------Oscillatori Voci-------------------------------------------
Oscil <SQUARE_NO_ALIAS_2048_NUM_CELLS, AUDIO_RATE> Osc0(SQUARE_NO_ALIAS_2048_DATA); // oscillatore SubBass
Oscil <SAW2048_NUM_CELLS, AUDIO_RATE> Osc1(SAW2048_DATA); // Oscillatore  basso
// Array di oscillatori di voci per poterci iterare sopra
Oscil <2048, AUDIO_RATE> oscillatori[] = {Osc0, Osc1};
int Sum = 0;
int ON0 = 0;
int ON1 = 0;



//---------------------------------------Filtri-------------------------------------------------
ResonantFilter<LOWPASS, uint16_t> lpf0; 
ResonantFilter<LOWPASS, uint16_t> lpf1; 
int cutoff0 = 0;
int cutoff1 = 0;
int resonance=0;

//---------------------------------------Distorsione-------------------------------------------------
byte gain=255;
int clip=255;


//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void setup() {
  //Serial.begin(9600);
  pinMode(LEDCLIP_PIN,OUTPUT);
  startMozzi(CONTROL_RATE); // :)
  
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void updateControl() { 

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
  Osc0.setFreq(frequenze_base[0]*4);
  Osc1.setFreq(frequenze_base[1]*8);

//------------------------------------------------------------------------Filtri------------------------------------------------
  cutoff0 = CUTOFF_map(mozziAnalogRead(POT2_PIN));
  cutoff1 = CUTOFF_map(mozziAnalogRead(POT3_PIN));
  if (cutoff0 < 1000) ON0 = 0;
  else if (cutoff0 > 1200) ON0=1;
  if (cutoff1 < 1000) ON1 = 0;
  else if (cutoff1 > 1200) ON1=1;
  resonance = 2000;
  lpf0.setCutoffFreqAndResonance(cutoff0, resonance);
  lpf1.setCutoffFreqAndResonance(cutoff1, resonance);
  //Serial.println(CUT3_map(cutoff1));
  //Serial.println(cutoff1);

//------------------------------------------------------------------------Distorsione------------------------------------------------
  clip = CLIP_map(mozziAnalogRead(POT4_PIN));
  //Serial.println(LEDCLIP_map(clip));
  if (clip<200) analogWrite(LEDCLIP_PIN,LEDCLIP_map(clip)); // Da quello che ho capito analogWrite non si può usare
  else analogWrite(LEDCLIP_PIN,0);
  gain = GAIN_map(clip);


}



AudioOutput_t updateAudio() {
  Sum = ( ((lpf0.next(Osc0.next())*4)*ON0) + ((lpf1.next(Osc1.next())*7)*ON1) ); // somma i due osc con guadagni diversi
  Sum = MonoOutput::fromAlmostNBit(13,Sum); // ricentro il segnale su 8 bit
  // Gain e clip sono opposti, il potenziometro 4 controlla la soglia del Clip e più bassa è la soglia più il volume è alto 
  if (Sum>clip-50) Sum=clip-50;
  else if (Sum<(-clip)) Sum=-clip;
  Sum = MonoOutput::from16Bit(Sum*gain).clip(); 
  return MonoOutput::from8Bit(Sum); 
}

void loop() {
  audioHook(); // required here
}
