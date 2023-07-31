#include <EEPROM.h>

#include <MovingAverage.h>

#include <LiquidCrystal_I2C.h>

#include <HX711.h>


#define DEBUG_HX711
 

// calibration factor by calibrating load cell
#define CALIBRATION_FACTOR -104346

//accelorometer pin define
#define xPin A1
#define yPin A2
#define zPin A3

float x_g_value;
float y_g_value;
float z_g_value;

int levelXint;
int levelYint;
int levelZint;


// Create the lcd display address 0x27 and 16 columns x 2 rows
LiquidCrystal_I2C lcd(0x27, 16, 2);

//define moving averages for HX711 and accelerometer data
MovingAverage<float> avgKg(20, 0.0);
MovingAverage<int> avgX(30, 0);
MovingAverage<int> avgY(30, 0);
MovingAverage<int> avgZ(30, 0);


// define HX711

// define data pin and clock pin
byte pinData = 3;
byte pinClk = 2;
float adj = 2.680;

//define the variables
int mode = 0;
int longPress = 0;
const int buzz = 11;
float a;  //for use in scaler button
bool onetimeFlag = false,onetimeWarning=false;

//define the HX711 Module
HX711 scale;

//define accelorometer
int xMin = 260;
int xMax = 420;

int yMin = 260;
int yMax = 420;

int zMin = 270;
int zMax = 430;

const int samples = 10;

void setup() {

  pinMode(buzz, OUTPUT);
  pinMode(8, INPUT_PULLUP);  //reset push button connected to pin 8
  pinMode(9, INPUT_PULLUP);  //scaler push button connected to pin 9

  digitalWrite(buzz, HIGH);
  delay(100);
  digitalWrite(buzz, LOW);
  delay(50);
  digitalWrite(buzz, HIGH);
  delay(100);
  digitalWrite(buzz, LOW);

  //LCD Display intialization
  lcd.init();
  lcd.backlight();

#ifdef DEBUG_HX711
  // Initialize serial communication
  Serial.begin(115200);
  Serial.println("[HX711] Sensor start HX711");
#endif

  scale.begin(pinData, pinClk);
  // apply the calibration value
  scale.set_scale(CALIBRATION_FACTOR);

  //define saved data in EPROM
  levelXint = fetchInt(0);
  levelYint = fetchInt(2);
  levelZint = fetchInt(4);

  for (int i = 0; i < 50; i++) {
    acc();
  }

}

 
void loop() {
  acc();
  weight();

  if (digitalRead(8) == LOW)  //reset button

  { 
    longPress = 0;
    delay(25);
    
    while (digitalRead(8) == LOW) {

      longPress++;
      delay(30);

      if (longPress == 30) {
        digitalWrite(buzz, HIGH);
        delay(100);
        digitalWrite(buzz, LOW);
        setLevel();
      }
    }

    if (longPress < 30)reset();
    delay(20);
  }  //end reset button


  if (digitalRead(9) == LOW)  //scaler button

  {
    digitalWrite(buzz, HIGH);
    delay(50);
    digitalWrite(buzz, LOW);
    mode++;

    if (mode > 2) mode = 0;

    while (digitalRead(9) == LOW) {
      delay(20);
    } //end scaler button

  }

  if (avgKg.get() > 18.0)errorMsg(" OVER WEIGHT !", "    18kg MAX");  //display warning message when 18Kg limit and stop displying mass
  else if (avgKg.get() > 16.0&&!onetimeWarning)warningMsg();  //display warning message when 18Kg limit 
  else if (!isLevel()) errorMsg(" NOT BALANCED !", ""); //display "Not balanced" when loading plate is not match with previously saved data

  else  {
    
    onetimeFlag = false;
    if(avgKg.get()<16)onetimeWarning =false;   //reseting flag only when weight is released

    switch (mode) {
      case 0: kgMode();   break;  //define funtion to display mass in kilograms
      case 1: gramMode();   break;  //define funtion to display mass in grams
      case 2: poundMode();    break;  //define funtion to display mass in pounds

    }

  }
  delay(50);

}


void weight() {
  float w = scale.get_units() - adj;
  avgKg.push(w);  //get the final output from HX711 Module

}

void errorMsg(String line1, String line2) { //define the message positions in LCD display
  lcd.clear();
  lcd.print(line1);
  lcd.setCursor(0, 1);
  lcd.print(line2);


  if (!onetimeFlag) {         //for onetime beep

    digitalWrite(buzz, HIGH);
    delay(1000);
    digitalWrite(buzz, LOW);
    onetimeFlag = true;

  }

}


void warningMsg() { //overload warning function

    lcd.clear();
    lcd.print("Overload Warning");
    lcd.setCursor(0, 1);
    lcd.print(avgKg.get(), 2);
    lcd.print("kg");
    digitalWrite(buzz, HIGH);
    delay(1000);
    digitalWrite(buzz, LOW);
    onetimeWarning = true;

}
 

void reset() {  //reset the scale function

  lcd.clear();
  scale.tare();
  adj = 0;
  digitalWrite(buzz, HIGH);
  delay(50);
  digitalWrite(buzz, LOW);
  delay(50);

  lcd.setCursor(0, 0);
  lcd.print("Calibration Done");
  delay(2500);

}


void setLevel() { //to inform user about saving acceleromter data

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Level updating..");
  
  for (int i = 0; i < 10; i++) {

    acc();
    delay(100);

  }

  saveInt(0, avgX.get());
  saveInt(2, avgY.get());
  saveInt(4, avgZ.get());

  levelXint = avgX.get();
  levelYint = avgY.get();
  levelZint = avgZ.get();

  lcd.clear();
  lcd.setCursor(1, 0);
  lcd.print("Level updated");
  delay(2500);

}


bool isLevel() {  //function to check the state of the loading plate comparing to previously saved data

  int xerr = levelXint - avgX.get();
  int yerr = levelYint - avgY.get();
  int zerr = levelZint - avgZ.get();

  int levelLimit = 2;

  if (xerr > levelLimit || xerr < -levelLimit)return false ;
  else if (yerr > levelLimit || yerr < -levelLimit)return false ;
  else if (zerr > levelLimit || zerr < -levelLimit)return false ;

  else

    onetimeFlag = false;

  return true;

}


void kgMode() { //function to get the mass in Kilograms

  lcd.clear();
  lcd.print("Weight (Kg)");
  lcd.setCursor(0, 1);
  lcd.print(avgKg.get(), 2);

}

 
void gramMode() { //function to get the mass in grams

  a = avgKg.get() * 1000;  // kg to g *1000

  lcd.clear();
  lcd.print("Weight (g)");
  lcd.setCursor(0, 1);
  //lcd.print(a);
  lcd.print(a,0.1);

}

 
void poundMode() {  //function to get the mass in pounds

  a = avgKg.get() * 2.205;  //kg to pound *2.205

  lcd.clear();
  lcd.print("Weight (lbs) ");
  lcd.setCursor(0, 1);
  lcd.print(a);

}

 
void acc() {  //getting values from acceleromter 

  avgX.push(analogRead(xPin));
  avgY.push(analogRead(yPin));
  avgZ.push(analogRead(zPin));

  long xMilliG = map(avgX.get(), xMin, xMax, -1000, 1000);
  long yMilliG = map(avgY.get(), yMin, yMax, -1000, 1000);
  long zMilliG = map(avgZ.get(), zMin, zMax, -1000, 1000);

  // re-scale to fractional Gs
  x_g_value = xMilliG / 1000.0;
  y_g_value = yMilliG / 1000.0;
  z_g_value = zMilliG / 1000.0;

  Serial.print (avgX.get());
  Serial.print (", ");
  Serial.print (avgY.get());
  Serial.print (", ");
  Serial.println (avgZ.get());

}


void saveInt(int address, int value) {  //save the acceleromter data in EEPROM 

  EEPROM.write(address, highByte(value));
  EEPROM.write(address + 1, lowByte(value));

}


int fetchInt(int address) { //to select the relevent address in EEPROM 

  int value = (EEPROM.read(address) << 8) | EEPROM.read(address + 1);
  return value;

}

 
