// ===== Blynk credentials (must be BEFORE the includes) =====
#define BLYNK_TEMPLATE_ID   "YourTemplateID"
#define BLYNK_TEMPLATE_NAME "ParkingSensor"
#define BLYNK_AUTH_TOKEN    "YourAuthToken"
#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ===== Wi-Fi (2.4 GHz only) =====
char ssid[] = "YourWiFiName";
char pass[] = "YourWiFiPassword";

// ===== Pins (ESP32-safe GPIOs) =====
const int trigPin  = 5;
const int echoPin  = 18;   // use a voltage divider (5V -> 3.3V)
const int redPin   = 25;
const int greenPin = 26;
const int bluePin  = 27;
const int buzzer   = 23;

// ===== Distance thresholds (cm) =====
const float SAFE_DISTANCE = 10.0;
const float STOP_DISTANCE = 3.0;

// ===== Objects =====
LiquidCrystal_I2C lcd(0x27, 16, 2);
BlynkTimer timer;

String lastStatus = "";   // so we only send status to Blynk when it changes

// Set RGB LED color (0-255 per channel)
// If your LED is common-anode, use: analogWrite(pin, 255 - value)
void setRGB(int r, int g, int b) {
    analogWrite(redPin, r);
    analogWrite(greenPin, g);
    analogWrite(bluePin, b);
}

// Show a status on LCD row 0, set LED, and send to Blynk
void showStatus(const char* lcdText, const char* appText, int r, int g, int b) {
    lcd.setCursor(0, 0);
    lcd.print(lcdText);          // padded with spaces to clear old text
    setRGB(r, g, b);

    if (lastStatus != appText) { // send only on change
        Blynk.virtualWrite(V1, appText);
        lastStatus = appText;
    }
}

// Runs every 500 ms (replaces the old delay()-based loop)
void measureDistance() {
    // Trigger the sensor
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    // Measure echo (30 ms timeout, returns 0 if no echo)
    long duration = pulseIn(echoPin, HIGH, 30000);

    // No reading: handle first so it isn't mistaken for "STOP"
    if (duration == 0) {
        Serial.println("No reading");
        lcd.setCursor(0, 1);
        lcd.print("Dist: --        ");
        showStatus("No Reading!   ", "No Reading", 0, 0, 255);  // Blue
        noTone(buzzer);
        return;
    }

    // Convert to centimeters (speed of sound ~0.0343 cm/us, halved for round trip)
    float distance = duration * 0.0343 / 2.0;

    Serial.print("Distance: ");
    Serial.print(distance);
    Serial.println(" cm");

    // LCD row 1
    lcd.setCursor(0, 1);
    lcd.print("Dist: ");
    lcd.print(distance, 1);
    lcd.print(" cm   ");

    // Send distance to Blynk
    Blynk.virtualWrite(V0, distance);

    // Decide action (no gaps at exactly 10 or 3)
    if (distance > SAFE_DISTANCE) {
        showStatus("Safe Zone     ", "Safe", 0, 255, 0);        // Green
        noTone(buzzer);
    }
    else if (distance > STOP_DISTANCE) {
        showStatus("Slow Down     ", "Slow Down", 255, 255, 0); // Yellow
        tone(buzzer, 1000, 200);   // non-blocking beep (200 ms)
    }
    else {
        showStatus("STOP!         ", "STOP", 255, 0, 0);        // Red
        tone(buzzer, 1000, 400);   // longer beep (400 ms)
    }
}

void setup() {
    Serial.begin(115200);

    pinMode(trigPin, OUTPUT);
    pinMode(echoPin, INPUT);
    pinMode(redPin, OUTPUT);
    pinMode(greenPin, OUTPUT);
    pinMode(bluePin, OUTPUT);
    pinMode(buzzer, OUTPUT);

    Wire.begin(21, 22);          // SDA, SCL
    lcd.init();
    lcd.backlight();
    lcd.setCursor(0, 0);
    lcd.print("Connecting...   ");

    // Connects to Wi-Fi and Blynk (waits up to ~10 s)
    Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

    lcd.clear();
    timer.setInterval(500L, measureDistance);
}

void loop() {
    Blynk.run();
    timer.run();
}
