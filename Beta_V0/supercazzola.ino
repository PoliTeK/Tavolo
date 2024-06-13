/*  CODICE SUPERCAZZOLA STM32
Questo codice implementa la supercazzola
*/

#include <MozziGuts.h>
// oscillator templates
#include <Oscil.h>
#include <tables/saw2048_int8.h> 
#include <tables/triangle_hermes_2048_int8.h>
#include <tables/square_no_alias_2048_int8.h>
#include <tables/envelop2048_uint8.h>
#include <tables/sin2048_int8.h> 
// libreria per map veloce
#include <IntMap.h> 

#include <EventDelay.h> // libreria ritardi di mozzi 
//libreria ADSR
#include <ADSR.h>
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
const IntMap GEN_map(0,4090,0,50);   // Per STM32 con potenziometri collegati a 3.3V
const IntMap NOTE_map(0,45,0,7);   
const IntMap CUTOFF_map(0,4096,200,5000);
const IntMap CLIP_map(0,4096,70,255);
const IntMap LEDCLIP_map(50,255,255,0);
const IntMap GAIN_map(70,255,65,20);

//--------------------------------------------Supercazzola------------------------------------------------------
ADSR <AUDIO_RATE, AUDIO_RATE> envelope;
EventDelay noteDelay;
unsigned int duration, attack, decay, sustain, release_ms;


//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void setup() {
  //Serial.begin(9600);
  pinMode(P0_PIN,INPUT);
  pinMode(LEDM_PIN,OUTPUT);
  startMozzi(CONTROL_RATE); // :)
  
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void updateControl() { 

//----------------------------------------Note Voci----------------------------------------------------------- 
  // Vettore con tutti i valori letti dai potenziometri
  int letture[4] = { GEN_map(mozziAnalogRead(POT0_PIN)), GEN_map(mozziAnalogRead(POT1_PIN)), GEN_map(mozziAnalogRead(POT2_PIN)), GEN_map(mozziAnalogRead(POT3_PIN))};
  int valori_discreti[4];    // Variabili per il calcolo delle note e della frequenza dei vari oscillatori
  float frequenze_base[4];
  // Discretizzazione delle frequenze iterate per ogni oscillatore 
  for (int i = 0; i < 4; i++) 
  {
    valori_discreti[i] = NOTE_map(letture[i]);
    frequenze_base[i] = note[valori_discreti[i]];
  }
// setto le frequenze in base alle letture
  Osc0.setFreq(frequenze_base[0]*2);
  Osc1.setFreq(frequenze_base[1]*4);
  Osc2.setFreq(frequenze_base[2]*8);  
  Osc3.setFreq(frequenze_base[3]*16);

// ------------------------------------------------Supercazzola----------------------------------------------------------------
  // note selezionate in base alle note delle voci in quel momento (1a, 5a, 1a +8,5a +8)
  float Note_Pianta[16] = {frequenze_base[0]*32, frequenze_base[1]*32, frequenze_base[2]*32, frequenze_base[3]*32,

                           frequenze_base[0]*24, frequenze_base[1]*24, frequenze_base[2]*24, frequenze_base[3]*24,

                           frequenze_base[0]*32, frequenze_base[1]*16, frequenze_base[2]*16, frequenze_base[3]*16,

                           frequenze_base[0]*12, frequenze_base[1]*12, frequenze_base[2]*12, frequenze_base[3]*12};
 
  // se è passato il tempo settato con noteDelay.start setta ADSR
  if(noteDelay.ready()){
      // valori massimi dell'envelope                                            // .attack levek |      ^              //
      byte attack_level =rand(50) + 50;                                          //               |     / \             //             
      byte decay_level = 20;                                                     //  decay level  |    /   \________    //                  
      envelope.setADLevels(attack_level,decay_level);                            //               |   /             \   //                   
      attack = 5;  // At                                                         //               |  /               \  //  
      decay = 50;  // Dt                                                         //               + --------------------//
      sustain = 50;// St                                                         //             0       ^  ^       ^  ^ //
      release_ms = rand(100) +300; // Rt                                         //                     |  |       |  |  //
      envelope.setTimes(attack,decay,sustain,release_ms);                        //                     At Dt     St  Rt//
      envelope.noteOn();
      int j = rand(15);
      Osc4.setFreq(Note_Pianta[j]); 
      // tempo tra una nota e l'altra deciso dal sensore di profondità con l'aiuto di rand
      int pianta = (mozziAnalogRead(DEPTH4_PIN)/(3+rand(5)))*rand(6);
      noteDelay.start(attack+decay+sustain+pianta); // setto la prox attesa
  }
}

int Sum = 0;


AudioOutput_t updateAudio() {
  Sum = ( (lpf1.next(Osc0.next())*4) + (lpf2.next(Osc1.next())*7) ); // somma i due osc con guadagni diversi
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
