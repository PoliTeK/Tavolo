#include <MozziGuts.h>
#include <Oscil.h> // oscillator template
#include <tables/sin4096_int8.h> // sine table for oscillator
#include <tables/square_no_alias_2048_int8.h>

#define AUDIO_RATE 32768 // Hz

// Definizione note associate a frequenze
#define La0 27.5
#define Si0 30.87
#define Do1 32.7
#define Re1 36.71
#define Mi1 41.2
#define Fa1 43.65
#define Sol1 49
#define La1 La0*2


Oscil <SIN4096_NUM_CELLS, AUDIO_RATE> Sin0(SIN4096_DATA);
Oscil <SIN4096_NUM_CELLS, AUDIO_RATE> Sin1(SIN4096_DATA);
Oscil <SIN4096_NUM_CELLS, AUDIO_RATE> Sin2(SIN4096_DATA);
Oscil <SIN4096_NUM_CELLS, AUDIO_RATE> Sin3(SIN4096_DATA);
Oscil <SIN4096_NUM_CELLS, AUDIO_RATE> Sin4(SIN4096_DATA);

// Array di oscillatori per poterci iterare sopra
Oscil <4096, AUDIO_RATE> oscillatori[] = { Sin0, Sin1, Sin2, Sin3, Sin4 };

void setup() {
  startMozzi(CONTROL_RATE); // :)
}

void updateControl() {
  int letture[] = { mozziAnalogRead(A0), mozziAnalogRead(A1), mozziAnalogRead(A2), mozziAnalogRead(A3), mozziAnalogRead(A4)};
  int valori_discreti[5];
  float frequenze_base[5];

  // Discretizzazione delle frequenze iterate per ogni oscillatore 
  for (int i = 0; i < 5; i++) {
    valori_discreti[i] = map(letture[i], 0, 1023, 0, 7);
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

  Sin0.setFreq(frequenze_base[0]*4);
  Sin1.setFreq(frequenze_base[1]*8);
  Sin2.setFreq(frequenze_base[2]*16);
  Sin3.setFreq(frequenze_base[3]*32);
  Sin4.setFreq(frequenze_base[4]*64);
}

//sin.next() restituisce un intero che sarà il return della funzione audio_output, perciò sommo sin.next() e scarto i 4 MSB di overflow dividendo per 16 per evitare il clipping
AudioOutput_t updateAudio() {
  int Sum = (Sin0.next() + Sin1.next() + Sin2.next() + Sin3.next() + Sin4.next())>>5;
  return MonoOutput::from8Bit(Sum);
}

void loop() {
  audioHook(); // required here
}
