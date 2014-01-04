// A simple "snake-clone" called "Mask" (Worm in Swedish) 
// by Peter Parnes and Agneta Hedenström 
// Contact: peter@parnes.com 
// License: Just give us credit :) 
// Created: 2013-06-07
// Ported to Gameduino2 2014-01-04
// 
// Hardware needed 
// 1: Arduino  
// 2: Gameduino2

#include <EEPROM.h>
#include <SPI.h>
#include <GD2.h>

const int maxlength = 400;
const int initialLength = 10;
int length; 
const int height = 272;
const int width = 480;
int dir = 0; // 0 = right, 1 = up, 2  = left, 3 = down
int tick;
; // how fast should we go
const int initialTick = 30;
word masken[maxlength][2];
const int initialFoodLife = 150;
const int totalFoodLife = 1000;
const int increaseLength = 5;
const int foodSize = 4;
int lastDirection;
int foodLife;
boolean keys;
boolean autoplay;
boolean cheat;
boolean soundOn;
int score;

void setup() {
  Serial.begin(9600);       // initialize serial communication with your computer
  Serial.println("Hej Masken");
  GD.begin();
  
  initGame();
  soundOn = true; 
}

void initGame() {
  GD.ClearColorRGB(0x000000); // clear display
  GD.Clear();

  foodLife = initialFoodLife;
  length = initialLength;
  lastDirection = 0;
  tick = initialTick;
  autoplay = false;
  cheat = false;
  score = 10;

  // Draw splash screen text
  GD.SaveContext();
  GD.ColorRGB(0xffff00);
  // XXX tft.setTextSize(5);
  // tft.setCursor(22, 15);
  GD.cmd_text(240,64,31,OPT_CENTER,"Mask");
 
  // XXX tft.setTextSize(1);
  GD.cmd_text(240,100,31,OPT_CENTER,"By:  Agneta Hedenstrom ");
  GD.cmd_text(240,140,31,OPT_CENTER,"  and Peter Parnes");

  
  GD.RestoreContext();
  GD.ColorRGB(0xffffff);

  // XXX tft.println(" ");
  //tft.println(" Press left for Yoystick.");
  //tft.println(" Press right for Keys.");
  //tft.println(" Press down for Auto.");
  //tft.println(" Hold up for Cheat.");

  boolean sw1 = false;
  boolean sw2 = false;
  boolean sw3 = false;
  boolean sw4 = false;
  
  GD.swap();

  /*
  while(!sw2 && !sw3 && !sw4) {
    // New game when swtich 2 or 4 is pressed
    sw1 = Esplora.readButton(SWITCH_UP)==LOW;
    sw2 = Esplora.readButton(SWITCH_LEFT)==LOW;
    sw3 = Esplora.readButton(SWITCH_DOWN)==LOW;
    sw4 = Esplora.readButton(SWITCH_RIGHT)==LOW;
  }
  if(sw2) {
    keys = false;  
  } 
  else {
    keys = true;
  }
  if(sw3) {
    autoplay = true;
    cheat = true;
    tick = 1;
  }
  if(sw1) {
    cheat = true; 
  }
  */
  
  // XXX sätt motsvarade SW3 ovan
  autoplay =  true; 
  cheat = true; 
  tick = 1;

  GD.Clear();
  // drawBorder();   // Draw boarder

  long seed = 10000; // XXX Esplora.readLightSensor() * Esplora.readMicrophone() / Esplora.readTemperature(DEGREES_F) ;
  randomSeed(seed); // create seed because on Esplora there are no unused analog pins for random noise

  // init masken 
  for(int i = 0; i < length; i++) {
    masken[i][1] = height/2;
    masken[i][0] = width/2+i;
  }

  drawMasken();
}

void tone(int freq, int s) {
  // XXX 
}

void loop() { 
  // XXX 
  /* if(autoplay && Esplora.readButton(SWITCH_UP)==LOW) {
    drawEnd(false);
  }  */ 

  GD.Clear();

  GD.PointSize(16 * 5);
  GD.Begin(POINTS);
  for(int i = 1; i < length; i++) {
    masken[i-1][0] = masken[i][0];
    masken[i-1][1] = masken[i][1];
  }
  addHead();

  if(eatingFood()) {
    tone(3500, 50);
    updateFood(true);
    GD.ColorRGB(0x00ff00);
    GD.Vertex2ii(masken[length-1][0], masken[length-1][1]); 
    for(int i = 0; i < increaseLength; i++) {
      if(length < maxlength) {
        length++;
        addHead();
      }
      score++;
    } 

    tick--;
    if(tick < 1) {
      tick = 1; 
    }
  }

  drawMasken();

  updateFood(false);

  drawScore(false);

  // drawFoodLeftRGB();
  
  drawBorder();
 
  GD.swap();


  // delay(tick);
  

}

// draw RGB for time on food left.... Not so good :/ 
void drawFoodLeftRGB() {
  // int intensity = map(foodLife, 0, totalFoodLife, 0, 50);
  // Esplora.writeRGB(0,0,intensity);

}

void addHead() {
  //if(length == maxlength) {
  //  drawEnd(true);
  //} 

  // addNewMaskenElement
  dir = getDirection();
    
  int x;
  int y;
  switch(dir) {
  case 0: 
    y = 0;
    x = 1; 
    break;
  case 1: 
    y = 1;
    x = 0; 
    break;
  case 2: 
    y = 0;
    x = -1; 
    break;
  case 3: 
    y = -1;
    x = 0; 
    break;
  }

  masken[length-1][0] = masken[length-2][0] + y;
  masken[length-1][1] = masken[length-2][1] + x; 

  if(masken[length-1][0] == 0 || masken[length-1][0] == height-1 || masken[length-1][1] == 0 || masken[length-1][1] == width) {
    drawEnd(false); 
  }
  if(!cheat) {
    if(eatingMySelf()) {
      drawEnd(false);
    }
  }
}

boolean eatingMySelf() {
  for(int i = 0; i < length-1; i++) {   
    if(checkEqualDotDot(masken[length-1][0], masken[length-1][1], masken[i][0], masken[i][1])) {
      return true;
    }
  } 
  return false;
}

int food[] = { -1,-1 };
void updateFood(boolean forceNew) {
  foodLife--;  
  if(foodLife == 0 || forceNew) {
    food[0] = random(10, width-30);
    food[1] = random(10, height-30);
    foodLife = totalFoodLife;
  }
  GD.ColorRGB(0xffff00);
  GD.Vertex2ii(food[0], food[1]); 
}

boolean eatingFood() {
  return checkIntersectCirleDot(food[0], food[1], foodSize, masken[length-1][0], masken[length-1][1]);
}

void drawScore(boolean erase) {
  // XXX 
  
  /*
  tft.setRotation(1); 
  tft.setTextSize(1);
  tft.setCursor(2, 2);
  if(erase) {
    tft.print("   ");
  } 
  else {
    tft.print(score);
  }
  tft.setRotation(0);  
  */
}

void drawEnd(boolean outOfMem) {
  // XXX 
  
  /*
  for(int i = 0; i < 5; i++) {
    tft.invertDisplay(0);
    delay(100);
    tft.invertDisplay(1);
    delay(100);
  } 
  tft.invertDisplay(0);
  drawScore(true);
  tft.setRotation(1); 
  tft.setTextSize(1);
  tft.setCursor(0, 15);
  if(outOfMem) {
    tft.print(" Out of memory :(");
    tft.println();
    tft.println();
  }
  tft.print(" Score: ");
  tft.println(length);
  tft.println();
  tft.println(" Press Switch 4 To Restart");
  tft.setRotation(0); 
  drawBorder();

  while(Esplora.readButton(SWITCH_RIGHT)==HIGH) {
  };    
  initGame();
  */
}

int getDirection() {
  if(autoplay) {
    return getDirectionAuto();
  } 
  else if(keys) {
    return getDirectionKey(); 
  } 
  else {
    return getDirectionJoystick();
  }
}

int getDirectionAuto() {
  int diff0 = food[0] - masken[length-1][0];
  int diff1 = food[1] - masken[length-1][1];

  int dir = lastDirection;
  if(diff1 < 0) {
    dir = 2; 
  } 
  else if(diff1 > 0) {
    dir = 0; 
  } 
  else {
    if(diff0 < 0) {
      dir = 3; 
    } 
    else if(diff0 > 0) {
      dir = 1; 
    }
  }

  lastDirection = dir;
  return dir;
}

int getDirectionKey() {
  int dir = lastDirection;
  
  // XXX 
  /*
  if(!Esplora.readButton(SWITCH_LEFT)) {
    dir = 2;
  } 
  else if(!Esplora.readButton(SWITCH_UP)) {
    dir = 1;
  } 
  else if(!Esplora.readButton(SWITCH_RIGHT)) {
    dir = 0;
  } 
  else if(!Esplora.readButton(SWITCH_DOWN)) {
    dir = 3;
  } 
  lastDirection = dir;
  */
  return dir;
  
}

int getDirectionJoystick() {
  // XXX
  int dir = lastDirection;

  /*
  int x = Esplora.readJoystickX();       
  int y = Esplora.readJoystickY();   
  if(x > 20) {
    dir = 2;
  } 
  else if(y < -20) {
    dir = 1;
  } 
  else if(x < -20) {
    dir = 0;
  } 
  else if(y > 20) {
    dir = 3;
  } 

  / *
  Serial.print(x);
   Serial.print(" ");
   Serial.print(y);
   Serial.print(" ");
   Serial.println(dir);
   * /

  lastDirection = dir;
  */
  return dir;
}

void drawMasken() {
  GD.PointSize(16 * 5); 
  GD.ColorRGB(0x00ff00);
  GD.Begin(POINTS);
  for(int i = 0; i < length-1; i++) {
    GD.Vertex2ii(masken[i][0], masken[i][1]);
  }
  GD.ColorRGB(0xff0000);
  GD.Vertex2ii(masken[length-1][0], masken[length-1][1]); 
}

void drawBorder() {
  GD.SaveContext();
  GD.LineWidth(16*5);
  GD.Begin(LINES);
  GD.ColorRGB(0xffffff);
  GD.Vertex2ii(0,0); GD.Vertex2ii(width,0);
  GD.Vertex2ii(width,0); GD.Vertex2ii(width,height);
  GD.Vertex2ii(0,0); GD.Vertex2ii(0,height);
  GD.Vertex2ii(0,height); GD.Vertex2ii(width,height);
  GD.RestoreContext();
}

boolean checkEqualDotDot(int x1, int y1, int x2, int y2) {
  return ( x1 == x2 && y1 == y2);  
}

// Based on the Adafruit GFX-library for drawing a circle. See AdaFruit_GFX.h 
/******************************************************************
 * This is the core graphics library for all our displays, providing
 * basic graphics primitives (points, lines, circles, etc.). It needs
 * to be paired with a hardware-specific library for each display
 * device we carry (handling the lower-level functions).
 * 
 * Adafruit invests time and resources providing this open
 * source code, please support Adafruit and open-source hardware
 * by purchasing products from Adafruit!
 * 
 * Written by Limor Fried/Ladyada for Adafruit Industries.
 * BSD license, check license.txt for more information.
 * All text above must be included in any redistribution.
 ******************************************************************/
boolean checkIntersectCirleDot(int x0, int y0, int r, int dotX, int dotY) {

  if((abs(x0-dotX) > foodSize) || (abs(y0-dotY) > foodSize)) {
    return false;
  }

  int f = 1 - r;
  int ddF_x = 1;
  int ddF_y = -2 * r;
  int x = 0;
  int y = r;

  boolean intersect = false;

  intersect = checkEqualDotDot(x0, y0+r, dotX, dotY);
  if(intersect) {
    return true;
  }
  intersect = checkEqualDotDot(x0, y0-r, dotX, dotY);
  if(intersect) {
    return true;
  }
  intersect = checkEqualDotDot(x0+r, y0, dotX, dotY);
  if(intersect) {
    return true;
  }
  intersect = checkEqualDotDot(x0-r, y0, dotX, dotY);
  if(intersect) {
    return true;
  }

  while (x<y) {
    if (f >= 0) {
      y--;
      ddF_y += 2;
      f += ddF_y;
    }
    x++;
    ddF_x += 2;
    f += ddF_x;

    intersect = checkEqualDotDot(x0 + x, y0 + y, dotX, dotY);
    if(intersect) {
      return true;
    }
    intersect = checkEqualDotDot(x0 - x, y0 + y, dotX, dotY);
    if(intersect) {
      return true;
    }
    intersect = checkEqualDotDot(x0 + x, y0 - y, dotX, dotY);
    if(intersect) {
      return true;
    }
    intersect = checkEqualDotDot(x0 - x, y0 - y, dotX, dotY);
    if(intersect) {
      return true;
    }
    intersect = checkEqualDotDot(x0 + y, y0 + x, dotX, dotY);
    if(intersect) {
      return true;
    }
    intersect = checkEqualDotDot(x0 - y, y0 + x, dotX, dotY);
    if(intersect) {
      return true;
    }
    intersect = checkEqualDotDot(x0 + y, y0 - x, dotX, dotY);
    if(intersect) {
      return true;
    }
    intersect = checkEqualDotDot(x0 - y, y0 - x, dotX, dotY);
    if(intersect) {
      return true;
    }
  }

  return intersect;
}
























