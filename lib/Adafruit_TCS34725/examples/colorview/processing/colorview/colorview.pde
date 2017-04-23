/* For use with the colorview Arduino example sketch 
   Update the Serial() new call to match your serial port
   e.g. COM4, /dev/usbserial, etc!
*/


import processing.serial.*;
import java.awt.datatransfer.*;
import java.awt.Toolkit;

Serial port;
 
void setup(){
 size(200,200);
 port = new Serial(this, "COM4", 9600); //remember to replace COM20 with the appropriate serial port on your computer
}
 
 
String buff = "";

int wRed, wGreen, wBlue, wClear;
String hexColor = "ffffff";
 

void draw(){
 background(wRed,wGreen,wBlue);
 // check for serial, and process
 while (port.available() > 0) {
   serialEvent(port.read());
 }
}
 
void serialEvent(int serial) {
 if(serial != '\n') {
   buff += char(serial);
 } else {
   //println(buff);
   
   int cRed = buff.indexOf("R");
   int cGreen = buff.indexOf("G");
   int cBlue = buff.indexOf("B");
   int clear = buff.indexOf("C");
   if(clear >=0){
     String val = buff.substring(clear+3);
     val = val.split("\t")[0]; 
     wClear = Integer.parseInt(val.trim());
   } else { return; }
   
   if(cRed >=0){
     String val = buff.substring(cRed+3);
     val = val.split("\t")[0]; 
     wRed = Integer.parseInt(val.trim());
   } else { return; }
   
   if(cGreen >=0) {
     String val = buff.substring(cGreen+3);
     val = val.split("\t")[0]; 
     wGreen = Integer.parseInt(val.trim());
   } else { return; }
   
   if(cBlue >=0) {
     String val = buff.substring(cBlue+3);
     val = val.split("\t")[0]; 
     wBlue = Integer.parseInt(val.trim());
   } else { return; }
   
   print("Red: "); print(wRed);
   print("\tGrn: "); print(wGreen);
   print("\tBlue: "); print(wBlue);
   print("\tClr: "); println(wClear);
   
   wRed *= 255; wRed /= wClear;
   wGreen *= 255; wGreen /= wClear; 
   wBlue *= 255; wBlue /= wClear; 

   hexColor = hex(color(wRed, wGreen, wBlue), 6);
   println(hexColor);
   buff = "";
 }
}
