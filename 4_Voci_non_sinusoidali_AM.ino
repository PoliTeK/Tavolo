


#include <MozziGuts.h>
#include <Oscil.h> // oscillator template
#include <tables/saw2048_int8.h>
#include <tables/triangle_hermes_2048_int8.h>
#include <tables/square_no_alias_2048_int8.h>
#include <tables/envelop2048_uint8.h>
#include <tables/sin2048_int8.h> 
#include <IntMap.h>

#define AUDIO_RATE 44100  // Hz    // Se si utilizza STM32
// #define AUDIO_RATE 16384 // Hz  Se si utilizza Arduino

// Definizione note associate a frequenze
#define La0 27.5
#define Si0 30.87
#define Do1 32.7
#define Re1 36.71
#define Mi1 41.2
#define Fa1 43.65
#define Sol1 49
#define La1 La0*2

// definizione oggetto per mappare in modo corretto (non si può usare map())
const IntMap NOTE_map(0,1023,0,7); // Per selezionare le note
const IntMap MOD_map(0,1023,0,50); // Per la frequenza di modulazione


Oscil <TRIANGLE_HERMES_2048_NUM_CELLS, AUDIO_RATE> Osc0(TRIANGLE_HERMES_2048_DATA);
Oscil <SAW2048_NUM_CELLS, AUDIO_RATE> Osc1(SAW2048_DATA);
Oscil <SAW2048_NUM_CELLS, AUDIO_RATE> Osc2(SAW2048_DATA);
Oscil <TRIANGLE_HERMES_2048_NUM_CELLS, AUDIO_RATE> Osc3(TRIANGLE_HERMES_2048_DATA);
Oscil <ENVELOP2048_NUM_CELLS, AUDIO_RATE> Osc4(ENVELOP2048_DATA);

Oscil <SIN2048_NUM_CELLS, AUDIO_RATE> Carrier2(SIN2048_DATA); // portante per Ring Modulatin(RM)  Osc1 
Oscil <SIN2048_NUM_CELLS, AUDIO_RATE> Carrier1(SIN2048_DATA);
// Array di oscillatori per poterci iterare sopra
Oscil <2048, AUDIO_RATE> oscillatori[] = { Osc0, Osc1, Osc2, Osc3, Osc4 };

void setup() {
  startMozzi(CONTROL_RATE); // :)
}

void updateControl() {
  int letture[] = { mozziAnalogRead(A0), mozziAnalogRead(A1), mozziAnalogRead(A2), mozziAnalogRead(A3), mozziAnalogRead(A4)};
  int valori_discreti[5];
  float frequenze_base[5];
  // Discretizzazione delle frequenze iterate per ogni oscillatore 
  for (int i = 0; i < 5; i++) {
    // valori_discreti[i] = map(letture[i], 0, 1023, 0, 7);   La funzione map non si può usare perchè è troppo lenta
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

  Osc0.setFreq(frequenze_base[0]*4);
  Osc1.setFreq(frequenze_base[1]*8);
  Osc2.setFreq(frequenze_base[2]*16);
  Osc3.setFreq(frequenze_base[3]*32);
  Osc4.setFreq(frequenze_base[4]*64);


  // float Carrier2_Freq = map(mozziAnalogRead(A5), 0, 1023, 0, 50 );// leggo il potenziometro per determinare la frequenza aggiuntiva
  float Carrier2_Freq = MOD_map(mozziAnalogRead(A5));
  Carrier1.setFreq(frequenze_base[2]*8+Carrier2_Freq/8); // portante1: è un'ottava sotto il segnale modulato + la frequenza letta dal potenziometro(divido per 8 per avere più precisione)
  Carrier2.setFreq(frequenze_base[2]*16+Carrier2_Freq/10); // portante2: è alla stessa ottava del segnale modulato + la frequenza letta dal potenziometro(divido per 10 per avere una precisione diversa dall'altra portante)


}

//Osc.next() restituisce un intero che sarà il return della funzione audio_output, perciò sommo Osc.next() e scarto i n-oscilla MSB di overflow attraverso un bitshift (>>)
AudioOutput_t updateAudio() {

  int Sum = (Osc0.next()*8 +  Osc1.next() * 3 + (Carrier2.next()>>15)*Osc2.next()*(Carrier1.next()>>15) * 4 + Osc3.next()*2 + Osc4.next()*2)>>5; // moltiplico il segnale per le 2 portanti divise (di molto) secondo il gusto per rendere la modulazione il giusto invasiva
  
  return MonoOutput::from8Bit(Sum);
}

void loop() {
  audioHook(); // required here
}
