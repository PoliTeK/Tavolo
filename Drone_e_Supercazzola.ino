#include <MozziGuts.h>
#include <Oscil.h> // oscillator template
#include <tables/saw2048_int8.h>
#include <tables/triangle_hermes_2048_int8.h>
#include <tables/square_no_alias_2048_int8.h>
#include <tables/envelop2048_uint8.h>
#include <tables/sin2048_int8.h> 
#include <IntMap.h> // libreria per map veloce

#include <EventDelay.h>
#include <ADSR.h>
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


bool Switch_RM=false;
int Sum = 0;

unsigned long debounceDelay = 50; // Ritardo di debounce in millisecondi
bool buttonState = false; // Variabile booleana da invertire
bool lastButtonState = false; // Stato precedente del pulsante
unsigned long lastDebounceTime = 0; // Tempo dell'ultimo cambiamento di stato del pulsante  



Oscil <TRIANGLE_HERMES_2048_NUM_CELLS, AUDIO_RATE> Osc0(TRIANGLE_HERMES_2048_DATA);
Oscil <SAW2048_NUM_CELLS, AUDIO_RATE> Osc1(SAW2048_DATA);
Oscil <SAW2048_NUM_CELLS, AUDIO_RATE> Osc2(SAW2048_DATA);
Oscil <TRIANGLE_HERMES_2048_NUM_CELLS, AUDIO_RATE> Osc3(TRIANGLE_HERMES_2048_DATA);
Oscil <SAW2048_NUM_CELLS, AUDIO_RATE> Osc4(SAW2048_DATA);// SUPERCAZZOLA

//Oscil <ENVELOP2048_NUM_CELLS, AUDIO_RATE> Osc4(ENVELOP2048_DATA);

Oscil <SIN2048_NUM_CELLS, AUDIO_RATE> Carrier2(SIN2048_DATA); // portante per Ring Modulatin(RM)  Osc1 
Oscil <SIN2048_NUM_CELLS, AUDIO_RATE> Carrier1(SIN2048_DATA);
// Array di oscillatori per poterci iterare sopra
Oscil <2048, AUDIO_RATE> oscillatori[] = { Osc0, Osc1, Osc2, Osc3, Osc4 };

ADSR <AUDIO_RATE, AUDIO_RATE> envelope;
EventDelay noteDelay;

float Note_Pianta[20] = {La0*32,Si0*32,Do1*32,Re1*32,Mi1*32,Fa1*32,Sol1*32,La1*32,La0*16,Si0*16,Do1*16,Re1*16,Mi1*16,Fa1*16,Sol1*16,La1*16};

void setup() {
  //Serial.begin(9600);
  pinMode(P0_PIN,INPUT);
  pinMode(LEDM_PIN,OUTPUT);
  startMozzi(CONTROL_RATE); // :)
}

unsigned int duration, attack, decay, sustain, release_ms;

void updateControl() {
  // Vettore con tutti i valori letti dai potenziometri
  int letture[5] = { GEN_map(mozziAnalogRead(POT0_PIN)), GEN_map(mozziAnalogRead(POT1_PIN)), GEN_map(mozziAnalogRead(POT2_PIN)), GEN_map(mozziAnalogRead(POT3_PIN)), GEN_map(mozziAnalogRead(POT4_PIN))};
  

  
  // Questo è tutto un pippone per attivare e disattivare la modulazione con un pulsante evitando i rimbalzi
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
  // save the reading. Next time through the loop, it'll be the lastButtonState:
  lastButtonState = reading;
  // Pippone finito ;)

  //Serial.println(Switch_RM);
  int valori_discreti[5];    // Variabili per il calcolo delle note e della frequenza dei vari oscillatori
  float frequenze_base[5];
  
  // Discretizzazione delle frequenze iterate per ogni oscillatore 
  for (int i = 0; i < 5; i++) {
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
  float Note_Pianta[16] = {frequenze_base[0]*32,frequenze_base[1]*32,frequenze_base[2]*32,frequenze_base[3]*32,frequenze_base[0]*24,frequenze_base[1]*24,frequenze_base[2]*24,frequenze_base[3]*24,frequenze_base[0]*32,frequenze_base[1]*16,frequenze_base[2]*16,frequenze_base[3]*16,frequenze_base[0]*12,frequenze_base[1]*12,frequenze_base[2]*12,frequenze_base[3]*12};
  
  if(noteDelay.ready()){
      // choose envelope levels
      byte attack_level =rand(50) + 50;
      byte decay_level = 20;
      envelope.setADLevels(attack_level,decay_level);
      unsigned int new_value = rand(100) +300;
      attack = 5;
      decay = 50;
      sustain = 50;
      release_ms =new_value;
      envelope.setTimes(attack,decay,sustain,release_ms);
      envelope.noteOn();
      int j = rand(15);
      Osc4.setFreq(Note_Pianta[j]);
      int pianta = (mozziAnalogRead(A4)/(3+rand(5)))*rand(6);
      noteDelay.start(attack+decay+sustain+pianta);
  }

  Osc0.setFreq(frequenze_base[0]*2);
  Osc1.setFreq(frequenze_base[1]*4);
  Osc2.setFreq(frequenze_base[2]*8);  
  Osc3.setFreq(frequenze_base[3]*16);

  


  float Carrier2_Freq = MOD_map(GEN_map(mozziAnalogRead(POT5_PIN)));// leggo il potenziometro per determinare la frequenza aggiuntiva
  Carrier1.setFreq(frequenze_base[3]*48+Carrier2_Freq/8); // portante1: è un'ottava sotto il segnale modulato + la frequenza letta dal potenziometro(divido per 8 per avere più precisione)
  Carrier2.setFreq(frequenze_base[2]*24+Carrier2_Freq/10); // portante2: è alla stessa ottava del segnale modulato + la frequenza letta dal potenziometro(divido per 10 per avere una precisione diversa dall'altra portante)
  
  if(Switch_RM==true){  // Questo è un modo che ho trovato per fare oscillare il led a tempo con la modulazione
    if(Carrier1.next()<0){
      analogWrite(LEDM_PIN,0);
    }
    else analogWrite(LEDM_PIN,Carrier1.next());   
  }
  else analogWrite(LEDM_PIN,0);

}

//Osc.next() restituisce un intero che sarà il return della funzione audio_output, perciò sommo Osc.next() e scarto i n-oscilla MSB di overflow attraverso un bitshift (>>)
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
