/*****************************************************************************/
//	Function:    Get the accelemeter of the x/y/z axis. 
//  Hardware:    Grove - 3-Axis Digital Accelerometer(±1.5g)
//	Arduino IDE: Arduino-1.0
//	Author:	 Frankie.Chu		
//	Date: 	 Jan 10,2013
//	Version: v0.9b
//	by www.seeedstudio.com
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2.1 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//
/*******************************************************************************/

#include "Wire.h"
#include "MMA7660.h"
#include "DFRobot_sim808.h"
#include <SD.h>
MMA7660 accelemeter;
DFRobot_SIM808 sim808(&Serial1);

int LED1=3;    //i forget if it should be defined like this
int BUZZER=2;  //define buzzer
int key1=7;    //digital port 6
int LED_light=8;

const int bx=0; const int by=0; const int bz=256; 
int BM=pow(pow(bx,2)+pow(by,2)+pow(bz,2),0.5);
boolean FALL = false;
boolean trigger1=false;
boolean trigger2=false;
boolean trigger3=false; 
byte trigger1count=0; 
byte trigger2count=0; 
byte trigger3count=0; 

void alarm_second(int t)
{
  int i=0;
  for(i=0; i<(2*t); i++){
      digitalWrite(LED1, HIGH);
      digitalWrite(BUZZER, HIGH);
      delay(250);
      digitalWrite(LED1, LOW);
      digitalWrite(BUZZER, LOW);
      delay(250);
   }
}

void setup()
{
	accelemeter.init();  
	Serial.begin(9600);
  Serial1.begin(9600);

  //******** Initialize sim808 module *************
  while(!sim808.init()) {
      delay(1000);
      Serial.print("Sim808 init error\r\n");
    }

  //************* Turn on the GPS power************
  if ( sim808.attachGPS())
    Serial.println("Open the GPS power success");
  else
    Serial.println("Open the GPS power failure");

  if (!SD.begin()) {  //如果从CS口与SD卡通信失败，串口输出信息Card failed, or not present
    Serial.println("Card failed, or not present");
    return;
  }
  Serial.println("card initialized.");  //与SD卡通信成功，串口输出信息card initialized.
  
  pinMode(LED1, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(key1, INPUT);
  pinMode(LED_light, OUTPUT);
}

void loop()
{
	int8_t x;
	int8_t y;
	int8_t z;
  int count=0;
  double AM; double angleChange=0;
  String dataString = "";

  //get three axis data
	accelemeter.getXYZ(&x,&y,&z);
  z=z-21;

  AM=(double)pow(pow(x,2)+pow(y,2)+pow(z,2),0.5);
  if(AM==0.0){
    AM=1;
  }
  
  //Serial.print("AM = ");
  //Serial.println(AM);

  //if key is pressed, alarm and turn on the light for several second.
  if(digitalRead(key1) == HIGH){
    digitalWrite(LED_light, HIGH);
    //send GPS signal
    alarm_second(2);  
    if(SD.exists("3axis.json")) SD.remove("3axis.json");
    File dataFile = SD.open("3axis.json", FILE_WRITE); 
    // 打开datalog.txt文件，读写状态，位置在文件末尾。
    if (dataFile) {
//      dataFile.println(dataString);
    
    dataFile.println("{");
    dataFile.print("\"Status\":");
    dataFile.println("\"Fall\"");
    dataFile.println("}");
      
      dataFile.close();
      // 数组dataString输出到串口
      
    }
    delay(5000);
    digitalWrite(LED_light, LOW);
  }
  
 if (trigger3==true){
   trigger3count++;
   if (trigger3count>=10){ 
    //Serial.println("***********");

    angleChange=abs(acos(((double)x*(double)bx+(double)y*(double)by+(double)z*(double)bz)/(double)AM/(double)BM));
     if (angleChange>=1.6 && angleChange<=2.5){ //if orientation  changes remains between 80-100 degrees
       FALL=true; trigger3=false; trigger3count=0;
       //Serial.print("ANGLE IS ");
       //Serial.println(angleChange);
     }
     else{ 
       trigger3=false; trigger3count=0;
       //Serial.println("TRIGGER 3 DEACTIVATED");
       //Serial.println(angleChange);
     }
   }
 }
 if (FALL==true){ 
   Serial.println("FALL DETECTED");
   alarm_second(3);
   FALL=false; 
   if(SD.exists("3axis.json"))
   SD.remove("3axis.json");
    File dataFile = SD.open("3axis.json", FILE_WRITE); 
    // 打开datalog.txt文件，读写状态，位置在文件末尾。
    if (dataFile) {
//      dataFile.println(dataString);
    
    dataFile.println("{");
    dataFile.print("\"Status\":");
    dataFile.println("\"Fall\"");
    dataFile.println("}");
      
      dataFile.close();
      // 数组dataString输出到串口
      
    }
   delay(10000);  //delay 10s and run again
 }
 if (trigger2count>=6){ 
   trigger2=false; trigger2count=0; angleChange=0;
   //Serial.println("TRIGGER 2 DECACTIVATED");
 }
 if (trigger1count>=6){ //allow 0.5s for AM to break upper threshold
   trigger1=false; trigger1count=0; angleChange=0;
   //Serial.println("TRIGGER 1 DECACTIVATED");
 } 

 if (trigger2==true){
   trigger2count++;
   angleChange=abs(acos(((double)x*(double)bx+(double)y*(double)by+(double)z*(double)bz)/(double)AM/(double)BM));
   //Serial.print("angle = ");
   //Serial.println(angleChange);
   if (angleChange>=1.9 && angleChange<=2.2){ 
     trigger3=true; trigger2=false; trigger2count=0;
     //Serial.println(angleChange);
     //Serial.println("TRIGGER 3 ACTIVATED");
   }
 }
 if (trigger1==true){
   trigger1count++;
   if (AM>=19){ 
     trigger2=true;
     //Serial.println("TRIGGER 2 ACTIVATED");
     trigger1=false; trigger1count=0;
   }
 }
 if ((AM<=10)||(trigger1==false && trigger2==false && trigger3==false)){ 
   trigger1=true;
   //Serial.println("TRIGGER 1 ACTIVATED");
 }
 
// iotkit.send("Accelerometer",AM);
// iotkit.send("FALL",trigger1);
  count++;
  if(SD.exists("3axis.json"))
    SD.remove("3axis.json");
    File dataFile = SD.open("3axis.json", FILE_WRITE); 
    // 打开datalog.txt文件，读写状态，位置在文件末尾。
    if (dataFile) {
//      dataFile.println(dataString);
    
    dataFile.println("{");
    dataFile.print("\"Status\":");
    dataFile.print("\"Normal\"");
    dataFile.println("}");
      
      dataFile.close();
      // 数组dataString输出到串口
      
    }
  if(count >= 20){
    if (sim808.getGPS()) {
      Serial.print("latitude :");
      Serial.println(sim808.GPSdata.lat);
      Serial.print("longitude :");
      Serial.println(sim808.GPSdata.lon);
    
    count = 0;
    
//    dataString = "Hello";

    // 打开文件，注意在同一时间只能有一个文件被打开
    // 如果你要打开另一个文件，就需要先关闭前一个
  
    // 打开datalog.txt文件，读写状态，位置在文件末尾。
    if(SD.exists("geo.json")) SD.remove("geo.json");
    File dataFile = SD.open("geo.json", FILE_WRITE); 
    if (dataFile) {
//      dataFile.println(dataString);

    dataFile.println("{");
    dataFile.print("\"longtitude\":");
    dataFile.print(sim808.GPSdata.lon);
    dataFile.println(",");
    dataFile.print("\"latitude\":");
    dataFile.println(sim808.GPSdata.lat);
    dataFile.println(dataString);
    dataFile.println("}");
      
      dataFile.close();
      // 数组dataString输出到串口 
    }
    }  
    // 如果无法打开文件，串口输出错误信息error opening datalog.txt
    else {
      Serial.println("error opening datalog.txt");
    }
  }
 delay(100);
  
}


