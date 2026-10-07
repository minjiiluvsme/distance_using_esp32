#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Define ultrasonic sensor pins
const int trigPin = 9;
const int echoPin = 10;

// Define RGB LED pins (ensure they are PWM-capable)
const int redPin = 6;
const int greenPin = 5;
const int bluePin = 4;

// Define buzzer pin
const int buzzer = 3;

// Initialize I2C LCD (I2C address: 0x27, 16 columns, 2 rows)
LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
    pinMode(trigPin, OUTPUT);
    pinMode(echoPin, INPUT);
    pinMode(redPin, OUTPUT);
    pinMode(greenPin, OUTPUT);
    pinMode(bluePin, OUTPUT);
    pinMode(buzzer, OUTPUT);

    lcd.init();
    lcd.backlight();
    Serial.begin(9600);  // Enable Serial Monitor
}

void loop() {
    long duration;
    float distance;

    // Trigger ultrasonic sensor
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    // Measure the echo pulse duration
    duration = pulseIn(echoPin, HIGH);
    
    // Convert duration to distance in meters
    distance = duration * 0.040 / 2;

    // Debug output to Serial Monitor
    Serial.print("Distance: ");
    Serial.print(distance);
    Serial.println(" m");

    // LCD display update
    lcd.setCursor(0, 1);
    lcd.print("Distance: ");
    lcd.print(distance, 2); // Show 2 decimal places
    lcd.print("m  "); // Spaces clear old characters

    // Condition checks for distance and corresponding actions
    if (distance > 10) {  
        lcd.setCursor(0, 0);
        lcd.print("Safe Zone     "); // Added spaces to clear previous text
        setRGB(0, 255, 0);  // Green
        noTone(buzzer);
    } 
    else if (distance <10 && distance > 3 ) {  
        lcd.setCursor(0, 0);
        lcd.print("Slow Down     ");
        setRGB(255, 255, 0);  // Yellow
        tone(buzzer, 1000, 200);
        delay(500);
        noTone(buzzer);
    } 
    else if (distance <3 ) {  
        lcd.setCursor(0, 0);
        lcd.print("STOP!         ");
        setRGB(255, 0, 0);  // Red
        tone(buzzer, 1000, 500);
        delay(200);
        noTone(buzzer);
    } 
    else {  
        lcd.setCursor(0, 0);
        lcd.print("No Reading!   ");
        setRGB(0, 0, 255);  // Blue (indicates sensor error)
        noTone(buzzer);
    }

    delay(500);
}

// Function to set RGB LED color
void setRGB(int r, int g, int b) {
    analogWrite(redPin, r);
    analogWrite(greenPin, g);
    analogWrite(bluePin, b);
}
