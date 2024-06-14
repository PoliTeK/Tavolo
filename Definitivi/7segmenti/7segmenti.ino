
/*
  Showing number 0-9 on a Common Anode 7-segment LED display
  Displays the numbers 0-9 on the display, with one second inbetween.
    A
   ---
F |   | B
  | G |
   ---
E |   | C
  |   |
   ---
    D
  This example code is in the public domain.
 */

#define N 5
 
// Pin 2-8 is connected  to the 7 segments of the display.

int pinA = 2;
int pinB = 3;
int pinC  = 4;
int pinD = 5;
int pinE = 6;
int pinF = 7;
int pinG = 8;
int  D1 = 9;
int D2 = 10;
int D3 = 11;
int D4 = 12;

// the setup routine  runs once when you press reset:
void setup() {                
  // initialize  the digital pins as outputs.
  pinMode(pinA, OUTPUT);     
  pinMode(pinB,  OUTPUT);     
  pinMode(pinC, OUTPUT);     
  pinMode(pinD, OUTPUT);     
  pinMode(pinE, OUTPUT);     
  pinMode(pinF, OUTPUT);     
  pinMode(pinG,  OUTPUT);   
  pinMode(D1, OUTPUT);  
  pinMode(D2, OUTPUT);  
  pinMode(D3,  OUTPUT);  
  pinMode(D4, OUTPUT);  
}

// the loop routine runs over and over again forever:
void loop() {

  switch (random (0,9)){
  
  case 0:
  digitalWrite(D1, LOW);
  digitalWrite(D2,  HIGH);
  digitalWrite(D3, HIGH);
  digitalWrite(D4, HIGH); 
  
  //0
  digitalWrite(pinA, HIGH);
  digitalWrite(pinB, HIGH);
  digitalWrite(pinC, HIGH);
  digitalWrite(pinD, HIGH);
  digitalWrite(pinE, HIGH);
  digitalWrite(pinF, HIGH);
  digitalWrite(pinG, LOW);

  delay(N);               // wait for  a second
  break;
  
  case 1:
  digitalWrite(D1, HIGH);
  digitalWrite(D2, LOW);
  digitalWrite(D3,  HIGH);
  digitalWrite(D4, HIGH); 
  //1
  digitalWrite(pinA, LOW);
  digitalWrite(pinB, HIGH);
  digitalWrite(pinC, HIGH);
  digitalWrite(pinD, LOW);
  digitalWrite(pinE, LOW);
  digitalWrite(pinF, LOW);
  digitalWrite(pinG, LOW);


  delay(N);               // wait for a second

  break;

  case 2:
  
  digitalWrite(D1, HIGH);
  digitalWrite(D2, HIGH);
  digitalWrite(D3,  LOW);
  digitalWrite(D4, HIGH); 
  //2
  digitalWrite(pinA, HIGH);
  digitalWrite(pinB, HIGH);
  digitalWrite(pinC, LOW);
  digitalWrite(pinD, HIGH);
  digitalWrite(pinE, HIGH);
  digitalWrite(pinF, LOW);
  digitalWrite(pinG, HIGH);


  delay(N);               // wait for a second

  break;

  case 3:

  digitalWrite(D1, HIGH);
  digitalWrite(D2, HIGH);
  digitalWrite(D3, HIGH);
  digitalWrite(D4, LOW); 
  //3
  digitalWrite(pinA, HIGH);
  digitalWrite(pinB, HIGH);
  digitalWrite(pinC, HIGH);
  digitalWrite(pinD, HIGH);
  digitalWrite(pinE, LOW);
  digitalWrite(pinF, LOW);
  digitalWrite(pinG, HIGH);

  delay(N);               // wait for a second

  break;

  case 4:

  digitalWrite(D1, LOW);
  digitalWrite(D2,  HIGH);
  digitalWrite(D3, HIGH);
  digitalWrite(D4, HIGH); 

  //4
  digitalWrite(pinA, LOW);
  digitalWrite(pinB, HIGH);
  digitalWrite(pinC, HIGH);
  digitalWrite(pinD, LOW);
  digitalWrite(pinE, LOW);
  digitalWrite(pinF, HIGH);
  digitalWrite(pinG, HIGH);
 
  delay(N);               // wait for a second

  break;

  case 5:
  
  digitalWrite(D1, HIGH);
  digitalWrite(D2,  LOW);
  digitalWrite(D3, HIGH);
  digitalWrite(D4, HIGH); 

  //5
  digitalWrite(pinA, HIGH);
  digitalWrite(pinB, LOW);
  digitalWrite(pinC, HIGH);
  digitalWrite(pinD, HIGH);
  digitalWrite(pinE, LOW);
  digitalWrite(pinF, HIGH);
  digitalWrite(pinG, HIGH);

   
  delay(N);               // wait for a second

  break;

  case 6:

  digitalWrite(D1, HIGH);
  digitalWrite(D2,  HIGH);
  digitalWrite(D3, LOW);
  digitalWrite(D4, HIGH); 
  
  //6
  digitalWrite(pinA, HIGH);
  digitalWrite(pinB, LOW);
  digitalWrite(pinC, HIGH);
  digitalWrite(pinD, HIGH);
  digitalWrite(pinE, HIGH);
  digitalWrite(pinF, HIGH);
  digitalWrite(pinG, HIGH);

  delay(N);               // wait for a second

  break;

  case 7:

  digitalWrite(D1, HIGH);
  digitalWrite(D2,  HIGH);
  digitalWrite(D3, HIGH);
  digitalWrite(D4, LOW); 
  
  //7
  digitalWrite(pinA, HIGH);
  digitalWrite(pinB, HIGH);
  digitalWrite(pinC, HIGH);
  digitalWrite(pinD, LOW);
  digitalWrite(pinE, LOW);
  digitalWrite(pinF, LOW);
  digitalWrite(pinG, LOW);
    
  delay(N);               // wait for a second

  break;

  case 8:

  digitalWrite(D1, LOW);
  digitalWrite(D2,  HIGH);
  digitalWrite(D3, HIGH);
  digitalWrite(D4, HIGH); 
  
  //8
  digitalWrite(pinA, HIGH);
  digitalWrite(pinB, HIGH);
  digitalWrite(pinC, HIGH);
  digitalWrite(pinD, HIGH);
  digitalWrite(pinE, HIGH);
  digitalWrite(pinF, HIGH);
  digitalWrite(pinG, HIGH);
     
  delay(N);               // wait for a second

  break;

  case 9:

  digitalWrite(D1, HIGH);
  digitalWrite(D2,  HIGH);
  digitalWrite(D3, LOW);
  digitalWrite(D4, HIGH); 

  //P
  digitalWrite(pinA, HIGH);
  digitalWrite(pinB, HIGH);
  digitalWrite(pinC, LOW);
  digitalWrite(pinD, LOW);
  digitalWrite(pinE, HIGH);
  digitalWrite(pinF, HIGH);
  digitalWrite(pinG, HIGH);

    
  delay(N);               // wait for a second

  break;

  default:
  digitalWrite(D1, LOW);
  digitalWrite(D2,  LOW);
  digitalWrite(D3, LOW);
  digitalWrite(D4, LOW); 

  //4
  digitalWrite(pinA, LOW);  
  digitalWrite(pinB, HIGH);  
  digitalWrite(pinC, HIGH);  
  digitalWrite(pinD, LOW);  
  digitalWrite(pinE, LOW);  
  digitalWrite(pinF, HIGH);  
  digitalWrite(pinG, HIGH);  
  break;

  }
}
