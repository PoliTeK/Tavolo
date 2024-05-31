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

// Definiamo i pin a cui colleghiamo potenziometri 
#define POT0_PIN A0 
#define POT1_PIN A1
#define POT2_PIN A2 
#define POT3_PIN A3
#define POT4_PIN A4 
#define POT5_PIN A5
#define P0_PIN 4      // Pulsante per attivare o disattivare la modulazione
#define LEDM_PIN 5    // Led per visualizzare la freq di modulazione

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
const IntMap GEN_map(0,4090,0,50);   // Per STM32 con potenziometri collegati a 3.3V
const IntMap NOTE_map(0,45,0,7);   
const IntMap MOD_map(0,45,0,50);
const IntMap WAT_map(400,450,0,50);

//--------------------------------------------Pulsante-------------------------------------------------
bool Switch_RM=false;


unsigned long debounceDelay = 50; // Ritardo di debounce in millisecondi
bool buttonState = false; // Variabile booleana da invertire
bool lastButtonState = false; // Stato precedente del pulsante
unsigned long lastDebounceTime = 0; // Tempo dell'ultimo cambiamento di stato del pulsante  


//--------------------------------------------Oscillatori Voci-------------------------------------------
Oscil <TRIANGLE_HERMES_2048_NUM_CELLS, AUDIO_RATE> Osc0(TRIANGLE_HERMES_2048_DATA); // oscillatore SubBass
Oscil <SAW2048_NUM_CELLS, AUDIO_RATE> Osc1(SAW2048_DATA); // Oscillatore  basso
Oscil <SAW2048_NUM_CELLS, AUDIO_RATE> Osc2(SAW2048_DATA); // oscillatore medio basso
Oscil <TRIANGLE_HERMES_2048_NUM_CELLS, AUDIO_RATE> Osc3(TRIANGLE_HERMES_2048_DATA); // oscillatore medio alto
Oscil <SAW2048_NUM_CELLS, AUDIO_RATE> Osc4(SAW2048_DATA);// SUPERCAZZOLA
// Array di oscillatori di voci per poterci iterare sopra
Oscil <2048, AUDIO_RATE> oscillatori[] = { Osc0, Osc1, Osc2, Osc3, Osc4 };

//--------------------------------------------Oscillatori Modulazioni-------------------------------------------
Oscil <SIN2048_NUM_CELLS, AUDIO_RATE> Carrier2(SIN2048_DATA); // portante per Ring Modulatin(RM)  Osc1 
Oscil <SIN2048_NUM_CELLS, AUDIO_RATE> Carrier1(SIN2048_DATA);



//--------------------------------------------Supercazzola------------------------------------------------------
ADSR <AUDIO_RATE, AUDIO_RATE> envelope;
EventDelay noteDelay;



//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void setup() {
  //Serial.begin(9600);
  pinMode(P0_PIN,INPUT);
  pinMode(LEDM_PIN,OUTPUT);
  startMozzi(CONTROL_RATE); // :)
}

unsigned int duration, attack, decay, sustain, release_ms;

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
  int letture[5] = { GEN_map(mozziAnalogRead(POT0_PIN)), GEN_map(mozziAnalogRead(POT1_PIN)), GEN_map(mozziAnalogRead(POT2_PIN)), GEN_map(mozziAnalogRead(POT3_PIN)), GEN_map(mozziAnalogRead(POT4_PIN))};
  int valori_discreti[5];    // Variabili per il calcolo delle note e della frequenza dei vari oscillatori
  float frequenze_base[5];
  // Discretizzazione delle frequenze iterate per ogni oscillatore 
  for (int i = 0; i < 5; i++) 
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
// setto le frequenze in base alle eltture
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
 
  // se è passato il temo settato con noteDelay.start
  if(noteDelay.ready()){
      // valori massimi dell'envelope                                            // .attack levek |      ^
      byte attack_level =rand(50) + 50;                                          //               |     / \                          
      byte decay_level = 20;                                                     //  decay level  |    /   \________                      
      envelope.setADLevels(attack_level,decay_level);                            //               |   /             \                      
      attack = 5;  // At                                                         //               |  /               \    
      decay = 50;  // Dt                                                         //               + --------------------
      sustain = 50;// St                                                         //             0       ^  ^       ^  ^
      release_ms = rand(100) +300;; // Rt                                        //                     |  |       |  |
      envelope.setTimes(attack,decay,sustain,release_ms);                        //                     At Dt     St  Rt
      envelope.noteOn();
      int j = rand(15);
      Osc4.setFreq(Note_Pianta[j]); 
      // tempo tra una nota e l'altra deciso dal sensore di profondità con l'aiuto di rand
      int pianta = (mozziAnalogRead(A4)/(3+rand(5)))*rand(6);
      noteDelay.start(attack+decay+sustain+pianta); // setto la prox attesa
  }
//-------------------------------------------------------RM-------------------------------------------------------------------------
  float Carrier2_Freq = MOD_map(GEN_map(mozziAnalogRead(POT5_PIN)));// leggo il potenziometro per determinare la frequenza aggiuntiva
  Carrier1.setFreq(frequenze_base[3]*48+Carrier2_Freq/8); // portante1: è un'ottava sotto il segnale modulato + la frequenza letta dal potenziometro(divido per 8 per avere più precisione)
  Carrier2.setFreq(frequenze_base[2]*24+Carrier2_Freq/10); // portante2: è alla stessa ottava del segnale modulato + la frequenza letta dal potenziometro(divido per 10 per avere una precisione diversa dall'altra portante)
  //  modo per fare oscillare il led a tempo con la modulazione
  if(Switch_RM==true){  
    if(Carrier1.next()<0){
      analogWrite(LEDM_PIN,0);
    }
    else analogWrite(LEDM_PIN,Carrier1.next());   
  }
  else analogWrite(LEDM_PIN,0);

}

int Sum = 0;


AudioOutput_t updateAudio() {
  envelope.update();
  if (Switch_RM==true){
    Sum = (
    Osc0.next() * 8 +
    Osc1.next() * 3 +
    ((Carrier2.next()>>9))*Osc2.next() * 4 +
    ((Carrier1.next()>>9))*Osc3.next() * 3 +
    (envelope.next() * Osc4.next()>>4)  )>> 5; 
    // moltiplico il segnale per le 2 portanti divise (di molto) secondo il gusto per rendere la modulazione il giusto invasiva
  }
  else{
    Sum = (
    Osc0.next() * 8 +
    Osc1.next() * 3 +
    Osc2.next() * 4 +
    Osc3.next() * 3 +
    (envelope.next() * Osc4.next()>>4) ) >> 5;
  }

  return MonoOutput::from8Bit(Sum);
}

void loop() {
  audioHook(); // required here
}
