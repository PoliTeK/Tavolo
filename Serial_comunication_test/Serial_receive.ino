/* Arduino ricevente */
#define Start_char 'S'
#define Addr_char 'A'
#define Val_char 'V'
#define Stop_char 'F' 

int address = 0;
int value = 0;
int spacer;

void setup() {
  Serial.begin(115200);
}





void loop() {
  /*
    {
      data = Serial.read();
      char state;
      switch(data) {
        case Addr_char {
            state = '5';
            break;
        }
        case Val_char {
            state = 'V3';
            break;
        }
        default {
            state = 'D';
            break;
        }
      }
      switch(state) {
        case 'A1' {
          state = "A2"
        }
      }
        address = Serial.read();        
        Serial.print("address: ");
        Serial.println(address);
    }
    */

    while (Serial.available() < 1) {}
    address = (Serial.read() - 48) * 10;
    while (Serial.available() < 1) {}
    address += (Serial.read() - 48);

    while (Serial.available() < 1) {}
    spacer = Serial.read();
    
    while (Serial.available() < 1) {}
    value = (Serial.read() - 48) * 1000;
    while (Serial.available() < 1) {}
    value += (Serial.read() - 48) * 100;
    while (Serial.available() < 1) {}
    value += (Serial.read() - 48) * 10;
    while (Serial.available() < 1) {}
    value += (Serial.read() - 48) * 1;

    while (Serial.available() < 1) {}
    spacer = Serial.read();

    Serial.println(address);
    Serial.println(value);
    delay(50);
    /*
    if (Serial.available() > 0) {
      address = Serial.read() - 48;        
      Serial.print("address: ");
      Serial.println(address);
    }
    */
}
