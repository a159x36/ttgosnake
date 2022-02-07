
#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "sdkconfig.h"
#include <Arduino.h>

#include <TFT_eSPI.h>

#include <Button2.h>

#define BUTTON_1            35
#define BUTTON_2            0

TFT_eSPI tft = TFT_eSPI(135, 240);

uint16_t *image;
uint8_t *cells;
const int X = 135;
const int Y = 240;

const int CXY = 5;

const int GX = X / CXY;
const int GY = Y / CXY;

bool running = true;
int snake_delay = 150;

struct pos {
    uint8_t x,y;
};
pos make_pos(uint8_t x, uint8_t y) {
    pos result;
    result.x = x;
    result.y = y;
    return result;
}

pos *snake;
uint16_t length = 3;
uint16_t max_length = 100;

uint8_t in = 0;
uint8_t out = 1;

int x = X / 2;
int y = Y / 2;
uint8_t heading = 0;

pos target;
int counter = 0;

Button2 lButton(BUTTON_2);
Button2 rButton(BUTTON_1);

void setup() {
   Serial.begin(9600);
   Serial.write("setup\n");

    delay(100);

    image = new uint16_t[X*Y];
    cells = new uint8_t[GX*GY];
    snake = new pos[max_length];

    tft.init();
    tft.setRotation(1);
    tft.setSwapBytes(1);

    if (TFT_BL > 0) {                           // TFT_BL has been set in the TFT_eSPI library in the User Setup file TTGO_T_Display.h
        pinMode(TFT_BL, OUTPUT);                // Set backlight pin to output mode
        digitalWrite(TFT_BL, TFT_BACKLIGHT_ON); // Turn backlight on. TFT_BACKLIGHT_ON has been set in the TFT_eSPI library in the User Setup file TTGO_T_Display.h
    }

    tft.setRotation(0);

    lButton.setPressedHandler([](Button2 & b) {
        heading = (heading == 0) ? 3 : heading-1;
    });

    rButton.setPressedHandler([](Button2 & b) {
        heading = (heading == 3) ? 0 : heading+1;
    });

    
    snake[0] = make_pos(GX/2, GY/2);
    for(int i =  1; i < length; ++i) {
        snake[i] = make_pos(snake[i-1].x, snake[i-1].y + 1);
    }

    for(int iy = 0; iy < GY; ++iy) {
        for(int ix = 0; ix < GX; ++ix) {
            cells[iy*GX + ix] = 0;
            
            for(int cy = 0; cy < CXY; ++cy) {
                for(int cx = 0; cx < CXY; ++cx) {
                    int y = iy*CXY + cy;
                    int x = ix*CXY + cx;

                    image[y*X + x] = TFT_BLACK;
                }
            }
        }
    }

      

    for(int i = 0; i < length; ++i) {
        cells[snake[i].y * GX + snake[i].x] = 1;
        for(int cy = 0; cy < CXY; ++cy) {
            for(int cx = 0; cx < CXY; ++cx) {
                int y = snake[i].y*CXY + cy;
                int x = snake[i].x*CXY + cx;

                image[y*X + x] = TFT_WHITE;
            }
        }
    }

    
    target = make_pos(rand()%GX, rand()%GY);
    while(cells[target.y*GX + target.x] != 0) {
        target = make_pos(rand()%GX, rand()%GY);
    }
    cells[target.y*GX + target.x] = 2;
    // image[target.y*GX + target.x] = TFT_RED;

    for(int cy = 0; cy < CXY; ++cy) {
        for(int cx = 0; cx < CXY; ++cx) {
            int y = target.y*CXY + cy;
            int x = target.x*CXY + cx;

            image[y*X + x] = TFT_RED;
        }
    }
}

void loop() {
    //Serial.write("loop\n");
    lButton.loop();
    rButton.loop();
   


    if(running) {
        counter += 10;
        if(counter >= snake_delay) {
            pos next = snake[0];
            if(heading == 0) { // N
                next.y = (next.y == 0) ? GY-1 : next.y-1;
            } else if(heading == 1) { // E
                next.x = (next.x == GX-1) ? 0 : next.x+1;
            } else if(heading == 2) { // S
                next.y = (next.y == GY-1) ? 0 : next.y+1;
            } else if(heading == 3) { // W
                next.x = (next.x == 0) ? GX-1 : next.x-1;
            }

            if(cells[next.y*GX + next.x] == 1) {
                running = false;
                tft.setTextColor(TFT_GREEN);
                tft.setTextSize(2);
                tft.drawString("Game Over!", X/2-55, Y/2);

                char score[80];
                sprintf(score, "Score: %i", length-3);

                tft.drawString(score, X/2-55, Y/2+20);
                // tft.drawCentreString("Game Over!", X/2, Y/2);
            } else {
                if(cells[next.y*GX + next.x] == 2) {
                    length++;
                    snake_delay = (snake_delay > 10) ? snake_delay - 10 : snake_delay;

                    target = make_pos(rand()%GX, rand()%GY);
                    while(cells[target.y*GX + target.x] != 0) {
                        target = make_pos(rand()%GX, rand()%GY);
                    }
                    cells[target.y*GX + target.x] = 2;

                    for(int cy = 0; cy < CXY; ++cy) {
                        for(int cx = 0; cx < CXY; ++cx) {
                            int y = target.y*CXY + cy;
                            int x = target.x*CXY + cx;

                            image[y*X + x] = TFT_RED;
                        }
                    }

                    // image[target.y*X + target.x] = TFT_RED;
                } else {
                    int j = length - 1;

                    cells[snake[j].y*GX + snake[j].x] = 0;
                    // image[snake[j].y*X + snake[j].x] = TFT_BLACK;
                    for(int cy = 0; cy < CXY; ++cy) {
                        for(int cx = 0; cx < CXY; ++cx) {
                            int y = snake[j].y*CXY + cy;
                            int x = snake[j].x*CXY + cx;

                            image[y*X + x] = TFT_BLACK;
                        }
                    }
                }

                for(int i = length-1; i > 0; --i) {
                    snake[i] = snake[i-1];
                }
                
                snake[0] = next;

                cells[snake[0].y*GX + snake[0].x] = 1;
                // image[snake[0].y*X + snake[0].x] = TFT_WHITE;
                for(int cy = 0; cy < CXY; ++cy) {
                    for(int cx = 0; cx < CXY; ++cx) {
                        int y = snake[0].y*CXY + cy;
                        int x = snake[0].x*CXY + cx;

                        image[y*X + x] = TFT_WHITE;
                    }
                }

                tft.pushImage(0, 0, 135, 240, image);
            }
            counter = 0;
        }
    }

    // Wait a bit before scanning again
    delay(10);
}
