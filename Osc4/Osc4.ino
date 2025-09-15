/*  CODICE OSC4 STM32
Questo codice implementa l'oscillatore alto che arpeggia in modo randomico
A4 collegato al sensore di livello dell'acqua (da ricalibrare il map).
I pin digitali 2 e 4 sono usati per leggere la pianta come un pulsante. (pin 2 collegato nella pianta)


*/




#include <MozziGuts.h>
// oscillator templates
#include <Oscil.h>
#include <tables/triangle_hermes_2048_int8.h>
#include <tables/envelop2048_uint8.h>
#include <tables/sin2048_int8.h> 
// libreria per map veloce
#include <IntMap.h> 

#include <EventDelay.h> // libreria ritardi di mozzi 
//libreria ADSR
#include <ADSR.h>
// libreria Random function
#include <mozzi_rand.h>
// Libreria sensori capacitivi (per usare la pianta come pulsante)
#include <CapacitiveSensor.h>



#define AUDIO_RATE 44100 // Hz   
//#define AUDIO_RATE 16384 // Hz

// Definiamo i pin a cui colleghiamo potenziometri 
#define POT0_PIN A0 
#define POT1_PIN A1
#define POT2_PIN A2 
#define POT3_PIN A3
#define POT4_PIN A4 //depth
#define POT5_PIN A5
#define Display_PIN 5
    

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
const IntMap SENS_map(0,4096,400,600);
const IntMap THRES_map(0,4096,10,800);




//--------------------------------------------Oscillatori Voci-------------------------------------------
Oscil <TRIANGLE_HERMES_2048_NUM_CELLS, AUDIO_RATE> Osc4(TRIANGLE_HERMES_2048_DATA);// SUPERCAZZOLA
int Sum = 0;
int ON=0;



//--------------------------------------------Supercazzola------------------------------------------------------
ADSR <AUDIO_RATE, AUDIO_RATE> envelope;
EventDelay noteDelay;
unsigned int duration, attack, decay, sustain, release_ms;


// create an instance of the library
// pin 4 sends electrical energy
// pin 2 senses senses a change
CapacitiveSensor capSensor = CapacitiveSensor(4, 2);
// threshold for turning the lamp on
int threshold = 10; // Da calibrare, si potrebbe mettere un potenziometro per rendere la pianta più o meno sensibile


//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void setup() {
  Serial.begin(9600);
  pinMode(Display_PIN,OUTPUT);
  startMozzi(CONTROL_RATE); // :)
  
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void updateControl() { 


//----------------------------------------Note Voci----------------------------------------------------------- 
  // Vettore con tutti i valori letti dai potenziometri
  // Devono essere gli stessi potenziometri che comandano le note dell'osc01 e osc23
  int letture[4] = {GEN_map(mozziAnalogRead(POT0_PIN)), GEN_map(mozziAnalogRead(POT1_PIN)), GEN_map(mozziAnalogRead(POT2_PIN)), GEN_map(mozziAnalogRead(POT3_PIN))};
  int valori_discreti[4];    // Variabili per il calcolo delle note e della frequenza dei vari oscillatori
  float frequenze_base[4];
  // Discretizzazione delle frequenze iterate per ogni oscillatore 
  for (int i = 0; i < 4; i++) 
  {
    valori_discreti[i] = NOTE_map(letture[i]);
    frequenze_base[i] = note[valori_discreti[i]];
  }


// ------------------------------------------------Supercazzola----------------------------------------------------------------
  // note selezionate in base alle note delle voci in quel momento (1a, 5a, 1a +8,5a +8)
  float Note_Pianta[16] = {frequenze_base[0]*32, frequenze_base[1]*32, frequenze_base[2]*32, frequenze_base[3]*32,

                           frequenze_base[0]*24, frequenze_base[1]*24, frequenze_base[2]*24, frequenze_base[3]*24,

                           frequenze_base[0]*32, frequenze_base[1]*16, frequenze_base[2]*16, frequenze_base[3]*16,

                           frequenze_base[0]*12, frequenze_base[1]*12, frequenze_base[2]*12, frequenze_base[3]*12};
 
  // se è passato il tempo settato con noteDelay.start setta ADSR
  if(noteDelay.ready()){
      threshold=THRES_map(mozziAnalogRead(POT5_PIN));
      // store the value reported by the plant in a variable
      long sensorValue = capSensor.capacitiveSensor(30);
      //Serial.println(sensorValue);
      // if the value is greater than the threshold
      if (sensorValue > threshold) {
        ON=1;
        digitalWrite(Display_PIN,1);
      }
      else {
        ON=0;
        digitalWrite(Display_PIN,0);
      }
      // valori massimi dell'envelope                                            // .attack levek |      ^              //
      byte attack_level =rand(50) + 50;                                          //               |     / \             //             
      byte decay_level = 20;                                                     //  decay level  |    /   \________    //                  
      envelope.setADLevels(attack_level,decay_level);                            //               |   /             \   //                   
      attack = 5;  // At                                                         //               |  /               \  //  
      decay = 50;  // Dt                                                         //               + --------------------//
      sustain = 50;// St                                                         //             0       ^  ^       ^  ^ //
      release_ms = rand(100) +300; // Rt                                        //                     |  |       |  |  //
      envelope.setTimes(attack,decay,sustain,release_ms);                        //                     At Dt     St  Rt//
      envelope.noteOn();
      int j = rand(15);
      Osc4.setFreq(Note_Pianta[j]); 
      // tempo tra una nota e l'altra deciso dal sensore di profondità con l'aiuto di rand
      int sens =SENS_map(mozziAnalogRead(POT4_PIN));
      int pianta = (sens/(3+rand(5)))*rand(6);
      noteDelay.start(attack+decay+sustain+release_ms+pianta); // setto la prox attesa
  }
  

}




AudioOutput_t updateAudio() {
  envelope.update();
  return MonoOutput::from16Bit(envelope.next() * Osc4.next()*ON);
}

void loop() {
  audioHook(); // required here
}
