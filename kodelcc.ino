#include "pitches.h"
#define NOTE_DURATION 500  // Durasi nada dalam milidetik

#define SPEAKER_PIN 13
const int segmentPins[7] = {14, 12, 32, 33, 25, 27, 26}; // segA, segB, segC, segD, segE, segF, segG
const int digitPins[3] = {23, 22, 21}; // DIG1, DIG2, DIG3

const byte characterPatterns[13] = {
  B00111111, //0
  B00000110, //1
  B01011011, //2
  B01001111, //3
  B01100110, //4
  B01101101, //5
  B01111101, //6
  B00000111, //7
  B01111111, //8
  B01101111, //9
  B01110111, // A
  B01111100, // b
  B00111001 // C
};

const int buttonTimA = 5;
const int buttonTimB = 18;
const int buttonTimC = 19;
const int buttonStandby = 4;
const int buttonBenar = 16;
const int buttonSalah = 15;


bool standbyMode = false;
bool showABC = false;
int timPressed = 0;
int timPressedstb = 0;
unsigned long standbyStartTime = 0;
const unsigned long standbyDuration = 15000;

volatile bool standbyButtonChanged = false; 
volatile unsigned long lastInterruptTime = 0; 
unsigned long lastDebounceTimeStandby = 0;
const unsigned long debounceDelay = 50; 
int lastButtonStateStandby = LOW;  
unsigned long lastDebounceTimeBenar = 0;
int lastButtonStateBenar = LOW;
unsigned long lastDebounceTimeSalah = 0; 
int lastButtonStateSalah = LOW;  // Status terakhir tombol salah
volatile unsigned long lastStandbyPressTime = 0;  // Waktu terakhir tombol standby ditekan
const unsigned long doublePressInterval = 500;   // Interval 1/2 detik untuk deteksi double press
int standbyPressCount = 0;  // Hitung jumlah penekanan tombol standby
unsigned long displayDelayStartTime = 0;
bool displayDelayActive = false;
unsigned long toneStartTime = 0; // Waktu mulai tone
bool tonePlaying = false; // Status tone sedang dimainkan
unsigned long previousMillis = 0;
const int interval = 1; 
const unsigned long countdownStart = 5000;
bool gameResetRequested = false;
const int offTime = 1;

int skorA = 0;
int skorB = 0;
int skorC = 0;

int timAktif = -1;


void setup() {
  Serial.begin(9600);

  // Inisialisasi segment display
  for (int i = 0; i < 7; i++) {
    pinMode(segmentPins[i], OUTPUT);
    digitalWrite(segmentPins[i], HIGH); 
  }

  for (int i = 0; i < 3; i++) {
    pinMode(digitPins[i], OUTPUT);
    digitalWrite(digitPins[i], LOW); 
  }

  // Inisialisasi tombol
  pinMode(buttonTimA, INPUT_PULLUP);
  pinMode(buttonTimB, INPUT_PULLUP);
  pinMode(buttonTimC, INPUT_PULLUP);
  pinMode(buttonStandby, INPUT_PULLUP);
  pinMode(buttonBenar, INPUT_PULLUP);
  pinMode(buttonSalah, INPUT_PULLUP);

  // Attach interrupt untuk tombol standby
  attachInterrupt(digitalPinToInterrupt(buttonStandby), handleStandbyInterrupt, CHANGE);
}

void loop() {
  //Serial.println(timAktif);
  //Serial.println("nilai tim preess : ");
  //Serial.println(timPressedstb);

  if (!standbyMode) {
    displayCharacter(skorA, 0); 
    displayCharacter(skorB, 1); 
    displayCharacter(skorC, 2); 
  }

  if (standbyButtonChanged) {
    handleStandbyButton();
  }

  if (standbyMode && showABC) {
    displayCharacter(10, 0);
    displayCharacter(11, 1);
    displayCharacter(12, 2);
    
    // Cek tombol tim
    checkTimButtons();
    if (millis() - standbyStartTime >= standbyDuration - countdownStart) {
    playTickSound(); }
    if (millis() - standbyStartTime >= standbyDuration -800) {
      playAlarmRing();  
      resetToStandby();
    }
  }

  if (timPressedstb == 1) {
    if (timAktif == 0) {
      displayCharacter(10, 0);
    } else if (timAktif == 1) {
      displayCharacter(11, 1);
    } else if (timAktif == 2) {
      displayCharacter(12, 2);
    }
  }
  if (timPressed == 1) {
    if (timAktif == 0) {
      displayCharacter(10, 0);
    } else if (timAktif == 1) {
      displayCharacter(11, 1);
    } else if (timAktif == 2) {
      displayCharacter(12, 2);
    }
  }
    if (displayDelayActive) {
        displaySkor();  // Tetap tampilkan skor secara multiplexing
        if (millis() - displayDelayStartTime >= 2000) {
            // 2 detik telah berlalu, umumkan pemenang dan disable tombol
            if (skorA == 9) {
                announceWinner("A");
            } else if (skorB == 9) {
                announceWinner("B");
            } else if (skorC == 9) {
                announceWinner("C");
            }
            disableAllButtons();
            displayDelayActive = false;  // Reset setelah pengumuman
        }
    }
  
  if (tonePlaying && (millis() - toneStartTime >= NOTE_DURATION)) {
  noTone(SPEAKER_PIN);  // Matikan buzzer setelah durasi selesai
  tonePlaying = false;  // Reset status tonePlaying
  }

  if (timPressed == 0 && !showABC && timPressedstb==0) {
    displaySkor();  // Tampilkan skor terus menerus
  }

  checkBenarButton();
  checkSalahButton();
  checkSalahButton2();
  delay(10);
}


void checkTimButtons() {
  if (digitalRead(buttonTimA) == HIGH) {
    Serial.println("Tombol Tim A ditekan");
    timAktif = 0;  // Menyimpan status bahwa Tim A aktif
    timPressed = 1;  // Menandai bahwa tombol tim telah ditekan
    showABC = false;
    tone(SPEAKER_PIN, NOTE_C4, NOTE_DURATION);  // Mainkan nada C3 selama 500ms
    toneStartTime = millis();  // Catat waktu mulai
    tonePlaying = true;  // Matikan buzzer
  }
  else if (digitalRead(buttonTimB) == HIGH) {
    Serial.println("Tombol Tim B ditekan");
    timAktif = 1;  // Menyimpan status bahwa Tim B aktif
    timPressed = 1;
    showABC = false;
    tone(SPEAKER_PIN, NOTE_C4, NOTE_DURATION);  // Mainkan nada C3 selama 500ms
     toneStartTime = millis();  // Catat waktu mulai
    tonePlaying = true;  // Matikan buzzer
  }
  else if (digitalRead(buttonTimC) == HIGH) {
    Serial.println("Tombol Tim C ditekan");
    timAktif = 2;  // Menyimpan status bahwa Tim C aktif
    timPressed = 1;
    showABC = false;
    tone(SPEAKER_PIN, NOTE_C4, NOTE_DURATION);  // Mainkan nada C3 selama 500ms
    toneStartTime = millis();  // Catat waktu mulai
    tonePlaying = true;  // Matikan buzzer
  }
}

void checkBenarButton() {
    if (timPressed == 1 || timPressedstb == 1) {  // Hanya jika tim sudah dipilih
        int readingBenar = digitalRead(buttonBenar);
        if (readingBenar == HIGH && lastButtonStateBenar == LOW && (millis() - lastDebounceTimeBenar) > debounceDelay) {
            if (timAktif == 0) {
                Serial.println("Tombol benar A ditekan");
                skorA++;
                if (skorA == 9) {
                   displayCharacter2(skorA, 0);  
                   announceWinner("A"); 
                  
                }
            }
            else if (timAktif == 1) {
                Serial.println("Tombol benar B ditekan");
                skorB++;
                if (skorB == 9) {
                   displayCharacter2(skorB, 1);  
                   announceWinner("B"); 
                    // Aktifkan mode penundaan
                }
            }
            else if (timAktif == 2) {
                Serial.println("Tombol benar C ditekan");
                skorC++;
                if (skorC == 9) {
                   displayCharacter2(skorC, 2);  
                   announceWinner("C"); 
                        // Aktifkan mode penundaan
                }
            }
            displaySkor();
            timPressed = 0;
            timPressedstb = 0;
            playLevelUpSound() ; // Reset setelah tombol "Benar" ditekan
        }
        lastButtonStateBenar = readingBenar;
    }
}



void checkSalahButton() {
  if (timPressed==1) {  // Hanya jika tim sudah dipilih
    int readingSalah = digitalRead(buttonSalah);
    if (readingSalah == HIGH && lastButtonStateSalah == LOW && (millis() - lastDebounceTimeSalah) > debounceDelay) {
      if (timAktif == 0) {
        Serial.println("Tombol salah A ditekan");
        skorA--;
        if (skorA < 0) skorA = 0;  // Hindari skor negatif
        displayCharacter(skorA, 0);  // Tampilkan skor A di digit pertama
      }
      else if (timAktif == 1) {
        Serial.println("Tombol salah B ditekan");
        skorB--;
        if (skorB < 0) skorB = 0;
        displayCharacter(skorB, 1);  // Tampilkan skor B di digit kedua
      }
      else if (timAktif == 2) {
        Serial.println("Tombol salah C ditekan");
        skorC--;
        if (skorC < 0) skorC = 0;
        displayCharacter(skorC, 2);  // Tampilkan skor C di digit ketiga
      }

      // Memasuki mode standby untuk tim lain
      salah();
      delay(500);
      standbyModeForOthers();

      timPressed = 0;  // Reset setelah tombol salah ditekan
    }
    lastButtonStateSalah = readingSalah;
  // Update waktu debounce
  }
}
void checkSalahButton2() {
  if (timPressedstb==1) {  // Hanya jika tim sudah dipilih
    int readingSalah = digitalRead(buttonSalah);
    if (readingSalah == HIGH && lastButtonStateSalah == LOW && (millis() - lastDebounceTimeSalah) > debounceDelay) {
      if (timAktif == 0) {
        Serial.println("Tombol salah A ditekan");
        skorA--;
        if (skorA < 0) skorA = 0;  // Hindari skor negatif
        displayCharacter(skorA, 0);  // Tampilkan skor A di digit pertama
      }
      else if (timAktif == 1) {
        Serial.println("Tombol salah B ditekan");
        skorB--;
        if (skorB < 0) skorB = 0;
        displayCharacter(skorB, 1);  // Tampilkan skor B di digit kedua
      }
      else if (timAktif == 2) {
        Serial.println("Tombol salah C ditekan");
        skorC--;
        if (skorC < 0) skorC = 0;
        displayCharacter(skorC, 2);  // Tampilkan skor C di digit ketiga
      }

      // Memasuki mode standby untuk tim lain
      salah();
      timPressedstb = 0;  // Reset setelah tombol salah ditekan
    }
    lastButtonStateSalah = readingSalah;
  // Update waktu debounce
  }
}
void announceWinner(String team) {
    Serial.print("Tim ");
    Serial.print(team);
    Serial.println(" menang!");
    playVictorySound();

    // Mulai periode penundaan, tapi bisa dihentikan jika tombol Standby ditekan
    unsigned long startDelayTime = millis();
    gameResetRequested = false;

    // Penundaan dengan kemampuan untuk dibatalkan
    while (millis() - startDelayTime < 100000) {
        if (digitalRead(buttonStandby) == HIGH) {  // Cek jika tombol Standby ditekan
            gameResetRequested = true;
            break;  // Keluar dari loop untuk memulai ulang permainan
        }
    }

    // Jika tombol standby ditekan, lakukan reset permainan
    if (gameResetRequested) {
        resetGame();
    } else {
        disableAllButtons();  // Lanjutkan ke penghentian tombol jika tidak ada reset
    }
}

void resetGame() {
    Serial.println("Permainan di-reset!");

    skorA = 0;
    skorB = 0;
    skorC = 0;
    timAktif = -1;
    timPressed = 0;
    timPressedstb = 0;
    standbyMode = false;
    showABC = false;

    displaySkor();  // Tampilkan skor yang di-reset
}

void standbyModeForOthers() {
  unsigned long standbyStart = millis();
  unsigned long lastTickTime = standbyStart; // Inisialisasi waktu tick terakhir

  // Durasi standby selama 3 detik untuk tim yang tidak aktif
  while (millis() - standbyStart < 2200) {
    // Mainkan suara tick setiap detik
    if (millis() - lastTickTime >= 0) {
      playTickSound();        // Mainkan suara tick
      lastTickTime = millis(); // Update waktu tick terakhir
    }
    // Cek apakah tim selain timAktif menekan tombol
    if (timAktif != 0 && digitalRead(buttonTimA) == HIGH) {
      Serial.println("Tombol Tim A ditekan selama standby mode");
      timAktif = 0;
      timPressedstb=1;
    tone(SPEAKER_PIN, NOTE_C4, NOTE_DURATION);  // Mainkan nada C3 selama 500ms
    toneStartTime = millis();  // Catat waktu mulai
    tonePlaying = true;   // Matikan buzzer
     // Update variabel global, tidak perlu return
      
      return; // Tidak perlu mengembalikan nilai, cukup keluar dari fungsi
    }
    if (timAktif != 1 && digitalRead(buttonTimB) == HIGH) {
      Serial.println("Tombol Tim B ditekan selama standby mode");
      timAktif = 1;
      timPressedstb=1;
    tone(SPEAKER_PIN, NOTE_C4, NOTE_DURATION);  // Mainkan nada C3 selama 500ms
    toneStartTime = millis();  // Catat waktu mulai
    tonePlaying = true;  // Matikan buzzer
      return; // Keluar dari fungsi
    }
    if (timAktif != 2 && digitalRead(buttonTimC) == HIGH) {
      Serial.println("Tombol Tim C ditekan selama standby mode");
      timAktif = 2;
      timPressedstb=1;
    tone(SPEAKER_PIN, NOTE_C4, NOTE_DURATION);  // Mainkan nada C3 selama 500ms
    toneStartTime = millis();  // Catat waktu mulai
    tonePlaying = true;   // Matikan buzzer
      return; // Keluar dari fungsi
    }

    // Tampilkan skor dan huruf ABC selama standby
    if (timAktif == 0) {
      displayCharacter(skorA, 0);  // Tampilkan skor A
      displayCharacter(11, 1);  // Tampilkan "b" untuk Tim B
      displayCharacter(12, 2);  // Tampilkan "C" untuk Tim C
    } else if (timAktif == 1) {
      displayCharacter(10, 0);  // Tampilkan "A" untuk Tim A
      displayCharacter(skorB, 1);  // Tampilkan skor B
      displayCharacter(12, 2);  // Tampilkan "C" untuk Tim C
    } else if (timAktif == 2) {
      displayCharacter(10, 0);  // Tampilkan "A" untuk Tim A
      displayCharacter(11, 1);  // Tampilkan "b" untuk Tim B
      displayCharacter(skorC, 2);  // Tampilkan skor C
    }

    delay(10);  
  }
  
     playAlarmRing();
}



void handleStandbyButton() {
    unsigned long currentMillis = millis();

    // Debounce: pastikan tombol stabil dengan jarak waktu tertentu
    if ((currentMillis - lastDebounceTimeStandby) > debounceDelay) {
        int readingStandby = digitalRead(buttonStandby);

        if (readingStandby == HIGH && lastButtonStateStandby == LOW) {
            unsigned long pressInterval = currentMillis - lastStandbyPressTime;
            
            if (pressInterval <= doublePressInterval) {
                // Jika tombol ditekan dua kali dengan cepat
                standbyPressCount++;

                if (standbyPressCount == 2) {
                    // Menampilkan skor dan menonaktifkan tombol tim
                    Serial.println("Tombol Standby ditekan dua kali, menampilkan skor dan menonaktifkan tombol tim");
                    displaySkor();  // Tampilkan skor saat ini
                    standbyPressCount = 0;  // Reset hitungan penekanan
                    standbyMode = false;  // Keluar dari mode standby
                    showABC = false;
                    timPressed = false;  // Reset status tim
                }
            } else {
                standbyPressCount = 1;  // Reset hitungan penekanan jika interval terlalu lama

                // Mode standby normal: jika hanya satu kali ditekan
                if (!standbyMode) {
                    standbyMode = true;
                    showABC = true;
                    standbyStartTime = millis();
                    tonePlaying = true;
                    toneStartTime = millis();
                    Serial.println("Tombol Standby ditekan, masuk ke mode standby normal");
                } else {
                    Serial.println("Tombol Standby ditekan lagi, reset ke mode standby");
                    timPressed = false;  // Reset status tim
                    showABC = true;
                    standbyStartTime = millis();
                    tonePlaying = true;
                    toneStartTime = millis();
                }
            }

            lastStandbyPressTime = currentMillis;  // Catat waktu penekanan terakhir
        }

        lastButtonStateStandby = readingStandby;
        standbyButtonChanged = false;  // Reset flag setelah tombol ditangani
    }
        if (tonePlaying) {
        playStartSound();
    }
}



void resetToStandby() {
  Serial.println("Waktu habis, kembali ke mode awal");
  standbyMode = false;
  showABC = false;
  timPressed = 0;
}

void displaySkor() {
  displayCharacter(skorA, 0);  // Tampilkan skor A di digit 1
 
  displayCharacter(skorB, 1);  // Tampilkan skor B di digit 2
 
  displayCharacter(skorC, 2);  // Tampilkan skor C di digit 3
  
}

// Fungsi untuk menampilkan karakter pada posisi digit tertentu
void displayCharacter(int charIndex, int position) {
  // Matikan semua digit terlebih dahulu
  for (int i = 0; i < 3; i++) {
    digitalWrite(digitPins[i], LOW);
  }

  // Atur pola segment ke LOW untuk karakter yang dipilih
  for (int i = 0; i < 7; i++) {
    digitalWrite(segmentPins[i], (characterPatterns[charIndex] >> i) & 0x01 ? LOW : HIGH); // Common Anode
  }

  // Aktifkan transistor untuk digit yang dipilih
  digitalWrite(digitPins[position], HIGH);

  // Tunggu hingga interval berakhir untuk mempertahankan kecerahan
  delayMicroseconds(interval * 1000);

  // Matikan semua digit sebentar untuk meratakan kecerahan
  digitalWrite(digitPins[position], LOW);
  delayMicroseconds(offTime * 1000);  // Waktu mati singkat
}
void displayCharacter2(int charIndex, int position) {
  // Matikan semua digit terlebih dahulu
  for (int i = 0; i < 3; i++) {
    digitalWrite(digitPins[i], LOW);
  }

  // Tampilkan karakter sesuai pattern di digit yang dipilih
  for (int i = 0; i < 7; i++) {
    digitalWrite(segmentPins[i], (characterPatterns[charIndex] >> i) & 0x01 ? LOW : HIGH);
  }

  // Aktifkan hanya satu digit yang diinginkan
  digitalWrite(digitPins[position], HIGH);
}

void disableAllButtons() {
    if (!gameResetRequested) {
        Serial.println("Program dihentikan. Reset untuk memulai ulang.");
        while (true) {
            // Loop infinitif hanya jika gameResetRequested tidak aktif
        }
    }
}

void playAlarmRing() {
  unsigned long startTime = millis();
  while (millis() - startTime < 1000) { // Durasi total 1 detik
    tone(SPEAKER_PIN, NOTE_C5); // Nada rendah (A4) untuk kesan jam beker
    delay(100);                 // Durasi pendek untuk suara "beep"
    noTone(SPEAKER_PIN);
    delay(100);                 // Jeda pendek di antara beep
    
    tone(SPEAKER_PIN, NOTE_C5); // Ulangi nada yang sama untuk "beep-beep"
    delay(100);
    noTone(SPEAKER_PIN);
    delay(100);
  }
}
void playLevelUpSound() {
  tone(SPEAKER_PIN, NOTE_E4, 150);
  delay(150);
  tone(SPEAKER_PIN, NOTE_G4, 150);
  delay(150);
  tone(SPEAKER_PIN, NOTE_B4, 150);
  delay(150);
  tone(SPEAKER_PIN, NOTE_E5, 300);
  delay(300);
  noTone(SPEAKER_PIN);
}
void playTickSound() {
  static unsigned long lastTickTime = 0;    // Untuk mengontrol jeda antara suara detik
  const unsigned long tickInterval = 1000;  // Interval detik 1 detik

  if (millis() - lastTickTime >= tickInterval) {
    tone(SPEAKER_PIN, NOTE_C4, 100); // Mainkan nada singkat untuk suara "tick"
    lastTickTime = millis(); // Atur waktu terakhir detik
  }
}
#include <pitches.h>

void salah() {
  tone(SPEAKER_PIN, NOTE_DS5);
  delay(300);
  tone(SPEAKER_PIN, NOTE_D5);
  delay(300);
  tone(SPEAKER_PIN, NOTE_CS5);
  delay(300);
  for (byte i = 0; i < 10; i++) {
    for (int pitch = -10; pitch <= 10; pitch++) {
      tone(SPEAKER_PIN, NOTE_C5 + pitch);
      delay(5);
    }
  }
  noTone(SPEAKER_PIN);
 
}


void playStartSound() {
    const int soundDuration = 1000; // Durasi suara total dalam milidetik (1 detik)

    if (millis() - toneStartTime < soundDuration) {
        // Memainkan nada secara berurutan untuk menghasilkan efek "cringg"
 
            tone(SPEAKER_PIN, NOTE_E5,200); // Nada awal

            tone(SPEAKER_PIN, NOTE_G5,200); // Nada lebih tinggi
  
            tone(SPEAKER_PIN, NOTE_B5,200); // Nada lebih tinggi lagi

            tone(SPEAKER_PIN, NOTE_E6,200); // Nada akhir untuk memberi kesan "cringg"
    
            tone(SPEAKER_PIN, NOTE_G6,200); // Nada terakhir yang berkelanjutan
        
    } else {
        noTone(SPEAKER_PIN);       // Matikan suara setelah 1 detik
        tonePlaying = false;       // Reset flag setelah selesai
    }
}
#include <pitches.h>

void playVictorySound() {
  // Nada pembuka yang megah
  tone(SPEAKER_PIN, NOTE_G5, 200);  // Nada G5 selama 200ms
  delay(200);

  tone(SPEAKER_PIN, NOTE_C6, 200);  // Nada C6 selama 200ms
  delay(200);

  tone(SPEAKER_PIN, NOTE_E6, 200);  // Nada E6 selama 200ms
  delay(200);

  // Nada klimaks
  tone(SPEAKER_PIN, NOTE_F6, 300);  // Nada G6 selama 300ms
  delay(300);

  // Nada turun untuk menyelesaikan efek kemenangan
  tone(SPEAKER_PIN, NOTE_E6, 150);  // Nada E6 selama 150ms
  delay(150);

  tone(SPEAKER_PIN, NOTE_C6, 150);  // Nada C6 selama 150ms
  delay(150);

  tone(SPEAKER_PIN, NOTE_G5, 400);  // Nada G5 lebih panjang untuk penutup selama 400ms
  delay(400);

  noTone(SPEAKER_PIN); // Matikan suara setelah selesai
}


// ISR untuk menangani interrupt dari tombol standby
void handleStandbyInterrupt() {
  unsigned long interruptTime = millis();
  // Cek debounce: abaikan interrupt jika terjadi terlalu cepat
  if (interruptTime - lastInterruptTime > debounceDelay) {
    standbyButtonChanged = true;  // Tandai bahwa tombol standby berubah
    lastInterruptTime = interruptTime;  // Catat waktu interrupt terakhir
  }
}
