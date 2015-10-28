// as of 03-JUN-2015, this is the latest version
// added the 30 degrees feature (lights off from 0 to 30)

String inputString="", lat="", longit="";
boolean acquired = false, breakOut = false, sweep=false;
unsigned long currentMillis;
int sweepStart, longitudeInt, b,Slider=255;

// Variables will change:
int ledState = LOW;             // ledState used to set the LED
long previousMillis = 0;        // will store last time LED was updated

// the follow variables is a long because the time, measured in miliseconds,
// will quickly become a bigger number than can be stored in an int.
unsigned long interval = 1000;           // interval at which to blink (milliseconds)

int fixationLED = 10;
void setup() {
  Serial.begin(9600);
    
  // SET THE PINS WHICH WILL BE USED 
  // we set the longitudes as OUTPUT
  for (int i=2; i<=10; i++) {
    pinMode(i, OUTPUT);
  }
  
  // we then set the latitudes as output
  for (int m=22; m<=52; m++) {
    pinMode(m, OUTPUT);
  }
  
  // we will set the PWM pin which control the camera-side LEDs 
  
  pinMode(fixationLED, OUTPUT);    // 4 gaze fixation target visible LEDs
  //pinMode(IRLED, OUTPUT);    // 4 IR LEDs for the camera
  //pinMode(ground,OUTPUT);
  // next we give the PWM command to drive them. THey have 100E current limiting resistors
  //analogWrite(fixationLED, 80);
  //digitalWrite(IRLED, HIGH);
  digitalWrite(ground,LOW);
}

void loop() {
 if (sweep == true) {
   currentMillis = millis();
   
   // the code for choosing which LEDs need to be on..
   // step 3: we put the latitudes high one by one with a time delay
            // Serial.println("entered loop");
         if(currentMillis - previousMillis <= interval) {
           // clear the previous latitude..
           if (b < sweepStart) {
             digitalWrite(b+1, LOW);
           } 
           if (b >= 2) {
             // then, write the present one HIGH
             analogWrite(b, Slider);
             
           } else {
             analogWrite(longitudeInt, Slider);
             digitalWrite(2, LOW);  // clear the last one as well, which will always be the topmost one (assuming a test is always completed when started).
             sweep = false;    // gtfo
           }
         } else {           // what to do when its within the interval
           Serial.println(b-1);    // That's the iteration of the LED that's ON 
           b--;    // change the b value
           previousMillis = currentMillis;   
           // We notify over serial (to processing), that the next LED has come on.
         }
 }
}

void serialEvent() {
  // breakOut = false;
  if (Serial.available()) {
    char inChar = (char)Serial.read(); 
    // adding an 'x' for breaking out of any for loop..
    if (inChar == ',') {  // normal, previous function
      breakOut = false;
      // Serial.println(inputString);
      lat = inputString;
 //***     Serial.println(lat);
      // reset that shit
      inputString = "";
    } else {
      
      if (inChar == '\n') {
        breakOut = false;
        longit = inputString;
 //***       Serial.println(longit);
        // now we reset the shit out of it all...
        
        // step 1: turn OFF all latitudes..
        for (int h=2; h<=10; h++) {
         digitalWrite(h, LOW);
        }
        // step 2: set all the longitudes to be HIGH, so that everything's shut off
        for (int j=22; j<=52; j++) {
         digitalWrite(j, HIGH);
        }
        
             // we deal with 3 cases: sweeps, hemispheres and quadrants
     switch(lat[0]) {
         
       case 'm':{
         Slider = String(longit).toInt();
         // Serial.println(Slider);
         break;
       }
       case 't':{
          interval= longit.toInt();
          //Serial.println(longit);
         break;
       }
       
       case 's': {
         
         // this is the case of sweeping a single longitude. 
         // step 1: we put the correspoding longitude pin LOW and prepare it for the inevitable...
         longitudeInt = longit.toInt();
         // Serial.println(longitudeInt);
         digitalWrite(longitudeInt, LOW);
         
         // step 2: depending on whether the chosen semi-meridian is a long or a short one (at the entrance), we need to choose a seperate starting LED for the sweep         
         if (longitudeInt >= 23 && longitudeInt <= 37 && longitudeInt%2 == 1) {
//  ****          // Serial.println("odd in range");
           sweepStart = 5;
         } else {
//****           // Serial.println("even in range");
           sweepStart = 9;
         }
         b = sweepStart+1;    // an extra 1 added becaus the first thing that's done is b--
         sweep = true;
         break;
       }
       case 'l':{
       digitalWrite(fixationLED,LOW);
       for (int j=22; j<=52; j++) {
         digitalWrite(j, LOW);
        }
        longitudeInt = longit.toInt();
       if(longitudeInt <8)
       {
       analogWrite(longitudeInt,Slider);
       }
   }
     case 'f':{         
       digitalWrite(fixationLED,LOW);
         // we then switch through WHICH hemisphere
         switch(longit[0]){
           case '1': {
             // THis is the hemisphere case. Turn on all the latitudes..
             for (int p=2; p<=10; p++) {
               analogWrite(p, Slider);
             }
             // LEFT hemisphere.. turn on U to X (31,37) and A to I (22,38). eliminated the 90 degree and 270 degree by eliminating 11 and 15
             // first we put on 2*(11,19)
             for (int q=11; q<=26; q++) {
               digitalWrite(2*q, LOW);
             }
             // then we put on 2*(15,18)+1
             for (int r=11; r<=18; r++) {
               digitalWrite((2*r+1), LOW);
             }
             break;
           }
     }
     break;
   }
       case 'h': {     
        digitalWrite(fixationLED,LOW);
         // we then switch through WHICH hemisphere
         switch(longit[0]){
           case 'l': {
             // THis is the hemisphere case. Turn on all the latitudes..
             for (int p=2; p<=10; p++) {
               analogWrite(p, Slider);
             }
             // LEFT hemisphere.. turn on U to X (31,37) and A to I (22,38). eliminated the 90 degree and 270 degree by eliminating 11 and 15
             // first we put on 2*(11,19)
             for (int q=11; q<=18; q++) {
               digitalWrite(2*q, LOW);
             }
             // then we put on 2*(15,18)+1
             for (int r=16; r<=18; r++) {
               digitalWrite((2*r+1), LOW);
             }
             break;
           }
           case 'r': { 
             // THis is the hemisphere case. Turn on all the latitudes..
             for (int p=2; p<=10; p++) {
               analogWrite(p, Slider);
             }
             // RIGHT hemisphere.. turn on J to T.. which is (40,52) in steps of 2 and (23, 29) in steps of 2
             // first we put on 2*(20,26)                                                    eliminated the 90 degree and 270 degree
             for (int q=20; q<=26; q++) {
               digitalWrite(2*q, LOW);
             }
             // then we put on 2*(11,14)+1
             for (int r=11; r<=14; r++) {
               digitalWrite((2*r+1), LOW);
             }
             break;  
           }
           // 30 degrees and outer case:
           case 'a': {
             for (int p=4; p<=10; p++) {
               analogWrite(p, Slider);
             }
             // LEFT hemisphere.. turn on U to X (31,37) and A to I (22,38).
             // first we put on 2*(11,19)
             for (int q=11; q<=18; q++) {
               digitalWrite(2*q, LOW);
             }
             // then we put on 2*(15,18)+1
             for (int r=16; r<=18; r++) {
               digitalWrite((2*r+1), LOW);
             }
             break;
           }
           case 'b': { 
             for (int p=4; p<=10; p++) {
               analogWrite(p, Slider);
             }
             // RIGHT hemisphere.. turn on J to T.. which is (40,52) in steps of 2 and (23, 29) in steps of 2
             // first we put on 2*(20,26)
             for (int q=20; q<=26; q++) {
               digitalWrite(2*q, LOW);
             }
             // then we put on 2*(11,14)+1
             for (int r=11; r<=14; r++) {
               digitalWrite((2*r+1), LOW);
             }
             break;  
           }
         }
         break;
       }
       case 'q': {
         // quadrants..
         digitalWrite(fixationLED,LOW);
         switch(longit[0]) {
           // we shall go anticlockwise. "1" shall start from the bottom right. 
          case '1': {
            // latitudes.. all on
            for (int s=2; s<=10; s++) {
               analogWrite(s, Slider);
            }
            // the bottom right. O (50 to 52) to T (23 to 29).
            // then we put on 2*(11,14)+1
            for (int r=11; r<=14; r++) {
              digitalWrite((2*r+1), LOW);
              delay(1);
            }
            for (int q=25; q<=26; q++) {
               digitalWrite(2*q, LOW);
               delay(1);
             }
            break;
          } 
          case '2': {
            // latitudes.. all on
            for (int s=2; s<=10; s++) {
               analogWrite(s, Slider);
            }
            // the top right. I to N (38 to 48) 
            for (int q=20; q<=24; q++) {
               digitalWrite(2*q, LOW);
             }
            break;
          } 
          case '3': {
            // latitudes.. all on
            for (int s=2; s<=10; s++) {
               analogWrite(s, Slider);
            }
            // the top left. C to H. (26 to 36).
            for (int q=13; q<=18; q++) {
               digitalWrite(2*q, LOW);
               delay(1);
             }
            break;
          } 
          case '4': {
            // latitudes.. all on
            for (int s=2; s<=10; s++) {
               analogWrite(s, Slider);
            }
            // the bottom left. U to X (31 to 37), A to B (22, 24)
            // then we put on 2*(11,14)+1
            for (int r=16; r<=18; r++) {
              digitalWrite((2*r+1), LOW);
              delay(1);
            }
            for (int q=11; q<=12; q++) {
               digitalWrite(2*q, LOW);
               delay(1);
             }
            break;
          } 
          case '5': {
            // turn on only the 30 degrees and higher latitudes
            for (int s=4; s<=10; s++) {
              analogWrite(s, Slider);
            }
            // outer 30 degrees bottom right
            for (int r=11; r<=14; r++) {
              digitalWrite((2*r+1), LOW);
              delay(1);
            }
            for (int q=25; q<=26; q++) {
               digitalWrite(2*q, LOW);
               delay(1);
             }
            break;
          }
          case '6': {
            // turn on only the 30 degrees and higher latitudes
            for (int s=4; s<=10; s++) {
              analogWrite(s, Slider);
            }
            // outer 30 degrees bottom right
            for (int q=20; q<=24; q++) {
               digitalWrite(2*q, LOW);
             }
            break;
          }
          case '7': {
            // turn on only the 30 degrees and higher latitudes
            for (int s=4; s<=10; s++) {
              analogWrite(s, Slider);
            }
            // the top left. C to H. (26 to 36).
            for (int q=13; q<=18; q++) {
               digitalWrite(2*q, LOW);
               delay(1);
             }
            break;
          }
         case '8': {
            // turn on only the 30 degrees and higher latitudes
            for (int s=4; s<=10; s++) {
              analogWrite(s, Slider);
            }
            // the bottom left. U to X (31 to 37), A to B (22, 24)
            // then we put on 2*(11,14)+1
            for (int r=16; r<=18; r++) {
              digitalWrite((2*r+1), LOW);
              delay(1);
            }
            for (int q=11; q<=12; q++) {
               digitalWrite(2*q, LOW);
               delay(1);
             }
            break;
          } 
         }
         break;
       }
     } 
        
        if (longit[0] == 'x') {
          digitalWrite(fixationLED,HIGH);
          breakOut = true;  // break out of the loops yo
          // reset everytnig...
          sweep = false;
          b=0;
 //***         Serial.println("breaking out");
          // break;
        }
        //else if (longit[0] == 'h'||longit[0] == 'q'||longit[0] == '1'||longit[0] == '2'||longit[0] == '3'||longit[0] == '4'||longit[0] == 'l'||longit[0] == 'r')
        //digitalWrite(fixationLED, HIGH);
        else
        digitalWrite(fixationLED,LOW);
        // lightHerUp(lat, longit);  // write to arduino
        // reset that shit
        inputString = "";
        
        
        
      } 
           
      else {
        
        inputString += inChar;
      }
    }
  }
}
