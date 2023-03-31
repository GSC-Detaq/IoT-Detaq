
#define BUTTON_PIN 16  // ESP32 GPIO16 pin connected to button's pin
#define BUZZER_PIN 21  // ESP32 GPIO21 pin connected to Buzzer's pin
#define PULSE_SENSOR_PIN A0 // Pulse Sensor connected to analog pin A0

int buzzer_state = LOW;
int input_sblm = HIGH;
int tepi_naik = HIGH;
int pulse_threshold = 50;
int state = 0;

void setup() {
  Serial.begin(9600);  // initialize serial
  pinMode(BUTTON_PIN, INPUT_PULLUP); // set ESP32 pin to input pull-up mode
  pinMode(BUZZER_PIN, OUTPUT);  // set ESP32 pin to output mode
}

void loop() {
  int input = digitalRead(BUTTON_PIN); // read new state
  

  switch(state){
    case 0: // Kasus pertama
      if(input == LOW && input_sblm == HIGH){
        input_sblm = input;
      }
      else if(input == HIGH && input_sblm == LOW){
        input_sblm = input;
        tepi_naik = LOW;
        buzzer_state = !buzzer_state;
      }
      if(tepi_naik==LOW){
        digitalWrite(BUZZER_PIN,buzzer_state);
      }
      tepi_naik = HIGH;
      break;

    case 1: // Kasus kedua
      int pulse_signal = analogRead(PULSE_SENSOR_PIN);
       Serial.println(pulse_signal);  // Print the value to Serial Monitor
      if(pulse_signal > pulse_threshold){
        digitalWrite(BUZZER_PIN, HIGH);
      }
      else{
        digitalWrite(BUZZER_PIN, LOW);
      }
      if(input == LOW){
        digitalWrite(BUZZER_PIN, LOW);
      }
      break;
  }

  if(input == LOW && state == 0){
    state = 1;
  }
  else if(input == LOW && state == 1){
    state = 0;
  }
}
