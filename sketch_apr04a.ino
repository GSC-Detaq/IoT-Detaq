#include <MAX30100.h>


#include <WiFi.h>
#include <FirebaseESP32.h>
//#include <ESP32Ping.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>



// Set these to run example.
#define WIFI_SSID "ua"
#define WIFI_PASSWORD "bismillah"                                       //password WiFi
#define FIREBASE_HOST "https://dht11-firebase-ad-default-rtdb.firebaseio.com/"
#define FIREBASE_AUTH "1pLf65HzYzXQMHtjU7XkAnIldUGo4TB3qX07mabS"
#define BUTTON_PIN 16  // ESP32 GPIO16 pin connected to button's pin
#define BUZZER_PIN 21  // ESP32 GPIO21 pin connected to Buzzer's pin
#define PULSE_SENSOR_PIN A0 // Pulse Sensor connected to analog pin A0
#define REPORTING_PERIOD_MS     1000

FirebaseData firebaseData;
FirebaseJson json;

MAX30100 sensor;
//PulseOximeter pox;

uint32_t tsLastReport = 0;

// Callback (registered below) fired when a pulse is detected
void onBeatDetected()
{
    Serial.println("B:1");
}

int buzzerState = LOW;
int input_sblm = HIGH;
int tepi_naik = HIGH;
int pulse_threshold = 50;
int state = 0;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;  
int buttonState;
int lastButtonState = HIGH;
const IPAddress routerIp(192, 168, 0, 1);
String googlDotCom = "www.google.com";
bool sosSent = false;
int BPM_THRESHOLD = 50;
int currentState = 1;
int buttonPin;
int buzzerPin;

void setup() {
  Serial.begin(115200);  // initialize serial
  pinMode(BUTTON_PIN, INPUT_PULLUP); // set ESP32 pin to input pull-up mode
  pinMode(BUZZER_PIN, OUTPUT);  // set ESP32 pin to output mode
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("connecting");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("connected: ");
  Serial.println(WiFi.localIP());
  
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
}
void stateOne(int bpmValue) {
  if (bpmValue > BPM_THRESHOLD) {
    currentState = 2; // jika BPM melebihi threshold, maka pindah ke state 2
  }
  
  buttonState = digitalRead(BUTTON_PIN); // membaca status tombol darurat
  
  if (buttonState == HIGH && lastButtonState == LOW) {
    currentState = 3; // jika tombol darurat ditekan, maka pindah ke state 3
  }
  
  lastButtonState = buttonState; // menyimpan status tombol darurat sebelumnya
}

void stateTwo(int bpmValue) {
  if (bpmValue <= BPM_THRESHOLD) {
    buzzerState = false; // matikan buzzer jika BPM sudah turun di bawah threshold
    digitalWrite(BUZZER_PIN, LOW);
    currentState = 1; // pindah ke state 1
   Firebase.setString(firebaseData, "/user1/emergency", "Aman");
     
  }
  
  if (digitalRead(BUTTON_PIN) == HIGH) {
    buzzerState = false; // matikan buzzer jika tombol darurat ditekan
    digitalWrite(BUZZER_PIN, LOW);
    currentState = 3; // pindah ke state 3
     Firebase.setString(firebaseData, "/user1/emergency", "SOS");
    
  }
  
  if (buzzerState == false) {
    buzzerState = true; // nyalakan buzzer jika belum dinyalakan sebelumnya
    digitalWrite(BUZZER_PIN, HIGH);
     Firebase.setString(firebaseData, "/user1/emergency", "SOS");
       
    delay(1000);
  }
}
void stateThree(){
  int buttonState = digitalRead(buttonPin);

  // memeriksa apakah tombol ditekan
  if(buttonState == HIGH && lastButtonState == LOW && millis()-lastDebounceTime > debounceDelay) {
    lastDebounceTime = millis();
    
    // memeriksa status buzzer
    if(buzzerState == false) {
      buzzerState = true; // nyalakan buzzer
      digitalWrite(buzzerPin, HIGH);
       Firebase.setString(firebaseData, "/user1/emergency", "SOS");
    
    }
    else {
      buzzerState = false; // matikan buzzer
      digitalWrite(buzzerPin, LOW);
      currentState = 1; // kembali ke state 1
      Firebase.setString(firebaseData, "/user1/emergency", "Aman");
       
    }
  }

  lastButtonState = buttonState;
}

void loop() {
  // membaca BPM dari sensor max30100
 sensor.readSensor();

  int BPM = meanDiff(sensor.IR);
 

  // mengirim BPM ke Firebase
  Firebase.setString(firebaseData, "/user1/BPM", String(BPM,2));



  // menjalankan state machine
  switch (currentState) {
    case 1:
      stateOne(BPM);
      break;
    case 2:
      stateTwo(BPM);
      break;
    case 3:
      stateThree();
      break;
    default:
      break;
  }

  // delay selama 100ms untuk menghindari pengambilan data yang terlalu cepat
  delay(100);
}
