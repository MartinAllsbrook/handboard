// The SFE_LSM9DS1 library requires both Wire and SPI be
// included BEFORE including the 9DS1 library.
#include <Wire.h>
#include <SPI.h>
#include <SparkFunLSM9DS1.h>

// Include seven segment library
#include "SevSeg.h"

// Use the LSM9DS1 class to create an object. [imu] can be
// named anything, we'll refer to that throught the sketch.
LSM9DS1 imu;

//Create an instance of the object.
SevSeg myDisplay;

#define PRINT_CALCULATED
// #define PRINT_RAW
#define PRINT_SPEED 250 // 250 ms between prints
static unsigned long lastPrint = 0; // Keep track of print time

// Earth's magnetic field varies by location. Add or subtract
// a declination to get a more accurate heading. Calculate
// your's here:
// http://www.ngdc.noaa.gov/geomag-web/#declination
#define DECLINATION -8.58 // Declination (degrees) in Boulder, CO.

//Function definitions
void printGyro();
void printAccel();
void printMag();
void printAttitude(float ax, float ay, float az, float mx, float my, float mz);



// -------------------- Create global variables --------------------
// These variables are for count up test of sev seg
unsigned long timer;
int deciSecond = 0;
char tempString[10]; //Used for sprintf
int score = 0;
int countDown = 0;
int toDis = 0;

int kfStage = 0;
int hfStage = 0;
int shoveStage = 0;
int fShoveStage = 0;

int kfDone = 0;
int hfDone = 0;
int shoveDone = 0;
int fShoveDone = 0;

float zRotTot = 0;

// -------------------- Setup function --------------------
void setup()
{
  Serial.begin(115200);

  Wire.begin();

  if (imu.begin() == false) // with no arguments, this uses default addresses (AG:0x6B, M:0x1E) and i2c port (Wire).
  {
    Serial.println("Failed to communicate with LSM9DS1.");
    Serial.println("Double-check wiring.");
    Serial.println("Default settings in this sketch will " \
                   "work for an out of the box LSM9DS1 " \
                   "Breakout, but may need to be modified " \
                   "if the board jumpers are.");
    while (1);
  }

// -------------------- /\ IMU || SevSeg \/ --------------------
  
  int displayType = COMMON_CATHODE; //Your display is either common cathode or common anode

  //This pinout is for a regular display
   //Declare what pins are connected to the digits
   int digit1 = 5; //Pin 12 on my 4 digit display
   int digit2 = 4; //Pin 9 on my 4 digit display
   int digit3 = 3; //Pin 8 on my 4 digit display
   int digit4 = 2; //Pin 6 on my 4 digit display
   
   //Declare what pins are connected to the segments
   int segA = 13; //Pin 11 on my 4 digit display
   int segF = 12; //Pin 10 on my 4 digit display
   int segB = 11; //Pin 7 on my 4 digit display
   int segG = 10; //Pin 5 on my 4 digit display
   int segC = 9; //Pin 4 on my 4 digit display
   int segDP= 8; //Pin 3 on my 4 digit display
   int segD = 7; //Pin 2 on my 4 digit display
   int segE = 6; //Pin 1 on my 4 digit display

  int numberOfDigits = 4; //Do you have a 1, 2 or 4 digit display?

  myDisplay.Begin(displayType, numberOfDigits, digit1, digit2, digit3, digit4, segA, segB, segC, segD, segE, segF, segG, segDP);
  
  myDisplay.SetBrightness(100); //Set the display to 100% brightness level

  timer = millis();
}
// -------------------- Setup function --------------------

// ---------------------------------------------------------------------------------------------------------------------------


void loop()
{  
  // Update the sensor values whenever new data is available
  if ( imu.gyroAvailable() )
  {
    imu.readGyro();
  }
  if ( imu.accelAvailable() )
  {
    imu.readAccel();
  }
  if ( imu.magAvailable() )
  {
    imu.readMag();
  }

  calcTricks(imu.ax, imu.ay, imu.az,
                  -imu.my, -imu.mx, imu.mz);
  
  // Everything in this block only happens on an interval of PRINT_SPEED
  if ((lastPrint + PRINT_SPEED) < millis())
  {
    printGyro();  // Print "G: gx, gy, gz"
    printAccel(); // Print "A: ax, ay, az"
    printMag();   // Print "M: mx, my, mz"
    // Print the heading and orientation for fun!
    // Call print attitude. The LSM9DS1's mag x and y
    // axes are opposite to the accelerometer, so my, mx are
    // substituted for each other.
    printAttitude(imu.ax, imu.ay, imu.az,
                  -imu.my, -imu.mx, imu.mz);
    Serial.println();

    lastPrint = millis(); // Update lastPrint time
  }

  // Print the score on seven segment display
  sprintf(tempString, "%4d", toDis);
  myDisplay.DisplayString(tempString, 0); //(numberscore, decimal point location)
  
  //Example ways of displaying a decimal number
  //char tempString[10]; //Used for sprintf
  //sprintf(tempString, "%4d", deciSecond); //Convert deciSecond into a string that is right adjusted
  //sprintf(tempString, "%d", imu.calcGyro(imu.gx)); //Convert deciSecond into a string that is left adjusted
  //sprintf(tempString, "%04d", deciSecond); //Convert deciSecond into a string with leading zeros
  //sprintf(tempString, "%4d", deciSecond * -1); //Shows a negative sign infront of right adjusted number
  //sprintf(tempString, "%4X", deciSecond); //Count in HEX, right adjusted
  //myDisplay.DisplayString(tempString, 0); //Display string, no decimal point
  //myDisplay.DisplayString("-23b", 3); //Display string, decimal point in third position
}

// ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void printGyro()
{
  // Now we can use the gx, gy, and gz variables as we please.
  // Either print them as raw ADC values, or calculated in DPS.
  Serial.print("G: ");
#ifdef PRINT_CALCULATED
  // If you want to print calculated values, you can use the
  // calcGyro helper function to convert a raw ADC value to
  // DPS. Give the function the value that you want to convert.
  Serial.print(imu.calcGyro(imu.gx), 2);
  Serial.print(", ");
  Serial.print(imu.calcGyro(imu.gy), 2);
  Serial.print(", ");
  Serial.print(imu.calcGyro(imu.gz), 2);
  Serial.println(" deg/s");
#elif defined PRINT_RAW
  Serial.print(imu.gx);
  Serial.print(", ");
  Serial.print(imu.gy);
  Serial.print(", ");
  Serial.println(imu.gz);
#endif
}

void printAccel()
{
  // Now we can use the ax, ay, and az variables as we please.
  // Either print them as raw ADC values, or calculated in g's.
  Serial.print("A: ");
#ifdef PRINT_CALCULATED
  // If you want to print calculated values, you can use the
  // calcAccel helper function to convert a raw ADC value to
  // g's. Give the function the value that you want to convert.
  Serial.print(imu.calcAccel(imu.ax), 2);
  Serial.print(", ");
  Serial.print(imu.calcAccel(imu.ay), 2);
  Serial.print(", ");
  Serial.print(imu.calcAccel(imu.az), 2);
  Serial.println(" g");
#elif defined PRINT_RAW
  Serial.print(imu.ax);
  Serial.print(", ");
  Serial.print(imu.ay);
  Serial.print(", ");
  Serial.println(imu.az);
#endif

}

void printMag()
{
  // Now we can use the mx, my, and mz variables as we please.
  // Either print them as raw ADC values, or calculated in Gauss.
  Serial.print("M: ");
#ifdef PRINT_CALCULATED
  // If you want to print calculated values, you can use the
  // calcMag helper function to convert a raw ADC value to
  // Gauss. Give the function the value that you want to convert.
  Serial.print(imu.calcMag(imu.mx), 2);
  Serial.print(", ");
  Serial.print(imu.calcMag(imu.my), 2);
  Serial.print(", ");
  Serial.print(imu.calcMag(imu.mz), 2);
  Serial.println(" gauss");
#elif defined PRINT_RAW
  Serial.print(imu.mx);
  Serial.print(", ");
  Serial.print(imu.my);
  Serial.print(", ");
  Serial.println(imu.mz);
#endif
}

void printAttitude(float ax, float ay, float az, float mx, float my, float mz)
{
  float roll = atan2(ay, az);
  float pitch = atan2(-ax, sqrt(ay * ay + az * az));

  float heading;
  if (my == 0)
    heading = (mx < 0) ? PI : 0;
  else
    heading = atan2(mx, my);

  heading -= DECLINATION * PI / 180;

  if (heading > PI) heading -= (2 * PI);
  else if (heading < -PI) heading += (2 * PI);

  // Convert everything from radians to degrees:
  heading *= 180.0 / PI;
  pitch *= 180.0 / PI;
  roll  *= 180.0 / PI;

  Serial.print("Pitch, Roll: ");
  Serial.print(pitch, 2);
  Serial.print(", ");
  Serial.println(roll, 2);
  Serial.print("Heading: "); Serial.println(heading, 2);
}

void calcTricks(float ax, float ay, float az, float mx, float my, float mz)
{
  float roll = atan2(ay, az);
  float pitch = atan2(-ax, sqrt(ay * ay + az * az));

  float heading;
  if (my == 0)
    heading = (mx < 0) ? PI : 0;
  else
    heading = atan2(mx, my);

  heading -= DECLINATION * PI / 180;

  if (heading > PI) heading -= (2 * PI);
  else if (heading < -PI) heading += (2 * PI);

  // Convert everything from radians to degrees:
  heading *= 180.0 / PI;
  pitch *= 180.0 / PI;
  roll  *= 180.0 / PI;

  float xRate = imu.calcGyro(imu.gx);
  float zAccel = imu.calcAccel(imu.az);
  float zRate = imu.calcGyro(imu.gz) + 3.2;
  zRotTot += zRate;
//  
//  score = xRate;

  float accelSum = (abs(imu.calcAccel(imu.ax)) + abs(imu.calcAccel(imu.ay)) + abs(imu.calcAccel(imu.az))) * 1000;
//if(zAccel < -1.6){
//  score = 124;
//}else if(score > 0){
//  score--;
//}

  toDis = score;
  if(kfDone > 0){
    toDis = 1010;
    kfDone--;
  }
  if(hfDone > 0){
    toDis = 2020;
    hfDone--;
  }
  if(shoveDone > 0){
    toDis = 3030;
    shoveDone--;
  }
  if(fShoveDone > 0){
    toDis = 4040;
    fShoveDone--;
  }

  //Coundown to make sure tricks are being executed quickly
  if(countDown > 0){
    countDown--;
  }else if(countDown == 0){
    kfStage = 0;
    hfStage = 0;
    shoveStage = 0;
    fShoveStage = 0;
  }

  // Detect kickflip
  if(xRate < -285 && kfStage == 0){
    kfStage++;
    countDown = 50;
  }else if(accelSum < 200 && kfStage == 1 && countDown > 0){
    kfStage++;
    countDown = 50;
  }else if(zAccel < -1.6 && kfStage == 2 && countDown > 0){
    kickflip();
  }

  // Detect Heelflip
  if(xRate > 285 && hfStage == 0){
    hfStage++;
    countDown = 50;
  }else if(accelSum < 200 && hfStage == 1 && countDown > 0){
    hfStage++;
    countDown = 50;
  }else if(zAccel < -1.6 && hfStage == 2 && countDown > 0){
    heelflip();
  }

  // Detect shove
  if(zRate > 250 && shoveStage == 0){
    shoveStage++;
    zRotTot = 0;
    countDown = 50;
  }else if(zRotTot > 9000 && shoveStage == 1 && countDown > 0){
//    shoveStage++;
//    countDown = 50;
    shove();
  }
//  else if(zAccel < -1.35 && shoveStage == 2 && countDown > 0){
//    shove();
//  }

  // Detect front shove
  if(zRate < -250 && fShoveStage == 0){
    fShoveStage++;
    zRotTot = 0;
    countDown = 50;
  }else if(zRotTot < -9000 && fShoveStage == 1 && countDown > 0){
//    fShoveStage++;
//    countDown = 50;
    fShove();
  }
//  else if(zAccel < -1.35 && fShoveStage == 2 && countDown > 0){
//    fShove();
//  }
}

void kickflip(){
  kfDone = 50;
  kfStage = 0;
  score += 10;
}

void heelflip(){
  hfDone = 50;
  hfStage = 0;
  score += 10;
}

void shove(){
  shoveDone = 50;
  shoveStage = 0;
  score += 5;
}

void fShove(){
  fShoveDone = 50;
  fShoveStage = 0;
  score += 5;
}
