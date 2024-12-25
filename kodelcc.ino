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
int lastButtonStateSalah = LOW;  
volatile unsigned long lastStandbyPressTime = 0; 
const unsigned long doublePressInterval = 500;   
int standbyPressCount = 0;  
unsigned long displayDelayStartTime = 0;
bool displayDelayActive = false;
unsigned long toneStartTime = 0;
bool tonePlaying = false;
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

  for (int i = 0; i < 7; i++) {
    pinMode(segmentPins[i], OUTPUT);
    digitalWrite(segmentPins[i], HIGH); 
  }

  for (int i = 0; i < 3; i++) {
    pinMode(digitPins[i], OUTPUT);
    digitalWrite(digitPins[i], LOW); 
  }

  pinMode(buttonTimA, INPUT_PULLUP);
  pinMode(buttonTimB, INPUT_PULLUP);
  pinMode(buttonTimC, INPUT_PULLUP);
  pinMode(buttonStandby, INPUT_PULLUP);
  pinMode(buttonBenar, INPUT_PULLUP);
  pinMode(buttonSalah, INPUT_PULLUP);

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
        displaySkor();  
        if (millis() - displayDelayStartTime >= 2000) {
            if (skorA == 9) {
                announceWinner("A");
            } else if (skorB == 9) {
                announceWinner("B");
            } else if (skorC == 9) {
                announceWinner("C");
            }
            disableAllButtons();
            displayDelayActive = false;  
        }
    }
  
  if (tonePlaying && (millis() - toneStartTime >= NOTE_DURATION)) {
  noTone(SPEAKER_PIN);  
  tonePlaying = false; 
  }

  if (timPressed == 0 && !showABC && timPressedstb==0) {
    displaySkor();  
  }

  checkBenarButton();
  checkSalahButton();
  checkSalahButton2();
  delay(10);
}


void checkTimButtons() {
  if (digitalRead(buttonTimA) == HIGH) {
    Serial.println("Tombol Tim A ditekan");
    timAktif = 0;  
    timPressed = 1;  
    showABC = false;
    tone(SPEAKER_PIN, NOTE_C4, NOTE_DURATION); 
    toneStartTime = millis(); 
    tonePlaying = true;  
  }
  else if (digitalRead(buttonTimB) == HIGH) {
    Serial.println("Tombol Tim B ditekan");
    timAktif = 1;  
    timPressed = 1;
    showABC = false;
    tone(SPEAKER_PIN, NOTE_C4, NOTE_DURATION); 
     toneStartTime = millis();  
    tonePlaying = true; 
  }
  else if (digitalRead(buttonTimC) == HIGH) {
    Serial.println("Tombol Tim C ditekan");
    timAktif = 2;  
    timPressed = 1;
    showABC = false;
    tone(SPEAKER_PIN, NOTE_C4, NOTE_DURATION);  
    toneStartTime = millis();  
    tonePlaying = true;  
  }
}

void checkBenarButton() {
    if (timPressed == 1 || timPressedstb == 1) {  
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
                }
            }
            else if (timAktif == 2) {
                Serial.println("Tombol benar C ditekan");
                skorC++;
                if (skorC == 9) {
                   displayCharacter2(skorC, 2);  
                   announceWinner("C"); 
                }
            }
            displaySkor();
            timPressed = 0;
            timPressedstb = 0;
            playLevelUpSound() ; 
        }
        lastButtonStateBenar = readingBenar;
    }
}



void checkSalahButton() {
  if (timPressed==1) { 
    int readingSalah = digitalRead(buttonSalah);
    if (readingSalah == HIGH && lastButtonStateSalah == LOW && (millis() - lastDebounceTimeSalah) > debounceDelay) {
      if (timAktif == 0) {
        Serial.println("Tombol salah A ditekan");
        skorA--;
        if (skorA < 0) skorA = 0;  
        displayCharacter(skorA, 0);  
      }
      else if (timAktif == 1) {
        Serial.println("Tombol salah B ditekan");
        skorB--;
        if (skorB < 0) skorB = 0;
        displayCharacter(skorB, 1);  
      }
      else if (timAktif == 2) {
        Serial.println("Tombol salah C ditekan");
        skorC--;
        if (skorC < 0) skorC = 0;
        displayCharacter(skorC, 2);  
      }

      salah();
      delay(500);
      standbyModeForOthers();

      timPressed = 0;  
    }
    lastButtonStateSalah = readingSalah;
  }
}
void checkSalahButton2() {
  if (timPressedstb==1) {  
    int readingSalah = digitalRead(buttonSalah);
    if (readingSalah == HIGH && lastButtonStateSalah == LOW && (millis() - lastDebounceTimeSalah) > debounceDelay) {
      if (timAktif == 0) {
        Serial.println("Tombol salah A ditekan");
        skorA--;
        if (skorA < 0) skorA = 0;  
        displayCharacter(skorA, 0); 
      }
      else if (timAktif == 1) {
        Serial.println("Tombol salah B ditekan");
        skorB--;
        if (skorB < 0) skorB = 0;
        displayCharacter(skorB, 1);  
      }
      else if (timAktif == 2) {
        Serial.println("Tombol salah C ditekan");
        skorC--;
        if (skorC < 0) skorC = 0;
        displayCharacter(skorC, 2);  
      }

      salah();
      timPressedstb = 0;  
    }
    lastButtonStateSalah = readingSalah;
  }
}
void announceWinner(String team) {
    Serial.print("Tim ");
    Serial.print(team);
    Serial.println(" menang!");
    playVictorySound();

    unsigned long startDelayTime = millis();
    gameResetRequested = false;

    while (millis() - startDelayTime < 100000) {
        if (digitalRead(buttonStandby) == HIGH) {
            gameResetRequested = true;
            break;  
        }
    }

    if (gameResetRequested) {
        resetGame();
    } else {
        disableAllButtons();  
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

    displaySkor();  
}

void standbyModeForOthers() {
  unsigned long standbyStart = millis();
  unsigned long lastTickTime = standbyStart; 

  while (millis() - standbyStart < 2200) {
    if (millis() - lastTickTime >= 0) {
      playTickSound();        
      lastTickTime = millis(); 
    }
    if (timAktif != 0 && digitalRead(buttonTimA) == HIGH) {
      Serial.println("Tombol Tim A ditekan selama standby mode");
      timAktif = 0;
      timPressedstb=1;
    tone(SPEAKER_PIN, NOTE_C4, NOTE_DURATION); 
    toneStartTime = millis();  
    tonePlaying = true;  
      
      return; 
    }
    if (timAktif != 1 && digitalRead(buttonTimB) == HIGH) {
      Serial.println("Tombol Tim B ditekan selama standby mode");
      timAktif = 1;
      timPressedstb=1;
    tone(SPEAKER_PIN, NOTE_C4, NOTE_DURATION);  
    toneStartTime = millis(); 
    tonePlaying = true; 
      return; 
    }
    if (timAktif != 2 && digitalRead(buttonTimC) == HIGH) {
      Serial.println("Tombol Tim C ditekan selama standby mode");
      timAktif = 2;
      timPressedstb=1;
    tone(SPEAKER_PIN, NOTE_C4, NOTE_DURATION);  
    toneStartTime = millis(); 
    tonePlaying = true;  
      return; 
    }

    if (timAktif == 0) {
      displayCharacter(skorA, 0);  
      displayCharacter(11, 1);  
      displayCharacter(12, 2); 
    } else if (timAktif == 1) {
      displayCharacter(10, 0); 
      displayCharacter(skorB, 1);  
      displayCharacter(12, 2);  
    } else if (timAktif == 2) {
      displayCharacter(10, 0);  
      displayCharacter(11, 1); 
      displayCharacter(skorC, 2); 
    }

    delay(10);  
  }
  
     playAlarmRing();
}



void handleStandbyButton() {
    unsigned long currentMillis = millis();

    if ((currentMillis - lastDebounceTimeStandby) > debounceDelay) {
        int readingStandby = digitalRead(buttonStandby);

        if (readingStandby == HIGH && lastButtonStateStandby == LOW) {
            unsigned long pressInterval = currentMillis - lastStandbyPressTime;
            
            if (pressInterval <= doublePressInterval) {
                standbyPressCount++;

                if (standbyPressCount == 2) {
                    Serial.println("Tombol Standby ditekan dua kali, menampilkan skor dan menonaktifkan tombol tim");
                    displaySkor();  
                    standbyPressCount = 0;
                    standbyMode = false; 
                    showABC = false;
                    timPressed = false;  
                }
            } else {
                standbyPressCount = 1;  

                if (!standbyMode) {
                    standbyMode = true;
                    showABC = true;
                    standbyStartTime = millis();
                    tonePlaying = true;
                    toneStartTime = millis();
                    Serial.println("Tombol Standby ditekan, masuk ke mode standby normal");
                } else {
                    Serial.println("Tombol Standby ditekan lagi, reset ke mode standby");
                    timPressed = false; 
                    showABC = true;
                    standbyStartTime = millis();
                    tonePlaying = true;
                    toneStartTime = millis();
                }
            }

            lastStandbyPressTime = currentMillis;  
        }

        lastButtonStateStandby = readingStandby;
        standbyButtonChanged = false;  
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
  displayCharacter(skorA, 0);  
  displayCharacter(skorB, 1);  
  displayCharacter(skorC, 2);  
}

void displayCharacter(int charIndex, int position) {
  for (int i = 0; i < 3; i++) {
    digitalWrite(digitPins[i], LOW);
  }

  for (int i = 0; i < 7; i++) {
    digitalWrite(segmentPins[i], (characterPatterns[charIndex] >> i) & 0x01 ? LOW : HIGH); 
  }

  digitalWrite(digitPins[position], HIGH);

  delayMicroseconds(interval * 1000);

  digitalWrite(digitPins[position], LOW);
  delayMicroseconds(offTime * 1000);  
}
void displayCharacter2(int charIndex, int position) {
  for (int i = 0; i < 3; i++) {
    digitalWrite(digitPins[i], LOW);
  }

  for (int i = 0; i < 7; i++) {
    digitalWrite(segmentPins[i], (characterPatterns[charIndex] >> i) & 0x01 ? LOW : HIGH);
  }

  digitalWrite(digitPins[position], HIGH);
}

void disableAllButtons() {
    if (!gameResetRequested) {
        Serial.println("Program dihentikan. Reset untuk memulai ulang.");
        while (true) {
        }
    }
}

void playAlarmRing() {
  unsigned long startTime = millis();
  while (millis() - startTime < 1000) { 
    tone(SPEAKER_PIN, NOTE_C5); 
    delay(100);                 
    noTone(SPEAKER_PIN);
    delay(100);                
    tone(SPEAKER_PIN, NOTE_C5); 
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
  static unsigned long lastTickTime = 0;   
  const unsigned long tickInterval = 1000;  

  if (millis() - lastTickTime >= tickInterval) {
    tone(SPEAKER_PIN, NOTE_C4, 100); 
    lastTickTime = millis(); 
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
    const int soundDuration = 1000; 

    if (millis() - toneStartTime < soundDuration) {
 
            tone(SPEAKER_PIN, NOTE_E5,200); 

            tone(SPEAKER_PIN, NOTE_G5,200); 
  
            tone(SPEAKER_PIN, NOTE_B5,200); 

            tone(SPEAKER_PIN, NOTE_E6,200); 
    
            tone(SPEAKER_PIN, NOTE_G6,200); 
        
    } else {
        noTone(SPEAKER_PIN);      
        tonePlaying = false;      
    }
}
#include <pitches.h>

void playVictorySound() {
  tone(SPEAKER_PIN, NOTE_G5, 200);  
  delay(200);

  tone(SPEAKER_PIN, NOTE_C6, 200);  
  delay(200);

  tone(SPEAKER_PIN, NOTE_E6, 200);  
  delay(200);

  tone(SPEAKER_PIN, NOTE_F6, 300);  
  delay(300);

  tone(SPEAKER_PIN, NOTE_E6, 150);  
  delay(150);

  tone(SPEAKER_PIN, NOTE_C6, 150);  
  delay(150);

  tone(SPEAKER_PIN, NOTE_G5, 400);  
  delay(400);

  noTone(SPEAKER_PIN); 
}

void handleStandbyInterrupt() {
  unsigned long interruptTime = millis();
  if (interruptTime - lastInterruptTime > debounceDelay) {
    standbyButtonChanged = true; 
    lastInterruptTime = interruptTime; 
  }
}
