

#include <HX711.h>


//data pin and clock pin
byte pinData = 3;
byte pinClk = 2;
const int buzz=11;
 
HX711 scale;
//Parameter to calibrate weight and sensor, different for different sensors
double calibration_factor = -104346; //put some value and adjust it through serial monitor 
 
void setup() 
{
  pinMode(buzz,OUTPUT);
  Serial.begin(9600);
  Serial.println("HX711 calibration");
  Serial.println("Remove all weight from scale");
  Serial.println("After readings begin, place known weight on scale");
  Serial.println("Press a,s,d,f to increase calibration factor by 10,100,1000,10000 respectively");
  Serial.println("Press z,x,c,v to decrease calibration factor by 10,100,1000,10000 respectively");
  digitalWrite(buzz,HIGH);
       delay(100);
       digitalWrite(buzz,LOW);
       delay(50);
        digitalWrite(buzz,HIGH);
       delay(100);
       digitalWrite(buzz,LOW);
  scale.begin(pinData, pinClk);
  //apply the calibration
  scale.set_scale();
  //initializing the tare.
  scale.tare();  //Reset the scale to 0
 
  long zero_factor = scale.read_average(); //Get a baseline reading
  Serial.print("Zero factor: "); //This can be used to remove the need to tare the scale. Useful in permanent scale projects.
  Serial.println(zero_factor);
  
}
void loop() { 
 
  scale.set_scale(calibration_factor); //Adjust to this calibration factor
 
  Serial.print("Reading: ");
  Serial.print(scale.get_units(), 3);
  Serial.print(" kgs"); //This can change this to grams and re-adjust the calibration factor if as another SI units
  Serial.print(" calibration_factor: ");
  Serial.print(calibration_factor);
  Serial.println();
 
  if(Serial.available())
  {
    char temp = Serial.read();
    if(temp == '+' || temp == 'a')
      calibration_factor += 10;
    else if(temp == '-' || temp == 'z')
      calibration_factor -= 10;
    else if(temp == 's')
      calibration_factor += 100;  
    else if(temp == 'x')
      calibration_factor -= 100;  
    else if(temp == 'd')
      calibration_factor += 1000;  
    else if(temp == 'c')
      calibration_factor -= 1000;
    else if(temp == 'f')
      calibration_factor += 100000;  
    else if(temp == 'v')
      calibration_factor -= 100000;  
    else if(temp == 't'){
       scale.tare();  //Reset the scale to zero
       digitalWrite(buzz,HIGH);
       delay(100);
       digitalWrite(buzz,LOW);
    }
     
  }
}
