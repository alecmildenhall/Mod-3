#include <ESP32Servo.h>
#include <WiFi.h>
#include <HTTPClient.h>
#define USE_SERIAL Serial

const char *ssid_Router     = "Fios-WDF3z"; //Enter the router name
const char *password_Router = "ohm665sake65ply"; //Enter the router password

String address= "http://134.122.113.13/apm2189/running";

Servo myservo;  // create servo object to control a servo

int servoPin = 15; // Servo motor pin2

int currVal = 0;
int flag = 1;

// Connect the port of the stepper motor driver 1
int outPorts1[] = {12, 27, 26, 25};

// Connect the port of the stepper motor driver 2
int outPorts2[] = {13, 33, 32, 2};

void setup() {
  USE_SERIAL.begin(115200);

  WiFi.begin(ssid_Router, password_Router);
  USE_SERIAL.println(String("Connecting to ")+ssid_Router);
  while (WiFi.status() != WL_CONNECTED){
    delay(500);
    USE_SERIAL.print(".");
  }
  
  USE_SERIAL.println("\nConnected, IP address: ");
  USE_SERIAL.println(WiFi.localIP());
  
  // set pins to output
  for (int i = 0; i < 4; i++) {
    pinMode(outPorts1[i], OUTPUT);
    pinMode(outPorts2[i], OUTPUT);
  }

  // servo setup
  
  myservo.setPeriodHertz(50);           // standard 50 hz servo
  myservo.attach(servoPin, 500, 2500);  // attaches the servo on servoPin to the servo object

  USE_SERIAL.println("Setup End");
}

void loop() {
  
  if((WiFi.status() == WL_CONNECTED)) {
    HTTPClient http;
    http.begin(address);
 
    int httpCode = http.GET(); // start connection and send HTTP header
    if (httpCode == HTTP_CODE_OK) { 
        String response = http.getString();
        if (response.equals("false")) {
            delay(200);
        }
        else if(response.equals("true")) {
            move();
        }
        USE_SERIAL.println("Response was: " + response);
    } else {
        USE_SERIAL.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
    delay(500); // sleep for half of a second
  }
}

void move()
{
  // Hug
  moveSteps(true, 32 * 20, 3);
  moveSteps(false, 32 * 20, 3);
  delay(1000);
  int startVal = 0;
  int endVal = 10;

  // turn 30 up and down
  if (flag) {
    currVal = 40;
    flag = 0;
  } else {
    currVal = 0;
    flag = 1;
  }
 
  myservo.write(currVal);       // tell servo to go to position in variable 'pos'
  delay(15);                   // waits 15ms for the servo to reach the position
}

//Suggestion: the motor turns precisely when the ms range is between 3 and 20
void moveSteps(bool dir, int steps, byte ms) {
  for (unsigned long i = 0; i < steps; i++) {
    moveOneStep(dir); // Rotate a step
    delay(constrain(ms,3,20));        // Control the speed
  }
}

void moveOneStep(bool dir) {
  // Define a variable, use four low bit to indicate the state of port
  static byte out = 0x01;
  // Decide the shift direction according to the rotation direction
  if (dir) {  // ring shift left
    out != 0x08 ? out = out << 1 : out = 0x01;
  }
  else {      // ring shift right
    out != 0x01 ? out = out >> 1 : out = 0x08;
  }
  // Output singal to each port
  for (int i = 0; i < 4; i++) {
    digitalWrite(outPorts1[i], (out & (0x01 << i)) ? HIGH : LOW);
    digitalWrite(outPorts2[i], (out & (0x01 << i)) ? HIGH : LOW);
  }
}

void moveAround(bool dir, int turns, byte ms){
  for(int i=0;i<turns;i++)
    moveSteps(dir,32*64,ms);
}
void moveAngle(bool dir, int angle, byte ms){
  moveSteps(dir,(angle*32*64/360),ms);
}
