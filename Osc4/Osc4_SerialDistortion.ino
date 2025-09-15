/*  CODICE OSC4 STM32
Questo codice implementa l'oscillatore alto che arpeggia in modo randomico
A4 collegato al sensore di livello dell'acqua (da ricalibrare il map).
I pin digitali 2 e 4 sono usati per leggere la pianta come un pulsante. (pin 2 collegato nella pianta)


*/




#include <MozziGuts.h>
// oscillator templates
#include <Oscil.h>
#include <tables/triangle_hermes_2048_int8.h>
#include <tables/saw2048_int8.h> 
#include <tables/square_no_alias_2048_int8.h>
#include <tables/envelop2048_uint8.h>
#include <tables/sin2048_int8.h>
#include <tables/uphasor256_uint8.h>
// libreria per map veloce
#include <IntMap.h> 

#include <EventDelay.h> // libreria ritardi di mozzi 
//libreria ADSR
#include <ADSR.h>
// libreria Random function
#include <mozzi_rand.h>
// Libreria sensori capacitivi (per usare la pianta come pulsante)
#include <CapacitiveSensor.h>



//#define AUDIO_RATE 44100 // Hz   
#define AUDIO_RATE 16384 // Hz

// Definiamo i pin a cui colleghiamo potenziometri 
#define POT0_PIN A0 
#define POT1_PIN A1
#define POT2_PIN A2 
#define POT3_PIN A3
#define POT4_PIN A4 //depth
#define POT5_PIN A5
#define ONPianta1 8
#define ONPianta2 10
#define ONPianta3 11
    

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
const IntMap GEN_map(0,800,0,50);   // Per STM32 con potenziometri collegati a 3.3V
const IntMap NOTE_map(0,45,0,7);   
const IntMap MOD_map(0,45,0,50);
const IntMap SENS_map(0,800,400,600);
const IntMap THRES_map(0,800,10,1000);




//--------------------------------------------Oscillatori Voci-------------------------------------------
Oscil <TRIANGLE_HERMES_2048_NUM_CELLS, AUDIO_RATE> Osc4(TRIANGLE_HERMES_2048_DATA);// SUPERCAZZOLA
Oscil <UPHASOR256_NUM_CELLS, AUDIO_RATE> Osc5(UPHASOR256_DATA); 
Oscil <SAW2048_NUM_CELLS, AUDIO_RATE> Osc6(SAW2048_DATA); 
int Sum = 0;
int ON[3]={0,0,0};



//--------------------------------------------Supercazzola------------------------------------------------------
ADSR <AUDIO_RATE, AUDIO_RATE> envelope1;
ADSR <AUDIO_RATE, AUDIO_RATE> envelope2;
ADSR <AUDIO_RATE, AUDIO_RATE> envelope3;
EventDelay noteDelay1, noteDelay2,noteDelay3 ;
unsigned int duration, attack, decay, sustain, release_ms1, release_ms2, release_ms3;


// create an instance of the library
// pin 4 sends electrical energy
// pin 2 senses senses a change
CapacitiveSensor capSensor1 = CapacitiveSensor(3, 2);
CapacitiveSensor capSensor2 = CapacitiveSensor(5, 4);
CapacitiveSensor capSensor3 = CapacitiveSensor(7, 6);
// threshold for turning the lamp on



//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void setup() {
  //Serial.begin(1200);
  pinMode(ONPianta1,OUTPUT);
  pinMode(ONPianta2,OUTPUT);
  pinMode(ONPianta3,OUTPUT);
  startMozzi(CONTROL_RATE ); // :)
  
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void updateControl() { 


//----------------------------------------Nota Voci----------------------------------------------------------- 
  // prende una frequenza a caso generando un numero randomico da 0 a 7 e usando la mappature note []
  
  float frequenza_base = note[rand(8)];
  // la nota generata viene alzata alla : 5a ottava sup, 4 ottava sup, quarta 5a superiore, terza 5a superiore
  float selezione_armoniche[4] = {12, 16, 24, 32};
  float Nota_Pianta1 = frequenza_base * selezione_armoniche[rand(4)];
  float Nota_Pianta2 = frequenza_base * selezione_armoniche[rand(4)];
  float Nota_Pianta3 = frequenza_base * selezione_armoniche[rand(4)];
  int threshold1, threshold2, threshold3;
  long sensorValue1,sensorValue2, sensorValue3;
      

 
  // se è passato il tempo settato con noteDelay.start setta ADSR
  if(noteDelay1.ready()){
      threshold1 = THRES_map(mozziAnalogRead(POT1_PIN));
      // store the value reported by the plant in a variable
      sensorValue1= capSensor1.capacitiveSensor(30);
      //-----------------------------
      
      // if the value is greater than the threshold
      
        if (sensorValue1 > threshold1) {
          ON[0]=1;
          digitalWrite(ONPianta1,1);
        }
        else {
          if (ON[0]) {
            digitalWrite(ONPianta1,0);
            ON[0]=0;
          }
        }
      // valori massimi dell'envelope                                            // .attack levek |      ^              //
      byte attack_level =rand(50) + 50;                                          //               |     / \             //             
      byte decay_level = 20;                                                     //  decay level  |    /   \________    //                  
      envelope1.setADLevels(attack_level,decay_level);                           //               |   /             \   //                   
      attack = 5;  // At                                                         //               |  /               \  //  
      decay = 50;  // Dt                                                         //               + --------------------//
      sustain = 50;// St                                                         //             0       ^  ^       ^  ^ //
      release_ms1 = rand(100) +300; // Rt                                         //                     |  |       |  |  //
      envelope1.setTimes(attack,decay,sustain,release_ms1);                        //                     At Dt     St  Rt//
      envelope1.noteOn();
      Osc4.setFreq(Nota_Pianta1); 
     
      // tempo tra una nota e l'altra deciso rand
      int pianta = rand(50,500);
      noteDelay1.start(attack+decay+sustain+release_ms1+pianta); // setto la prox attesa
  }

  if(noteDelay2.ready()){
      threshold2 = THRES_map(mozziAnalogRead(POT2_PIN));
      // store the value reported by the plant in a variable
      sensorValue2= capSensor2.capacitiveSensor(30);
      //-----------------------------
      
      // if the value is greater than the threshold
      
        if (sensorValue2 > threshold2) {
          ON[1]=1;
          digitalWrite(ONPianta2,1);
        }
        else {
          if (ON[1]) {
            digitalWrite(ONPianta2,0);
            ON[1]=0;
          }
        }
      // valori massimi dell'envelope                                            // .attack levek |      ^              //
      byte attack_level =rand(50) + 50;                                          //               |     / \             //             
      byte decay_level = 20;                                                     //  decay level  |    /   \________    //                  
      envelope2.setADLevels(attack_level,decay_level);                           //               |   /             \   //                   
      attack = 5;  // At                                                         //               |  /               \  //  
      decay = 50;  // Dt                                                         //               + --------------------//
      sustain = 50;// St                                                         //             0       ^  ^       ^  ^ //
      release_ms2 = rand(100) +300; // Rt                                         //                     |  |       |  |  //
      envelope2.setTimes(attack,decay,sustain,release_ms2);                        //                     At Dt     St  Rt//
      envelope2.noteOn();
      Osc5.setFreq(Nota_Pianta2*0.5f); 
     
      // tempo tra una nota e l'altra deciso rand
      int pianta = rand(50,500);
      noteDelay2.start(attack+decay+sustain+release_ms2+pianta); // setto la prox attesa
  }

  if(noteDelay3.ready()){
      threshold3 = THRES_map(mozziAnalogRead(POT3_PIN));
      // store the value reported by the plant in a variable
      sensorValue3= capSensor3.capacitiveSensor(30);
      //-----------------------------
      
      // if the value is greater than the threshold
      
        if (sensorValue3 > threshold3) {
          ON[2]=1;
          digitalWrite(ONPianta3,1);
        }
        else {
          if (ON[2]) {
            digitalWrite(ONPianta3,0);
            ON[2]=0;
          }
        }
      // valori massimi dell'envelope                                            // .attack levek |      ^              //
      byte attack_level =rand(50) + 50;                                          //               |     / \             //             
      byte decay_level = 20;                                                     //  decay level  |    /   \________    //                  
      envelope3.setADLevels(attack_level,decay_level);                           //               |   /             \   //                   
      attack = 5;  // At                                                         //               |  /               \  //  
      decay = 50;  // Dt                                                         //               + --------------------//
      sustain = 50;// St                                                         //             0       ^  ^       ^  ^ //
      release_ms3 = rand(100) +300; // Rt                                         //                     |  |       |  |  //
      envelope3.setTimes(attack,decay,sustain,release_ms3);                        //                     At Dt     St  Rt//
      envelope3.noteOn();
      Osc6.setFreq(Nota_Pianta3*0.25f); 
     
      // tempo tra una nota e l'altra deciso rand
      int pianta = rand(50,500);
      noteDelay3.start(attack+decay+sustain+release_ms3+pianta); // setto la prox attesa
  }
  /*Serial.println("-------\t Treshold1 \t SensorValue1");
      Serial.print("pianta1 \t");
      Serial.print(threshold1);
      Serial.print("\t\t\t");
      Serial.println(sensorValue1);
      Serial.println("-------\t Treshold2 \t SensorValue2");
      Serial.print("pianta2 \t");
      Serial.print(threshold2);
      Serial.print("\t\t\t");
      Serial.println(sensorValue2);
      Serial.println("-------\t Treshold3 \t SensorValue3");
      Serial.print("pianta3 \t");
      Serial.print(threshold3);
      Serial.print("\t\t\t");
      Serial.println(sensorValue3);
      Serial.print("\n\n");*/

}




AudioOutput_t updateAudio() {
  envelope1.update();
  envelope2.update();
  envelope3.update();

  return MonoOutput::from16Bit( 
                                 envelope1.next() *Osc4.next()*ON[0] +
                                 envelope2.next() *Osc5.next()*ON[1] +
                                 envelope3.next() *Osc6.next()*ON[2]);
}

void loop() {
  audioHook(); // required here
}
