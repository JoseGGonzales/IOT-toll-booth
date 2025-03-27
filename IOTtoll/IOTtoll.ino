#include <Servo.h>

#define IR_SENSOR_PIN_1 13 
#define IR_SENSOR_PIN_2 3 
#define SERVO_PIN_9 9 // Servo pin for IR sensor
#define SERVO_PIN_10 10 // Servo pin for ultrasonic sensor
#define TRIGGER_PIN 6
#define ECHO_PIN 7
#define OBSTACLE_DETECTED LOW // Change this if the sensors have different logic levels

Servo gateServo9; // Servo for IR sensor
Servo gateServo10; // Servo for ultrasonic sensor

int carsEntered = 0;
int carsExited = 0;

bool gate9Open = false;
bool gate10Open = false;

unsigned long entryTimes[100]; // Array to store entry times (assuming a maximum of 100 cars)
unsigned long exitTimes[100]; // Array to store exit times (assuming a maximum of 100 cars)
int currentCarIndex = 0; // Index to keep track of the current car

void setup() {
  Serial.begin(9600);
  pinMode(IR_SENSOR_PIN_1, INPUT);
  pinMode(IR_SENSOR_PIN_2, INPUT);
  pinMode(TRIGGER_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  gateServo9.attach(SERVO_PIN_9);
  gateServo10.attach(SERVO_PIN_10);

  // Initially, keep the gates closed
  closeGate9();
  closeGate10();

  // Prompt the user to type 'data' to see information
  Serial.println("Please type 'data' to see information.");
}

void loop() {
  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n');
    if (input == "data") {
      Serial.print("Cars Entered: ");
      Serial.print(carsEntered);
      Serial.print(", Cars Exited: ");
      Serial.println(carsExited);
      
      // Calculate and print the average time cars stay in the lot
      if (carsExited > 0) {
        unsigned long totalStayTime = 0;
        for (int i = 0; i < carsExited; i++) {
          totalStayTime += exitTimes[i] - entryTimes[i];
        }
        unsigned long avgStayTime = totalStayTime / carsExited / 1000; // Convert milliseconds to seconds
        Serial.print("Average Stay Time (s): ");
        Serial.println(avgStayTime);
      }
    }
  }

  int irSensor1Value = digitalRead(IR_SENSOR_PIN_1);
  int irSensor2Value = digitalRead(IR_SENSOR_PIN_2);

  // Check if the gate needs to be opened or closed based on IR sensor
  if (irSensor1Value == OBSTACLE_DETECTED && !gate9Open) {
    // If sensor 1 detects an obstacle and gate 9 is not already open, open gate 9
    openGate9();
    carsEntered++;
    gate9Open = true;
    entryTimes[currentCarIndex] = millis(); // Record entry time
  } else if (irSensor2Value == OBSTACLE_DETECTED && gate9Open) {
    // If sensor 2 detects an obstacle and gate 9 is open, close gate 9
    closeGate9();
    carsExited++;
    gate9Open = false;
    exitTimes[currentCarIndex] = millis(); // Record exit time
    currentCarIndex++;
  } else {
    // If no obstacle detected by IR sensors, check distance using ultrasonic sensor
    int distance = measureDistance();
    if (distance < 10 && !gate10Open) { // Adjust the threshold according to your needs
      // If distance is less than 10cm and gate 10 is not already open, open gate 10
      openGate10();
      gate10Open = true;
      delay(2500); // Keep the gate open for 2.5 seconds
      closeGate10(); // Close gate 10
      gate10Open = false;
    }
  }
}

void openGate9() {
  gateServo9.write(90); // Rotate the servo attached to pin 9 to open the gate
}

void closeGate9() {
  gateServo9.write(0); // Rotate the servo attached to pin 9 to close the gate
}

void openGate10() {
  gateServo10.write(90); // Rotate the servo attached to pin 10 to open the gate
}

void closeGate10() {
  gateServo10.write(0); // Rotate the servo attached to pin 10 to close the gate
}

int measureDistance() {
  // Send a pulse to trigger the ultrasonic sensor
  digitalWrite(TRIGGER_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIGGER_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER_PIN, LOW);
  
  // Measure the duration of the pulse from the echo pin
  long duration = pulseIn(ECHO_PIN, HIGH);
  
  // Convert the duration to distance in centimeters
  int distance = duration * 0.034 / 2;
  
  return distance;
}
