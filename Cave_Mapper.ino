// Robotics Project - Ultrasonic Cave Mapper. Morgan Potter

#include <Servo.h>
#include "NewPing.h"

// Define the size of the map (cannot be done at runtime due to compiler constraints)


#define mapx 20
#define mapy 20

// Define pi as arduino has no built in number
#define pi 3.1415926535897932384626433832795

// Give required pins a name for later use
int servo1 = 2;
int servo2 = 3;
int UT_Echo = 6;
int UT_Trig = 5;
int button = 4;

// Make two-dimensional map array (row by column for easier printing)
char plot[mapy][mapx];

// Initialize components
Servo bottom_servo;
Servo top_servo;
NewPing sonar(UT_Trig, UT_Echo, 10); // Last value is max distance in cm

void setup()
{
  // Give both servos a starting position
  bottom_servo.write(0);
  top_servo.write(0);

  // Attach the servos to the desired pins and set min and max rotation in ms.
  bottom_servo.attach(servo1, 400, 2400);
  top_servo.attach(servo2, 400, 2400);

  // Initialize button pin as an Input
  pinMode(button, INPUT);

  // Start communicating with serial at baud 9600
  Serial.begin(9600);

  // Fill map with empty spaces
  for (int y = 0; y < mapy; y++) {
    for (int x = 0; x < mapx; x++) {
      plot[y][x] = ' ';
    }
  }
}

String input(String message) {
  // Sends serial message, waits for a response and returns full response string
  Serial.print(message);
  while (!Serial.available()) {
  }
  String read_in = Serial.readString();
  return read_in;
}

void rotate(int increment) {
  // Rotates ultrasonic sensor to a given 15 degree increment. As there is two servos, they are moved in halves.
  bottom_servo.write((7.5)*increment);
  delay(100);
  top_servo.write((7.5)*increment);
  delay(100);
}

int get_x(int distance, int angle)
{
  int i;
  // Returns x values from measured distance and angle as rounded integers
  if ((angle == 90) || (angle == 270)) { // When the coordinate lies on the y axis
    i = 90; // This is required as the modulus operation moves y axis points to the x axis. cosine(0)=1, cosine(90)=0
  } else {
    i = angle % 90; // Gets the interior angle in all quadrents
  }
  float rad = i * (pi / 180); // Converts degrees to radians (Arduino trig uses radians)
  if ((floor(angle / 90) == 1) || (floor(angle / 90) == 2)) {
    // If cosine of the angle is negative in the given quadrent, make the final result negative.
    distance *= -1;
  }
  return round(cos(rad) * distance);
}

int get_y(int distance, int angle) {
  // Returns y values from measured distance and angle as rounded integers
  
  int i;
  if ((angle == 90) || (angle == 270)) {// When the coordinate lies on the y axis
    i = 90; // This is required as the modulus operation moves y axis points to the x axis. sine(0)=0, sine(90)=1.
  } else {
    i = angle % 90; // Gets the interior angle in all quadrents
  }
  
  float rad = i * (pi / 180); // Converts degrees to radians (Arduino trig uses radians)
  
  if ((floor(angle / 90) == 2) || (floor(angle / 90) == 3)) {
    // If sine of the angle is negative in the given quadrent, make the final result negative.
    distance *= -1;
  }
  return round(sin(rad) * distance);
}

void print_plot() {
  // Prints the plot to serial row by column
  
  for (int y = 0; y < mapy; y++) {
    Serial.print("\n");
    
    for (int x = 0; x < mapx; x++) {
      Serial.print(plot[y][x]);
      Serial.print(" "); // Used to make somewhat equal spacing between x and y axis
    }
  }
}
void loop()
{
  if (digitalRead(button) == HIGH) { // If the button is pressed
    
    // Gets a x and y displacement value so multiple scans can be meshed together.
    int startx = input("Starting X Coordinate: \n").toInt();
    int starty = input("Starting Y Coordinate: \n").toInt();
    
    for (int increment = 0; increment < 25; increment += 1) {
      // Rotates the ultrasonic sensor 360 degrees in increments of 15
      rotate(increment);
      
      // Calculates x and y coordinates based on ultrasonic reading, and current rotation
      int distance = round(sonar.ping_cm());
      int angle = 15 * increment;
      int x = get_x(distance, angle);
      int y = get_y(distance, angle);
      
      // Checks if the point is on the map, and places a marker.
      if (mapx > y + starty) {
        if (mapy > x + startx) {
          plot[y + starty][x + startx] = 'x';
        }
      }
    }
    
    // Prints a visualization of the plot, and starts a newline for the next serial message.
    print_plot();
    Serial.print("\n");

  }
}
