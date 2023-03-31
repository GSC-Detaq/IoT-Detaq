#include <CircularBuffer.h>
#include <MAX30100.h>
#include <MAX30100_BeatDetector.h>
#include <MAX30100_Filters.h>
#include <MAX30100_PulseOximeter.h>
#include <MAX30100_Registers.h>
#include <MAX30100_SpO2Calculator.h>

#include <Adafruit_FeatherOLED.h>
#include <Adafruit_FeatherOLED_SH110X.h>
#include <Adafruit_FeatherOLED_SH110X_WiFi.h>
#include <Adafruit_FeatherOLED_WiFi.h>


#include <WiFi.h>
#include <FirebaseESP32.h>


// Set these to run example.
#define WIFI_SSID "ua"
#define WIFI_PASSWORD "bismillah"                                       //password WiFi
#define FIREBASE_HOST "smartwatchgsc-default-rtdb.firebaseio.com/"       //link database Firebase
#define FIREBASE_Authorization_key "AIzaSyCIv84rKsxx-B98kOerTkkSm7VWzY1Qz8I"


FirebaseData firebaseData;
FirebaseJson json;

MAX30100 sensor;

#define BPM_THRESHOLD 100 //nilai threshold BPM
#define BUTTON_PIN 13 //pin pushbutton



//Inisialisasi objek OLED
//U8G2_SSD1306_128X64_NONAME_F_HW_I2C display(U8G2_R0);

//Inisialisasi objek MAX30100
//MAX30100_PulseOximeter MAX30100;
 //pox.setIRLedCurrent(MAX30100_LED_CURR_7_6MA);
// Create a PulseOximeter object
PulseOximeter pox;

// Time at which the last beat occurred
uint32_t tsLastReport = 0;

// Callback routine is executed when a pulse is detected
void onBeatDetected() {
    Serial.println("♥ Beat!");
    bpmValue = pox.getHeartRate();
}


//Deklarasi variabel state
enum State {
  STATE_1,
  STATE_2,
  STATE_3
};

State current_state = STATE_1; //menginisialisasi state awal sebagai STATE_1

//Variabel untuk menyimpan nilai BPM
int bpmValue = 0;

//Variabel untuk mengontrol buzzer
int buzzerPin = 12;

//Kondisi saat tombol ditekan
bool buttonPressed = false;

//Fungsi untuk membunyikan buzzer dengan frekuensi 1kHz dan durasi 1 detik
void buzzerOn() {
  digitalWrite(buzzerPin, HIGH);
  delay(500);
  digitalWrite(buzzerPin, LOW);
  delay(500);
}

//Fungsi untuk mengirim pesan darurat ke Firebase
void sendEmergency() {
Firebase.setString(firebaseData, "/user1/emergency", "SOS");
}

//Fungsi untuk mengirim pesan aman ke Firebase
void sendSafe() {
 Firebase.setString(firebaseData, "/user1/emergency", "AMAN");
}

//Fungsi untuk memproses input
void process_input(int input) {
    switch(current_state) {
        case STATE_1:
            // membaca BPM dari sensor max30100
            bpmValue = pox.getHeartRate();
            // mengirim BPM ke Firebase
            Firebase.setFLoat("BPM", bpmValue);
            // menampilkan BPM pada OLED
            display.clear();
            display.drawString(0, 0, "BPM: " + String(bpmValue));
            display.display();
            
            if(bpmValue > BPM_THRESHOLD) {
              current_state = STATE_2;
            }
            else if(digitalRead(BUTTON_PIN) == HIGH) {
              current_state = STATE_3;
            }
            break;
        case STATE_2:
            //Kondisi ketika BPM melebihi nilai threshold
            stateTwo(bpmValue);
            break;
        case STATE_3:
            //Kondisi ketika tombol darurat ditekan
            stateThree();
            break;
        default:
            current_state = STATE_1;
            break;
    }
}

//Fungsi untuk membaca input dari pushbutton
int get_input() {
    if(digitalRead(BUTTON_PIN) == HIGH) {
        if(!buttonPressed) {
            buttonPressed = true;
            return 1;
        }
    } else {
        buttonPressed = false;
    }
    return 0;
}
void stateTwo(int bpmValue) {
  // Membunyikan buzzer dengan frekuensi 1000 Hz
  tone(BUZZER_PIN, 1000);
  // Mengirim string "SOS" ke Firebase
  Firebase.setString(firebaseData, "/user1/emergency", "SOS");
  
  // Menunggu selama 2 detik atau sampai tombol ditekan
  unsigned long start_time = millis();
  while(millis() - start_time < 2000) {
    if(digitalRead(BUTTON_PIN) == HIGH) {
      current_state = STATE_1;
      noTone(BUZZER_PIN);
      return;
    }
    delay(10);
  }

  // Mengubah state menjadi STATE_1 jika tombol tidak ditekan
  current_state = STATE_1;
  noTone(BUZZER_PIN);
}
void stateThree() {
  //Kondisi ketika tombol darurat ditekan
  if(buttonState == LOW) {
    if(buzzerState == LOW) {
      digitalWrite(BUZZER_PIN, HIGH);
      Firebase.setString(firebaseData, "/user1/emergency", "SOS");
      buzzerState = HIGH;
    }
    else {
      digitalWrite(BUZZER_PIN, LOW);
      Firebase.setString(firebaseData, "/user1/emergency", "AMAN");
      buzzerState = LOW;
      current_state = STATE_1;
    }
    delay(500);
  }
}
