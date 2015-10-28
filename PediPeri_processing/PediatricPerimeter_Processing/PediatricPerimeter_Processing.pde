/***************************************
THIS IS THE LATEST VERSION AS OF 28-oct-2015
Written By :Karthik Reddy, Dhruv Joshi
Project Name :Pediatric Perimeter
The Project is Intended to caliberate the field of vision and other few vision related issues in Babies under a year 
There are three main parts of the Program Basically the custom Buttons,Video, The input for the Doctor to enter the description and details of the patient 
->There are Three Text Field For Age,Name,Description
->One DropDown List 
->Four Bangs(Buttons which Act as Functions)
->The Inputs are taken to a text file(.txt) and are written to a File and when the Button Save is Pressed
->Later these files can be viewed by Doctors as a Reference
->The Video is Captured using the GS Video Library and The Saving of the video Captured is Recorded by the Controls by the Bangs Start and Stop 
->The File name of the Video can be Controlled and the Quality of the Video is kept Low as the File Size of the Video is more if the Quality is kept more
->The Buttons which represent the segments of the Perimeter can be clicked on the respective Segment ,the clicked segment will displayed on the Screen 
*/
import processing.serial.*;
import codeanticode.gsvideo.*;  // Importing GS Video Library 
import controlP5.*;             // Import Control P5 Library

Serial arduino;                 // create serial object
int kk=0;

PrintWriter output;             // The File Writing Object

int ext = 0, me=-1;             // "me" tracks the meridian number
String azimuth,m,time="1000";                // converts "me" to the azimuth, which is used for preparing the proper isopter
boolean detailsEntered = false, videoRecording = false,timeStampDone=true,testStopped=true,isMeridian= false;    // These booleans follow whether information's been entered and when to start the video

// These variables relate to hte ellipses that indicate current LED position.
int i = 25, xi, yi;             // Store position of the LED, but not for long.
float theta;                    // This is the azimuthal angle on the perimeter "polar diagram"
int[] perimeter = 
{ -1, -1, -1, -1,   
  -1, -1, -1, -1,   
  -1, -1, -1, -1,
  -1, -1, -1, -1,
  -1, -1, -1, -1,
  -1, -1, -1, -1};           // THe most important variable in this whole project
// The perimeter shall store the radial positions (discrete) of the LEDs presently, which wil come from feedback from the arduino as it sweeps. The cardinal order of the elements indicates the azimuthal angle (discrete). There are 24 elements.
int[] hemquad = {0, 0, 0, 0, 0, 0};  // this stores the alpha value of the hemisphere and quadrants. When one is clicked, it just puts that damn value.
int Brightness = 100;
// controlp5 related objects
ControlP5 cp5;                  // Control P5 Object Creation     
ControlTimer c;
Textlabel t;
DropdownList d1;                // Dropdown List creation

String folderName = "";           // Will store the folder name into which shit will be saved

// te following bariables shall hold the values that were entered about the patient
String textValue = "";
String textFile = "";           
String textName = "";
String textAge = "";
String textSex = "";
String textEMR = "";
String textVideo="Please Fill the name and click on SAVE.";
String textDescription = "";

// These will hold the timer variables, for teh realtime clock in the video etc
String textTimer="";
String textDate="";
String textTime="";
String textMe="";

// button colour map variables...
PImage buttonm;       // image of the buttons (visible)
PImage buttoncolmap;  // colormap of the buttons (hidden)

// movie/video related variables
GSCapture cam;        // GS Video Capture Object
GSMovieMaker mm;      // GS Video Movie Maker Object  

int fps = 7;          // The Number of Frames per second Declaration
int ang = 0;
//Declaration of the names for the buttons and their parameters 
String[] buttonstring= {
  "48", "46", "44", "42", 
  "40", "38", "36", "34", 
  "32", "30", "28", "26", 
  "24", "22", "37", "35", 
  "33", "31", "29", "27", 
  "25", "23", "52", "50", 
  "l", "r", 
  "3", "2", "1", "4"
}; //the names of the buttons

color[] buttoncolor= {
  0xFF7D8075, 0xFF6F686F, 0xFF7E0516, 0xFFB97A57, 
  /**/ 0xFFF0202E /**/, 0xFFFEAEC7, 0xFFF78525, /**/ 0xFFFFC10A /**/,   // changed to reflect what processing sees
  0xFFCC00FF, 0xFFEFE3AF, 0xFF23B14D, 0xFFB5E51D, 
  /**/ 0xFF00A3E8 /**/, 0xFF9AD9EA, 0xFF3F47CE, 0xFF7092BF,   // changed to reflect what processing sees
  0xFFA349A3, 0xFFC7BFE6, 0xFF417B7D, /**/ 0xFFFF0080 /**/,   // changed to reflect what processing is seeing
  0xFF838ADB, 0xFFDA9D80, 0xFF86AADE, 0xFFA3D981,
  0xFF40003F, 0xFFA4A351,
  0xFF000079, 0xFF870C3A, 0xFF55761F, 0xFF457894
  
}; //the colors of the buttons
String textfield=""; // Text field String for display

void setup() {
  // going to initiate serial connection...
  if (Serial.list().length != 0) {
    println("Arduino MEGA connected succesfully.");
    String port = Serial.list()[0];
    // then we open up the port.. 9600 bauds
    arduino = new Serial(this, port, 9600);
    arduino.buffer(1);
  } else {
    println("Arduino not connected or detected, please replug"); 
    exit();
  }
 
  size(1300, 600);  //The Size of the Panel 
   
  cp5 = new ControlP5(this);
  t = new Textlabel(cp5,"--",840,20);
  buttonm = loadImage("buttonm.png");//Front End
  
  c = new ControlTimer();
  c.setSpeedOfTime(1);
  cp5.setColorLabel(0xff000000);
  d1 = cp5.addDropdownList("Sex") //The DropDown List With name Se
    .setPosition(20, 450)
      ;
  customize(d1); // customize the first list
  
  cp5.addTextfield("Name") //Text Field Name and the Specifications
    .setPosition(20, 100)
      .setSize(200, 30)
        //.setFont(font)
        .setFocus(true)
          .setFont(createFont("arial", 16))
            .setAutoClear(false)
              //.setColorCursor(0)
              ;
  // the next one is the serial no of the patient.. added 30-10-2014 on Sourav's suggestion.
  // changed on 26-feb-2015 to EMR No.
  cp5.addTextfield("EMR No")
    .setPosition(20, 250)
      .setSize(200, 30)
          .setFont(createFont("arial", 16))
            .setAutoClear(false)
              ;

  cp5.addTextfield("Age") //Text Field Age and the Specifications
    .setPosition(20, 170)
      .setSize(200, 30)
        .setFont(createFont("arial", 16))
          .setAutoClear(false)
              ;

  cp5.addTextfield("Description") //Text Field Description and the Specifications
    .setPosition(20, 340)
      .setSize(200, 30)
        .setFont(createFont("arial", 16))
          .setAutoClear(false)
           ;
cp5.addTextfield("Time") //Text Field Description and the Specifications
    .setPosition(360, 550)
      .setSize(50,25)
        .setFont(createFont("arial", 16))
          .setAutoClear(false)
           .setText("1000")
           ;
  cp5.addBang("clear") //The Bang Clear and the Specifications
    .setPosition(110, 20)
      .setSize(80, 40)
        .getCaptionLabel().align(ControlP5.CENTER, ControlP5.CENTER) //Caption and the alignment
          ;  
 
  cp5.addBang("Save")  //The Bang Save and the Specifications
    .setPosition(20, 20)
      .setSize(80, 40)
        .getCaptionLabel().align(ControlP5.CENTER, ControlP5.CENTER)
          ; 
       cp5.addBang("Random")  //The Bang starts the test by giving random lights
    .setPosition(1075, 445)
      .setSize(40, 40)
        .getCaptionLabel().align(ControlP5.CENTER, ControlP5.CENTER)
          ;      
  cp5.addBang("Stop")
    .setPosition(120, 540)
      .setSize(80, 40)
        .getCaptionLabel().align(ControlP5.CENTER, ControlP5.CENTER)
          //.setColor(0)
            ; 
        /**/ cp5.addBang("Send")
    .setPosition(820, 540)
      .setSize(30, 30)
        .getCaptionLabel().align(ControlP5.CENTER, ControlP5.CENTER)
          //.setColor(0)
            ;/**/       
           cp5.addSlider("Brightness")
     .setPosition(500,550)
     .setSize(220,20)
     .setRange(50,255)
     .setNumberOfTickMarks(4)
     ;
     
  // the fixation button...
  cp5.addBang("Fixation") //The Bang Fixation and the Specifications
    .setPosition(1075, 545)
      .setSize(40, 40)
        .getCaptionLabel().align(ControlP5.CENTER, ControlP5.CENTER) //Caption and the alignment
          ; 
     
  frameRate(fps);  //The Frames per second of the Video
  String[] cameras = GSCapture.list(); //The avaliable Cameras
     
  // We check if the right camera is plugged in, and if so only then do we proceed, otherwise we exit the program.
  if (cameras.length == 0) {
    println("There are no cameras available for capture.");
    //exit();
  } else {
    println("Checking if correct camera has been plugged in ...");
    
    for (int i = 0; i < cameras.length; i++) {  //Listing the avalibale Cameras
      // println(cameras[i].length());
      if (cameras[i].length() == 24 && cameras[i].substring(10,17).equals("LifeCam")) {
        print("...success!\n");
        cam = new GSCapture(this, 640, 480, cameras[i]);      // Camera object will capture in 640x480 resolution
        cam.start();      // shall start acquiring video feed from the camera
        break; 
      } 
      println("...NO. Please check the camera connected!"); 
      exit();
    }  
  }
}

void draw() {
  // The shit that has to be done each time...
  background(buttonm);//BackgEnd
    text(textTimer, 700, 530); 
  fill(0);
  //textFont(createFont("arial", 16), 16);
  text(textVideo, 320, 530); 
  text(textValue, 320, 510);
 text(textTimer, 400, 530); 

  // text(textfield, 1050, 440); 
  // text(textMe, 1110, 440);
  
    
  // The following is the red ellipse being printed to indicate which LED is on
  for (int p = 0; p < 24; p++) {
    if (perimeter[p] > 1) {
      fill(255,0,0);  // set the color to RED
      // We will calculate the x,y position that the particular LED is supposed to be at...
      ang = int(((p + 1)*15));    // we will be re-drawing each time for all meridians. This is due to the refresh-methodology on which processing operates.
      float multipler = 25 + (perimeter[p] - 2)*22 ;
      int x = int(1096.00 + cos(radians(ang))* multipler);    
      int y = int(201.00  - sin(radians(ang))* multipler);
      ellipse(x, y, 10, 10);                                      // LED PRINT ON THE PERIMETER
    }
  }
  // drawing a rectangle on top of the quadrants which are done
  stroke(195, 195, 195, 255);    // 0 < alpha < 255
  fill(195, 195, 195, hemquad[2]);
  rect(923,452,66,62);
  fill(195, 195, 195, hemquad[1]);
  rect(991,452,66,62);
  fill(195, 195, 195, hemquad[3]);
  rect(923,516,66,62);
  fill(195, 195, 195, hemquad[0]);
  rect(991,516,66,62);
  
  // drawing a rectangle on top of the hemispheres which are done..
  stroke(195, 195, 195, 255);
  fill(195, 195, 195, hemquad[4]);
  rect(1123,450,66,125);
  fill(195, 195, 195, hemquad[5]);
  rect(1192,450,66,125);
  
 

  if(detailsEntered == true) {    // Has the doctor entered the details which are required?
        
    // start showing the camera feed...
    if (cam.available() == true) {
      cam.read();
      image(cam, 245, 0);
     
stroke(255,0,0);
//strokeWeight(4);  // Thicker

 //fill(0,0,255);
line(555,240,575,240);
line(565,230,565,250);
point(565,240);
      PImage videoSection = get(245, 0, 1055, 600);    // crop our section of interest of the page
      videoSection.loadPixels();    // Loads the pixel data for the *CROPPED* display window into the pixels[] array. This function must always be called before reading from or writing to pixels[].
      
      if (videoRecording == true) {
        mm.addFrame(videoSection.pixels);  // Array containing the values for all the pixels in the display window.
      } 
    }
//strokeWeight(1);
  }    // Add window's pixels to movie
  
  // Here we continuously update the timedate on the screen...
  t.setValue(day()+ "-" + month() + "-" + year() + "\n" + hour() + ":" + minute() + ":" + second());
  t.draw(this);
  t.setPosition(20,480);
  t.setColorValue(0x000000);
  t.setFont(createFont("Arial",14));

 
  thread("timerData");
   
  
}
public void Send(){
        arduino.write('m'); //Brightness Slider
        arduino.write(',');
        arduino.write(m);
        //println("slider = " + int(m));
        arduino.write('\n');
        
         time=cp5.get(Textfield.class, "Time").getText();
        arduino.write('t');//Time Slider
        arduino.write(',');
        arduino.write(time);
        //println("time = " + time);
        arduino.write('\n');
        
}
public void clear() {    //Bang Function for the Button Clear
  // This function deals with what happens when you click on "CLEAR"
  cp5.get(Textfield.class, "Name").clear();
  cp5.get(Textfield.class, "Age").clear();
  //cp5.get(Textfield.class,"Sex").clear();
  cp5.get(Textfield.class, "Description").clear();
  //
}

public void Random(){
textValue = "Randomize the Gaze of the baby";
  arduino.write('h');
        arduino.write(',');
        arduino.write('r');
        arduino.write('\n');
delay(1500);
        arduino.write('h');
        arduino.write(',');
        arduino.write('l');
        arduino.write('\n');
delay(1500);
        arduino.write('q');
        arduino.write(',');
        arduino.write('2');
        arduino.write('\n');
delay(1500);
arduino.write('x');
        arduino.write('\n');
}
public void Save() {//Bang Function for the Button Save
  // Clicking SAVE
 textName = cp5.get(Textfield.class, "Name").getText();// these text fields need not be refreshed each and every time hence they are updated only when we save the files hence they are moved from draw()
  textAge = cp5.get(Textfield.class, "Age").getText();// updated on 5-may-2015
  textEMR = cp5.get(Textfield.class, "EMR No").getText();
  textDescription = cp5.get(Textfield.class, "Description").getText();
 
  if (textName.isEmpty()){//Do not Create a file if there is no name assigned to the File
    textValue="No File Created" ;
    textVideo="Please Enter the File Name to see the Video";
    detailsEntered = false;
  } else {
    // First, create the folder name into which everything will be stored (including the subsequent videos)...
    folderName = year()+"/"+month()+"/"+textName;
    
    //Writing the input texts to a .txt file 
    output = createWriter(folderName + "/" + textName+".txt"); 
    output.print("Date: " + day() + "/" + month() + "/" + year() + "\t\t\t");
    output.println("Time: " + hour() + ":" + minute() +":" + second() + "\n\n");
    output.println("Patient Name :" + textName);
    output.println("Patient Age :" + textAge);
    output.println("Patient Sex :" + textSex);
    output.println("Patient EMR No :" + textEMR);
    output.println("Patient Description :" + textDescription);
    output.println("\n\r\n\r\n\r" + "##############################" + "\n\r\n\r" + "PATIENT RESULTS" + "\n\r\n\r" + "##############################");
    output.println("TEST\t\t\tSTART\t\tStop\t\tAngleStopped\t\tBrightness\tDelay");
    output.flush(); // Writes the remaining data to the file
    // output.close(); // File written, all's well
    
    // notify the user..
    textValue = "File Created with "+textName+".txt  as the Name";
    textVideo = "Thank you. Video is ON. Please click on a test..";
    detailsEntered = true;      // Details have been entered. Awesome. Show the video.
    
    mm = new GSMovieMaker(this, width-245, height, folderName + "/" + year() + "" + month() + "" + day() + "_" + textName + ".avi", GSMovieMaker.MJPEG, GSMovieMaker.LOW, fps); // the Mavie Maker Object
    mm.setQueueSize(50, 10);
    videoRecording = true;        // start recording the video.
    // then we start the video..
    mm.start();               // Starting the Pictures
  }
cp5.controller("Save").hide();
cp5.controller("clear").hide();
}

public void Stop(){
  // this function stops the video taking and also stops the present operation on the arduino.
  // mm.finish(); // Completes the Video at this Instant
  
  textVideo = "Test has stopped. All lights OFF.";
  kk = 0;
  if(timeStampDone == false) {
    output.print("\t" + hour() + ":" + minute() +":" + second()+":" + millis()); 
    output.print("\t\t" + textTimer + "\t");
    // write the presently completed meridian to the file...
    if (isMeridian==true) {  // checking if it's a meridian or not...
      output.print( "\t" + ((perimeter[me] )-1)*10+"\t\t"+m+"\t\t"+time);
    } else {  // the case where it's not a meridian
      output.print( "\t\t\t"+m);
    }
    output.println();
    output.flush();
    timeStampDone = true;
  }
  
  arduino.write('x');
  arduino.write('\n');
  
  
  // overwrite the isopter image to reflect what's currently been done...
  PImage isopter = get(890, 0, 410, 380);     // get that particular section of the screen where the isopter lies.
  isopter.save(folderName + "/isopter.jpg");  // save it to a file in the same folder
}

void mousePressed() {
   //println(mouseX+" "+mouseY);
  // This part presumable deals with the LED indication being printed on the screen...
  // When one clicks on the perimeter sweep diagram, of course...
  //println(mouseX +" "+ mouseY);
  
    if (detailsEntered == true) {
        m = str(Brightness);
        textVideo="The test has Started"; 
       
      float r = sqrt(sq(mouseX - 1096) + sq(mouseY - 200));    // radial distance from the center of the perimeter and the mouse 
   //println(r);
  float r1 = sqrt(sq(mouseX - 991) + sq(mouseY - 516));   
     //println(r1);
  float r2 = sqrt(sq(mouseX - 1191) + sq(mouseY - 516));   
  //println(r2);
  
  if (r <= 213 ) {  // If the mouse hath clicked in the general sweep region.. remember we're just trying to find theta here
    // println("chose a semi-meridian");
    isMeridian=true;
    // The next 3 lines simply find the azimuthal angle
    theta = (float(mouseY) - 200)/ (float(mouseX) - 1096);
    theta = atan(theta);
    theta = degrees(theta);
    
    // Then we choose the sign of theta based on standard polar coordinates convention
    if(mouseX>1096  && mouseY<200)
      theta= -1*theta;
    else if(mouseX<1096)// && mouseY<200)
      theta = 180 - theta; 
    else if(mouseX>1096 && mouseY>200)
      theta = 360 - theta;
    
    // What's next? discretization of theta into a variable that represents which LED is on..  
    float a = ((theta  -7.5)/15);
    
    if (a < 0) {
      a = 23;                    // The single meridian which is at the end is numbered 23
    }
     me = int(a);    // The variable "me" tracks the meridian number, by discretizing "a"
    //println(me);
  
     azimuth = str(((me + 1)*15)%360);    // calculate the azimuth angle which has actually been mentioned from the 'me' variable
      // we will check the different cases for i...
      if (int(buttonstring[me]) >= 22) {  
        // the following resets the timer..
        kk++;
        if (kk == 1) {
          c.reset();
          
        }
        
        // println("sweep");
        // this is the case of the sweeps..
        Stop();
        arduino.write('s');
        arduino.write(',');
        arduino.write(buttonstring[me]);
        arduino.write('\n');
        textValue = "kinetic perimetry, Meridian " + azimuth + " degrees";
        if (timeStampDone == true) {
          output.print("Meridian " + azimuth); 
         output.print("\t\t" + hour() + ":" + minute() +":" + second()+":" + millis()+"\t"); 
          output.flush(); // Writes the remaining data to the file
          timeStampDone = false;  
        }
      }
   
    
    //println(azimuth);
    // Now we know whch meridian's been selected.
    // println("Meridian" + me);
    textMe = ("Angle: " + theta + " degrees");    // print to the textfield indicating which meridian was selected
    // ang = int(((me+1)*15) /*+ 7.5*/);    // The 7.5 degrees has been removed so that the red dot will come in the center of the meridian, which is more representative of reality
        
  } 
  else if(r1<66){
    isMeridian=false;
    theta = (float(mouseY) - 516)/ (float(mouseX) - 991);
    theta = atan(theta);
    theta = degrees(theta);
    
    // Then we choose the sign of theta based on standard polar coordinates convention
    if(mouseX>991  && mouseY<516)
      theta= -1*theta;
    else if(mouseX<991)// && mouseY<200)
      theta = 180 - theta; 
    else if(mouseX>991 && mouseY>516)
      theta = 360 - theta;
    kk++;
        if (kk == 1) {
          c.reset();
        }
  if(r1>33){
  
  if (theta <=90){    
        Stop();    
        arduino.write('q');
        arduino.write(',');
        arduino.write('2');
        arduino.write('\n');
        textValue = "Quadrant " + "top right";
         if (timeStampDone == true) {
          output.print("Quadrant");
            output.print(" top right");
             timeStampDone = false;
           output.print("\t" + hour() + ":" + minute() +":" + second()+":" + millis());  
         output.flush(); // Writes the remaining data to the file
       }
        hemquad[2 - 1] = 200;  // set that particular quadrant to 'done' 
  }
  else if(theta>90 && theta <=180){
        Stop();
        arduino.write('q');
        arduino.write(',');
        arduino.write('3');
        arduino.write('\n');
        textValue = "Quadrant " + "top left";
         if (timeStampDone == true) {
          output.print("Quadrant");
            output.print(" top left");
               timeStampDone = false; 
                output.print("\t" + hour() + ":" + minute() +":" + second()+":" + millis()); 
       output.flush(); // Writes the remaining data to the file  
        }
        hemquad[3 - 1] = 200;  // set that particular quadrant to 'done'
  }
  else if(theta>180 && theta <=270){
        Stop();
        arduino.write('q');
        arduino.write(',');
        arduino.write('4');
        arduino.write('\n');
        textValue = "Quadrant " + "bottom left"; 
         if (timeStampDone == true) {
          output.print("Quadrant");
            output.print(" bottom left");
             timeStampDone = false;
           output.print("\t" + hour() + ":" + minute() +":" + second()+":" + millis());   
       output.flush(); // Writes the remaining data to the file  
       }
        hemquad[4 - 1] = 200; } // set that particular quadrant to 'done'}
  else if(theta>270 && theta <=360){
        Stop(); 
        arduino.write('q');
        arduino.write(',');
        arduino.write('1');
        arduino.write('\n');
        textValue = "Quadrant " + "bottom right";
         if (timeStampDone == true) {
          output.print("Quadrant");
            output.print(" bottom right");
   timeStampDone = false; 
output.print("\t" + hour() + ":" + minute() +":" + second()+":" + millis()); 
output.flush(); // Writes the remaining data to the file
}
 }
  }
  else if(r1<33){
    if (theta <=90){        
        Stop();
        arduino.write('q');
        arduino.write(',');
        arduino.write('6');
        arduino.write('\n');
        textValue = "Quadrant " + "2 inner";
         if (timeStampDone == true) {
          output.print("Quadrant");
            output.print(" top right inner");
             timeStampDone = false;
           output.print("\t" + hour() + ":" + minute() +":" + second()+":" + millis());  
      output.flush(); // Writes the remaining data to the file  
       }
        hemquad[2 - 1] = 200;  // set that particular quadrant to 'done' 
  }
  else if(theta>90 && theta <=180){
        Stop();
        arduino.write('q');
        arduino.write(',');
        arduino.write('7');
        arduino.write('\n');
        textValue = "Quadrant " + "top left inner";
         if (timeStampDone == true) {
          output.print("Quadrant");
            output.print(" top left inner");
                           timeStampDone = false; 
            output.print("\t" + hour() + ":" + minute() +":" + second()+":" + millis()); 
       output.flush(); // Writes the remaining data to the file  
        }
        hemquad[3 - 1] = 200;  // set that particular quadrant to 'done'
  }
  else if(theta>180 && theta <=270){
        Stop();
        arduino.write('q');
        arduino.write(',');
        arduino.write('8');
        arduino.write('\n');
        textValue = "Quadrant " + "bottom left inner";
         if (timeStampDone == true) {
          output.print("Quadrant");
            output.print(" bottom left inner");
             timeStampDone = false;
           output.print("\t" + hour() + ":" + minute() +":" + second()+":" + millis()); 
       output.flush(); // Writes the remaining data to the file  
       }
        hemquad[4 - 1] = 200; } // set that particular quadrant to 'done'}
  else if(theta>270 && theta <=360){
        Stop();
        arduino.write('q');
        arduino.write(',');
        arduino.write('5');
        arduino.write('\n');
        textValue = "Quadrant " + "bottom right inner";
         if (timeStampDone == true) {
          output.print("Quadrant");
            output.print(" bottom right inner");
   timeStampDone = false; 
output.print("\t" + hour() + ":" + minute() +":" + second()+":" + millis()); 
output.flush(); // Writes the remaining data to the file
}
 hemquad[1 - 1] = 200;}
  }
}      
  else if(r2<66 ){
    isMeridian=false;
     kk++;
        if (kk == 1) {
          c.reset();
        }
    theta = (float(mouseY) - 516)/ (float(mouseX) - 991);
    theta = atan(theta);
    theta = degrees(theta);
    
    // Then we choose the sign of theta based on standard polar coordinates convention
    if(mouseX>1191  && mouseY<516)
      theta= -1*theta;
    else if(mouseX<1191)// && mouseY<200)
      theta = 180 - theta; 
    else if(mouseX>1191 && mouseY>516)
      theta = 360 - theta; 
   if(r2<33){
     if (theta >90 && theta<270){
        Stop();
        arduino.write('h');
        arduino.write(',');
        arduino.write('a');
        arduino.write('\n');
        textValue = "Hemisphere " + "left inner";
        if (timeStampDone == true) {
          output.print("Hemisphere");
         output.print(" left inner");
           output.print("\t" + hour() + ":" + minute() +":" + second()+":" + millis()); 
          timeStampDone = false;  
          output.flush(); // Writes the remaining data to the file
        }
       
          hemquad[4] = 200;
        }
        else {
        Stop();
        arduino.write('h');
        arduino.write(',');
        arduino.write('b');
        arduino.write('\n');
        textValue = "Hemisphere " + "right inner";
        if (timeStampDone == true) {
          output.print("Hemisphere");
         output.print(" right inner");
           output.print("\t" + hour() + ":" + minute() +":" + second()+":" + millis()); 
          timeStampDone = false;  
     output.flush(); // Writes the remaining data to the file  
      }
      
          hemquad[5] = 200;
        }}
        else{
        if (theta >90 && theta <270){
        Stop();
        arduino.write('h');
        arduino.write(',');
        arduino.write('l');
        arduino.write('\n');
        textValue = "Hemisphere " + "left";
        if (timeStampDone == true) {
          output.print("Hemisphere");
         output.print(" left ");
 
          output.print("\t" + hour() + ":" + minute() +":" + second()+":" + millis()); 
          timeStampDone = false;  
     output.flush(); // Writes the remaining data to the file  
      }
       
          hemquad[4] = 200;
        }
        else {
        Stop();
        arduino.write('h');
        arduino.write(',');
        arduino.write('r');
        arduino.write('\n');
        textValue = "Hemisphere " + "right ";
        if (timeStampDone == true) {
          output.print("Hemisphere");
         output.print(" right ");
 
         output.print("\t" + hour() + ":" + minute() +":" + second()+":" + millis()); 
          timeStampDone = false;  
     output.flush(); // Writes the remaining data to the file  
      }
      
          hemquad[5] = 200;
        }
     }
  }
  else textMe = "None";
}
else {
        textVideo="Please Enter the Patient name";
      }
}
   


void customize(DropdownList ddl) {
  // This part changes the properties of the MALE/FEMALE dropdown
  ddl.setBackgroundColor(color(190));
  ddl.setColorLabel(color(190));
  ddl.setItemHeight(40);
  ddl.setBarHeight(35);
  ddl.captionLabel().set("Sex");
  ddl.captionLabel().style().marginTop = 3;
  ddl.captionLabel().style().marginLeft = 3;
  ddl.valueLabel().style().marginTop = 3;
  ddl.addItem("Male", 0);
  ddl.addItem("Female", 1);
  ddl.scroll(0);
  ddl.setColorBackground(color(28,59,107));
  ddl.setColorActive(color(255, 128));
}

void controlEvent(ControlEvent theEvent) {
  // This part changes the value of variable textSex based on what's selected on the dropdown
  if (theEvent.isGroup()) {
    // check if the Event was triggered from a ControlGroup
    if (theEvent.getGroup().getValue()==0.0)
      textSex="Male";
    else
      textSex="Female";
  } else if (theEvent.isController()) {
    //println("event from controller : "+theEvent.getController().getValue()+" from "+theEvent.getController());
  }
}

void serialEvent(Serial arduino) { 
  String inString = arduino.readStringUntil('\n');
  if (inString != null) {
    // if (parseInt(inString.substring(0,1)) > 0) {    // we want to reject the last value "9" the arduino spits out from serial
      perimeter[me] = parseInt(inString.substring(0,1));  // write the number to the perimeter variable.
    // } 
  } 
}

// last but not least: we need to add a keypress functionality which checks if any key has been pressed and stops the test if so...
void keyPressed() {
  if (detailsEntered == true) {    // after the patient's data's been entered, ofc
    // println("key pressed");
    testStopped=true;
    Stop();
  } 
}

// the code that puts the fixation OFF..
public void Fixation(){
  arduino.write('x');
  arduino.write('\n');
}
void timerData(){
if(kk>=1){
textTimer = c.toString()+":"+c.millis();
println(textTimer);
}
}
