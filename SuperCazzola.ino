#include <Oscil.h>
#include <EventDelay.h>
#include <ADSR.h>
#include <tables/waveshape1_softclip_int8.h>
#include <tables/triangle512_int8.h> // wavetable
#include <mozzi_rand.h>
#include <mozzi_midi.h>
#include <Oscil.h> // oscillator template
#include <tables/saw2048_int8.h>
#include <AudioDelayFeedback.h>

#define CONTROL_RATE 128 // Hz, powers of 2 are most reliable

// Definizione note associate a frequenze
#define La0 27.5
#define Si0 30.87
#define Do1 32.7
#define Re1 36.71
#define Mi1 41.2
#define Fa1 43.65
#define Sol1 49
#define FaD1 46.25
#define SolD1 51.91
#define La1 La0*2

float scale[20] = {La0*32,Si0*32,Do1*32,Re1*32,Mi1*32,Fa1*32,Sol1*32,La1*32,La0*16,Si0*16,Do1*16,Re1*16,Mi1*16,Fa1*16,Sol1*16,La1*16};

Oscil <WAVESHAPE1_SOFTCLIP_NUM_CELLS, AUDIO_RATE> aOscil(WAVESHAPE1_SOFTCLIP_DATA);// SUPERCAZZOLA
ADSR <AUDIO_RATE, AUDIO_RATE> envelope;
EventDelay noteDelay;



void setup() {
  startMozzi(CONTROL_RATE);
  Serial.begin(9600);
  randSeed(); // fresh random
  noteDelay.set(2000); // 2 second countdown
}

unsigned int duration, attack, decay, sustain, release_ms;

void updateControl(){
  if(noteDelay.ready()){

      // choose envelope levels
      byte attack_level =rand(50) + 50;
      byte decay_level = 20;
      envelope.setADLevels(attack_level,decay_level);

    // generate a random new adsr time parameter value in milliseconds
     unsigned int new_value = rand(100) +300;
     
     // randomly choose one of the adsr parameters and set the new value

     attack = 5;
   
     decay = 50;
   
     sustain = 50;

     release_ms =new_value;
     
     envelope.setTimes(attack,decay,sustain,release_ms);
     envelope.noteOn();

     int note = rand(15);
     aOscil.setFreq(scale[note]);
     int pianta = (mozziAnalogRead(A4)/(3+rand(5)))*rand(6);
     Serial.println(pianta);

     noteDelay.start(attack+decay+sustain+pianta);
   }
}


AudioOutput_t updateAudio(){
  envelope.update();
  return MonoOutput::from16Bit((int) (envelope.next() * aOscil.next()));
}


void loop(){
  audioHook(); // required here
}

