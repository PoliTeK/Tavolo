#include <MozziGuts.h>
#include <Oscil.h> // oscillator template
#include <tables/sin4096_int8.h> // sine table for oscillator
 // richiamo dalla classe l'elemento Sin0 con wavetable da 4096 elementi
Oscil <SIN4096_NUM_CELLS, AUDIO_RATE> Sin0(SIN4096_DATA);
Oscil <SIN4096_NUM_CELLS, AUDIO_RATE> Sin1(SIN4096_DATA);
Oscil <SIN4096_NUM_CELLS, AUDIO_RATE> Sin2(SIN4096_DATA);
Oscil <SIN4096_NUM_CELLS, AUDIO_RATE> Sin3(SIN4096_DATA);

void setup() {
  startMozzi(); // :)

}

void updateControl(){
  int freq = map (mozziAnalogRead(A0), 0,1023,440,880); // il potenziometro controlla la frequenza da 440Hz(La5) a 880 (La6)
  Sin0.setFreq(freq);
  Sin1.setFreq(int(freq*1.25)); //440 * 1.25 = 550 = DO#
  Sin2.setFreq(int(freq*1.5));// Mi
  Sin3.setFreq(int(freq*2));// La un ottava sopra ... insomma ho costruito un accordo maggiore

  
}

//sin.next() restituisce un intero che sarà il return della funzione audio_output, perciò sommo sini.next() e scarto i 4 MSB di overflow dividendo per 16 per evitare il clipping
AudioOutput_t updateAudio(){
  int Sum = (Sin0.next() + Sin1.next() + Sin2.next() + Sin3.next())/16;
  return MonoOutput::from8Bit(Sum); 
}


void loop(){
  audioHook(); // required here
}


