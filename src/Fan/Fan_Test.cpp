#include <Arduino.h>

// -----------------------------
// PIN-KONFIGURATION
// -----------------------------
const int fanPwmPin = 13;   // GPIO13 → PWM-Ausgang über Transistor
const int tachPin    = 12;  // GPIO12 → Tachosignal vom Lüfter

// -----------------------------
// RPM-MESSUNG
// -----------------------------
volatile int pulseCount    = 0;
volatile unsigned long lastPulse = 0;

void IRAM_ATTR countPulse() {
  unsigned long now = micros();
  if (now - lastPulse > 1000) {   // 1 ms Debounce
    pulseCount++;
    lastPulse = now;
  }
}

// -----------------------------
// Tabellen und Konstanten
// -----------------------------
const int   N = 11;
const float rpmTable[N]  = {   0,   780,  1140,  1440,  1740,  2730,  6720,  9960, 12930, 14520, 12570 };
const int   dutyTable[N] = { 255,   230,   204,   179,   153,   128,   102,    77,    51,    26,     0 };
const float maxRpm = 14520.0;    // Referenz-Maximum aus Messung

// Laufvariablen
int currentPercent = 0;          
unsigned long lastRpmTime = 0;

// -----------------------------
// Funktion: Duty per Interpolation
// -----------------------------
int getInterpolatedDuty(int percent) {
  float desiredRpm = percent / 100.0 * maxRpm;

  // unterhalb Minimum
  if (desiredRpm <= rpmTable[0]) return dutyTable[0];
  // oberhalb Maximum
  if (desiredRpm >= rpmTable[N-1]) return dutyTable[N-1];

  // Segment suchen und interpolieren
  for (int i = 0; i < N - 1; i++) {
    if (desiredRpm <= rpmTable[i+1]) {
      float frac = (desiredRpm - rpmTable[i]) / (rpmTable[i+1] - rpmTable[i]);
      float d    = dutyTable[i] + frac * (dutyTable[i+1] - dutyTable[i]);
      return int(d + 0.5);  // aufrunden
    }
  }
  return dutyTable[N-1];
}

void setup1() {
  Serial.begin(115200);
  delay(200);

  pinMode(fanPwmPin, OUTPUT);
  analogWrite(fanPwmPin, 255);   // startet mit voller Drehzahl (0 % Input)

  pinMode(tachPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(tachPin), countPulse, FALLING);

  Serial.println();
  Serial.println("Gib im Serial Monitor eine Zahl 0-100 ein und drücke Enter:");
}

void loop1() {
  // 1) Serielle Eingabe prüfen
  if (Serial.available()) {
    int p = Serial.parseInt();             
    if (p >= 0 && p <= 100) {
      currentPercent = p;
      int duty = getInterpolatedDuty(currentPercent);
      analogWrite(fanPwmPin, duty);
      Serial.printf("→ Lüfter auf %3d %% gesetzt (Duty=%d)\n", currentPercent, duty);
    } else {
      Serial.println("Ungültig! Bitte 0–100 eingeben.");
    }
    while (Serial.available()) Serial.read();
  }

  // 2) Einmal pro Sekunde RPM ausgeben
  if (millis() - lastRpmTime >= 1000) {
    lastRpmTime = millis();
    noInterrupts();
      int count = pulseCount;
      pulseCount = 0;
    interrupts();

    float rpm = (count / 2.0) * 60.0;  // 2 Pulse/Umdrehung
    Serial.printf("Aktuelle RPM bei %3d%%: %.0f\n", currentPercent, rpm);
  }
}