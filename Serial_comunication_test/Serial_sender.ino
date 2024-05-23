#include <MozziGuts.h>

#define A0_PIN A0
#define A1_PIN A1
#define A2_PIN A2
#define A3_PIN A3
#define A4_PIN A4
#define A5_PIN A5
#define A6_PIN A6
#define A7_PIN A7
#define A8_PIN A8
#define A9_PIN A9
#define A10_PIN A10
#define A11_PIN A11
#define A12_PIN A12
#define A13_PIN A13
#define A14_PIN A14
#define A15_PIN A15
#define A16_PIN A16

int A0_Val,A1_Val,A2_Val,A3_Val,A4_Val,A5_Val,A6_Val,A7_Val,A8_Val,A9_Val,A10_Val,A11_Val,A12_Val,A13_Val,A14_Val,A15_Val=23;
char buff[512];


void setup() {
  Serial.begin(115200);
  
}



    
void loop(){
  A0_Val=analogRead(A0_PIN);
  A1_Val=analogRead(A1_PIN);
  A2_Val=analogRead(A2_PIN);
  A3_Val=analogRead(A3_PIN);
  A4_Val=analogRead(A4_PIN);
  sprintf(buff,"00:%04d;01:%04d;02:%04d;03:%04d;04:%04d;", A0_Val,A1_Val,A2_Val,A3_Val,A4_Val);
  Serial.print(buff); 
  delay(500);

}
