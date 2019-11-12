/*
 * -------------------------------------------
 *    MSP432 DriverLib - v3_21_00_05
 * -------------------------------------------
 *
 * --COPYRIGHT--,BSD,BSD
 * Copyright (c) 2016, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * --/COPYRIGHT--*/

/******************************************************************************
 *         Name(s): Adan Pantoja && Wesley Luna
 *       Professor: Dr. Kandalaft
 *            Date: October 25, 2019
 * Class & Section: EGR 326 - 903
 *     Description: The program will keep track of the last 5 times the user pressed
 *                  the '*' button on the external keypad. All 5 instances will be
 *                  saved in flash memory of the MSP432. The data will then be read
 *                  from memory and displayed on the Liquid Crystal Display.
 *******************************************************************************/
/* DriverLib Includes */
#include "driverlib.h"

/* Standard Includes */
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

/*Specialized includes*/
#include "KeypadInitialization.h"
#include "RTC.h"
#include "ST7735.h"
#include "Rotary_Encoder.h"

/*Boolean Macros*/
#define TRUE  1
#define FALSE 0


/*RTC Macros*/
extern int cw;                      //Clockwise Counter
extern int ccw;                     //Counter-Clockwise Counter
extern volatile uint8_t REFLAG;     //RE Button Flag

//Macros for size of LCD
#define HEIGHT 160
#define WIDTH 128

//Step size for size four font
#define STEP 10

//Watchdog counter
#define WDResetCount 120


#define CALIBRATION_START 0x000200000
#define SLAVE_ADDR 0x68     // DS1337

/*Macro for '10' on keypad*/
#define ASTERISK 10

//WD Function Prototype
void WDInit();
void resetWDCount();

//RTC and Flash function prototypes
void printTime(void);
void saveAlarm(void);
void clockInit48MHzXTL(void);
void save_to_flash(void);
void display_date(int i);
void read_from_flash();

extern unsigned char timeDateToSet[15];
extern unsigned char timeDateReadback[7];

char flashTime[70];

//Function prototypes
//void init48MHz();                   //initialize clock to 48MHz
void delaySeconds(int i);           //delays seconds
void displayScreen();               //Displays time, speed, date, temperature, and weekday
void settingsMenu();                //Allows the user to access all of the submenus
void setTimeMenu();                 //Allows the user to set the time
void setDateMenu();                 //Allows the user to set the date
void setBackgroundColor();          //Allows the user to set the background
void playlist();                    //Allows the user to play a song
void viewAlarms();                  //Allows the user to view the 5 most recent alarms
void resetdisplayScreen();          //resets the display screen
void initAlarmTimes();              //initializes the alarm structs
int verifyDate(int month, int day); //Verifies the user-entered date
int verifyTime(int hour, int date); //Verifies the user-entered time
void invalidInput();                //Notifies the user that the input was invalid

//Playlist Function Prototypes
void songOne();                     //Plays song 1 (Mario Theme)
void songTwo();                     //Plays song 2 (Legend of Zelda Theme)
void songThree();                   //Plays song 3 (Tetris Theme)
void songFour();                    //Plays song 4 (Pokemon Theme)



void initTimeOutTimer();            //60 second inactivity time out timers


//Struct to time and dates
typedef struct
{
    uint8_t second;
    uint8_t day;
    uint8_t month;
    uint8_t year;
    uint8_t hour;
    uint8_t minute;
    uint8_t weekday;
    uint8_t AM;
    uint8_t H24;
    uint8_t alarm;
}TIME;

//LCD global variables
int bgColor;                        //Background color of LCD Screen
int hlColor;                        //highlighted color of LCD Screen
int txtColor;                       //text color of LCD Screen
uint8_t WDCount;
//TIME struct related global variables
TIME time;
TIME alarm[5];

const uint8_t simulatedCalibrationData[]="test data block"; // array to hold data
uint8_t read_back_data[90]; // array to hold values read back from flash
uint8_t* addr_pointer; // pointer to address in flash for reading back values

int main(void)
{
    WDCount = 0;
    uint16_t BLACK = ST7735_Color565(0,0,0);
    /* Stop Watchdog  */
    MAP_WDT_A_holdTimer();
    I2C1_Initialization();          // Initializes I2C for Real Time Clock
    //    Keypad_Initialization();        //Initializes the Keypad
    clockInit48MHzXTL();            // Setting MCLK to 48MHz for faster programming



    rotary_encoder_initialization();
    RE_setInterrupts();
    get_CLK_DT_values();
    // Enable Interrupts
    NVIC_EnableIRQ(PORT2_IRQn);
    NVIC->ISER[0] = 1 <<((WDT_A_IRQn) & 31);
    __enable_interrupt();


    // Sends data to Real Time Clock
    I2C1_burstWrite(SLAVE_ADDR, 0, 7, timeDateToSet);

    //    Output_On();                    // Turn on LCD
    //    ST7735_InitR(INITR_BLACKTAB);   // Initialize LCD
    //    Output_Clear();                 // Clear output
    //    ST7735_FillScreen(BLACK);       // Fills background color


    ST7735_InitR(INITR_BLACKTAB);
    bgColor = ST7735_CYAN;
    hlColor = ST7735_WHITE;
    txtColor = ST7735_BLACK;
    read_from_flash();

    WDInit();
    // Enable Interrupts
    NVIC_EnableIRQ(PORT2_IRQn);
    NVIC->ISER[0] = 1 <<((WDT_A_IRQn) & 31);
    __enable_interrupt();

    //Cycle through the parts
    displayScreen();
    //setTimeMenu();
    //setDateMenu();
    //settingsMenu();
    //playlist();
    //viewAlarms();
    while(1);



    //    printTime();
    //
    while(1)
    {
        // Reads data from Real Time Clock via I^2C
        I2C1_burstRead(SLAVE_ADDR, 0, 7, timeDateReadback);


        if(readKeypad() == ASTERISK)
        {

            saveAlarm();
            read_from_flash();
            printTime();
            //            for(i = 4; i >= 0; i--)
            //            {
            //                if(alarm[i].hour != -1)
            //                {
            //                    printf("%x:%x:%x\n\n", alarm[i].hour, alarm[i].minute, alarm[i].second);
            //                }
            //            //            }
            //            Output_Clear();                 // Clear output
            //            ST7735_FillScreen(BLACK);       // Fills background color
            //            ST7735_SetCursor(1, 4);
            //            read_from_flash();
            //            for(i = 0; i < 5; i++)
            //            {
            //                sprintf(temp,"%x %x %x %x %x %x %x %x %x %s\n",
            //                        alarm[i].second,
            //                        alarm[i].minute,
            //                        alarm[i].hour,
            //                        alarm[i].weekday,
            //                        alarm[i].day,
            //                        alarm[i].month,
            //                        alarm[i].year,
            //                        alarm[i].AM,
            //                        alarm[i].H24,
            //                        alarm[i].alarm
            //                );
            //                printf("%s\n", temp);

            //            }
        }
    }
}

//
void displayScreen()
{
    int s1 = 5;                     //Size of speed string
    int s2 = 7;                     //Size of time string
    int s3 = 5;                     //Size of date string
    int s4 = 8;                     //Size of weekday string
    int s5 = 12;                    //Size of temperature string
    int pos = 0;                    //Position of which string is emphasized (blinking)
    int cw_prev = cw;               //Previous clockwise counter
    int ccw_prev = ccw;             //Previous counter clockwise counter
    char c1[5] = "Speed";           //Speed string
    char c2[7] = "Time";            //Time string
    char c3[5] = "Date";            //Date string
    char c4[8] = "Weekday";         //Weekday string
    char c5[12] = "Temperature";    //Temperature string

//    Output_Clear();                 //Clears the output of the LCD screen
    ST7735_FillScreen(bgColor);     //Sets background color
    int i;                          //Integer to loop through printing the strings on the LCD

    resetdisplayScreen();           //resets the screen display

    while(1)
    {
        resetWDCount();
        if(REFLAG)
        {
            REFLAG = 0;
            settingsMenu();         //If the RE Button is pressed, go to the settings menu
            resetdisplayScreen();   //Resets the display screen after returning from the settings menu
        }

        //Blinks the highlighted text
        //Increments position counter clockwise turns
        //Decrement positions at clockwise turns
        switch(pos)
        {
        case 0:                     //Position at Speed
            for(i = 0; i < s1; i++)
            {
                ST7735_DrawChar(i*STEP, HEIGHT*(1.0/6) + 10, c1[i], hlColor, bgColor, 2);
            }
            Delay1ms(500);

            if(cw > cw_prev)        //clockwise
            {
                ccw_prev = ccw;
                cw_prev = cw;
                pos = 4;
            }else if(ccw > ccw_prev) //counterclockwise
            {
                ccw_prev = ccw;
                cw_prev = cw;
                pos++;
            }
            for(i = 0; i < s1; i++)
            {
                ST7735_DrawChar(i*STEP, HEIGHT*(1.0/6) + 10, c1[i], txtColor, bgColor, 2);
            }
            Delay1ms(500);
            break;

        case 1:                     //Position at Time
            for(i = 0; i < s2; i++)
            {
                ST7735_DrawChar(i*STEP, HEIGHT*(2.0/6) + 10, c2[i], hlColor, bgColor, 2);
            }
            Delay1ms(500);
            if(cw > cw_prev)        //clockwise
            {
                ccw_prev = ccw;
                cw_prev = cw;
                pos--;
            }else if(ccw > ccw_prev) //counterclockwise
            {
                ccw_prev = ccw;
                cw_prev = cw;
                pos++;
            }
            for(i = 0; i < s2; i++)
            {
                ST7735_DrawChar(i*STEP, HEIGHT*(2.0/6) + 10, c2[i], txtColor, bgColor, 2);
            }
            Delay1ms(500);
            break;
        case 2:                     //Position at Date
            for(i = 0; i < s3; i++)
            {
                ST7735_DrawChar(i*STEP, HEIGHT*(3.0/6) + 10, c3[i], hlColor, bgColor, 2);
            }
            Delay1ms(500);
            if(cw > cw_prev)        //clockwise
            {
                ccw_prev = ccw;
                cw_prev = cw;
                pos--;
            }else if(ccw > ccw_prev) //counterclockwise
            {
                ccw_prev = ccw;
                cw_prev = cw;
                pos++;
            }
            for(i = 0; i < s3; i++)
            {
                ST7735_DrawChar(i*STEP, HEIGHT*(3.0/6) + 10, c3[i], txtColor, bgColor, 2);
            }
            Delay1ms(500);
            break;
        case 3:                     //Position at Weekday
            for(i = 0; i < s4; i++)
            {
                ST7735_DrawChar(i*STEP, HEIGHT*(4.0/6) + 10, c4[i], hlColor, bgColor, 2);
            }
            Delay1ms(500);
            if(cw > cw_prev)        //clockwise
            {
                ccw_prev = ccw;
                cw_prev = cw;
                pos--;
            }else if(ccw > ccw_prev) //counterclockwise
            {
                ccw_prev = ccw;
                cw_prev = cw;
                pos++;
            }
            for(i = 0; i < s4; i++)
            {
                ST7735_DrawChar(i*STEP, HEIGHT*(4.0/6) + 10, c4[i], txtColor, bgColor, 2);
            }
            Delay1ms(500);
            break;
        case 4:                     //Position at Temperature
            for(i = 0; i < s5; i++)
            {
                ST7735_DrawChar(i*STEP, HEIGHT*(5.0/6) + 10, c5[i], hlColor, bgColor, 2);
            }
            Delay1ms(500);
            if(cw > cw_prev)        //clockwise
            {
                ccw_prev = ccw;
                cw_prev = cw;
                pos--;
            }else if(ccw > ccw_prev) //counter clockwise
            {
                ccw_prev = ccw;
                cw_prev = cw;
                pos = 0;
            }
            for(i = 0; i < s5; i++)
            {
                ST7735_DrawChar(i*STEP, HEIGHT*(5.0/6) + 10, c5[i], txtColor, bgColor, 2);
            }
            Delay1ms(500);
            break;
        default:
            pos = 0;
            break;
        }
    }
}

void settingsMenu()
{
    int s = 9;                      //Size of string "Settings"
    int s1 = 9;                     //Size of string "Set Time"
    int s2 = 9;                     //Size of string "Set Date"
    int s3 = 12;                    //Size of string "View Alarms"
    int s4 = 9;                     //Size of string "Playlist"
    int s5 = 5;                     //Size of string "Back"
    int pos = 0;                    //Position of which string is emphasized (blinking) and will be selected
    int ccw_prev = ccw;             //Previous counter clockwise counter
    int cw_prev = cw;               //Previous clockwise counter
    char c[9] = "Settings";         //String "Settings"
    char c1[9] = "Set Time";        //String "Set Time"
    char c2[9] = "Set Date";        //String "Set Date"
    char c3[12] = "View Alarms";    //Size of string "View Alarms"
    char c4[9] = "Playlist";        //Size of string "Playlist"
    char c5[5] = "Back";            //Size of string "Back"
    int i;                          //Integer to loop through printing the strings on the LCD
    int resetFlag = 1;              //Flag to reset the display to page one
    int resetFlag2 = 1;             //Flag to reset the display to page two
    int returnFlag = 0;             //Determines whether to return to the display screen

    //Loops while the RE Button isn't pressed. Once it is, then the highlighted menu option will be selected
    while(REFLAG == 0)
    {
        resetWDCount();
        if(resetFlag)
        {
//            Output_Clear();             //Clears the LCD screen
            ST7735_FillScreen(bgColor); //Sets background color
            resetFlag = 0;              //Flag to reset the display to page one
            //Fills the screen with all the options for the first page of the settings menu
            for(i = 0; i < s; i++)
            {
                ST7735_DrawChar(i*STEP + 20, HEIGHT*(1.0/6) + 10, c[i], txtColor, bgColor, 2);
            }
            for(i = 0; i < s1; i++)
            {
                ST7735_DrawChar(i*STEP, HEIGHT*(2.0/6) + 10, c1[i], txtColor, bgColor, 2);
            }
            for(i = 0; i < s2; i++)
            {
                ST7735_DrawChar(i*STEP, HEIGHT*(3.0/6) + 10, c2[i], txtColor, bgColor, 2);
            }
            for(i = 0; i < s3; i++)
            {
                ST7735_DrawChar(i*STEP, HEIGHT*(4.0/6) + 10, c3[i], txtColor, bgColor, 2);
            }
            for(i = 0; i < s4; i++)
            {
                ST7735_DrawChar(i*STEP, HEIGHT*(5.0/6) + 10, c4[i], txtColor, bgColor, 2);
            }
        }
        switch(pos)
        {
        case 0:
            for(i = 0; i < s1; i++)
            {
                ST7735_DrawChar(i*STEP, HEIGHT*(2.0/6) + 10, c1[i], hlColor, bgColor, 2);
            }
            Delay1ms(500);

            if(cw > cw_prev)                //clockwise
            {
                ccw_prev = ccw;
                cw_prev = cw;
                pos = 4;
                resetFlag2 = 1;
            }else if(ccw > ccw_prev)        //counter clockwise
            {
                ccw_prev = ccw;
                cw_prev = cw;
                pos++;
            }
            for(i = 0; i < s1; i++)
            {
                ST7735_DrawChar(i*STEP, HEIGHT*(2.0/6) + 10, c1[i], txtColor, bgColor, 2);
            }
            Delay1ms(500);
            break;
        case 1:
            for(i = 0; i < s2; i++)
            {
                ST7735_DrawChar(i*STEP, HEIGHT*(3.0/6) + 10, c2[i], hlColor, bgColor, 2);
            }
            Delay1ms(500);
            if(cw > cw_prev)                //clockwise
            {
                ccw_prev = ccw;
                cw_prev = cw;
                pos--;
            }else if(ccw > ccw_prev)        //counter clockwise
            {
                ccw_prev = ccw;
                cw_prev = cw;
                pos++;
            }
            for(i = 0; i < s2; i++)
            {
                ST7735_DrawChar(i*STEP, HEIGHT*(3.0/6) + 10, c2[i], txtColor, bgColor, 2);
            }
            Delay1ms(500);
            break;
        case 2:
            for(i = 0; i < s3; i++)
            {
                ST7735_DrawChar(i*STEP, HEIGHT*(4.0/6) + 10, c3[i], hlColor, bgColor, 2);
            }
            Delay1ms(500);
            if(cw > cw_prev)                //clockwise
            {
                ccw_prev = ccw;
                cw_prev = cw;
                pos--;
            }else if(ccw > ccw_prev)        //counter clockwise
            {
                ccw_prev = ccw;
                cw_prev = cw;
                pos++;
            }
            for(i = 0; i < s3; i++)
            {
                ST7735_DrawChar(i*STEP, HEIGHT*(4.0/6) + 10, c3[i], txtColor, bgColor, 2);
            }
            Delay1ms(500);
            break;
        case 3:
            for(i = 0; i < s4; i++)
            {
                ST7735_DrawChar(i*STEP, HEIGHT*(5.0/6) + 10, c4[i], hlColor, bgColor, 2);
            }
            Delay1ms(500);
            if(cw > cw_prev)                //clockwise
            {
                ccw_prev = ccw;
                cw_prev = cw;
                pos--;
            }else if(ccw > ccw_prev)        //counter clockwise
            {
                resetFlag2 = 1;
                ccw_prev = ccw;
                cw_prev = cw;
                pos++;
            }
            for(i = 0; i < s4; i++)
            {
                ST7735_DrawChar(i*STEP, HEIGHT*(5.0/6) + 10, c4[i], txtColor, bgColor, 2);
            }
            Delay1ms(500);
            break;
        case 4:
            if(resetFlag2)
            {
//                Output_Clear();             //Clears the LCD screen
                ST7735_FillScreen(bgColor); //Sets background color
                resetFlag2 = 0;             //Flag to reset the display to page two
                //Fills the screen with all the options for the second page of the settings menu
                for(i = 0; i < s; i++)
                {
                    ST7735_DrawChar(i*STEP + 20, HEIGHT*(1.0/6) + 10, c[i], txtColor, bgColor, 2);
                }
            }
            for(i = 0; i < s5; i++)
            {
                ST7735_DrawChar(i*STEP, HEIGHT*(2.0/6) + 10, c5[i], hlColor, bgColor, 2);
            }
            Delay1ms(500);
            if(cw > cw_prev)            //clockwise
            {
                ccw_prev = ccw;
                cw_prev = cw;
                pos--;
                resetFlag = 1;
            }else if(ccw > ccw_prev)    //counter clockwise
            {
                ccw_prev = ccw;
                cw_prev = cw;
                pos = 0;
                resetFlag = 1;
            }
            for(i = 0; i < s5; i++)
            {
                ST7735_DrawChar(i*STEP, HEIGHT*(2.0/6) + 10, c5[i], txtColor, bgColor, 2);
            }
            Delay1ms(500);
            break;
        default:
            pos = 0;
            break;
        }
    }

    //If RE Button is pressed, the highlighted menu option will be selected
    if(REFLAG)
    {
        REFLAG = 0;                     //RE Button flag

        //Determines what menu option should be navigated to based off the
        //position of the cursor
        switch(pos)
        {
        case 0:
            setTimeMenu();              //Set time menu is selected
            break;
        case 1:
            setDateMenu();              //Set date menu is selected
            break;
        case 2:
            viewAlarms();               //View alarms menu is selected
            break;
        case 3:
            playlist();                 //Playlist menu is selected
            break;
        case 4:
            returnFlag = 1;             //Returns to the display screen
            break;
        default:
            break;
        }

        if(returnFlag)                  //Returns to the display screen
        {
            return;
        }
    }
}

void setTimeMenu()                          //MUST READ TIME FROM RTC FIRST!!! FIX ME!!!
{
    int hour1 = time.hour / 10;             //Digit 1 of the hour
    int hour2 = time.hour % 10;             //Digit 2 of the hour
    int minute1 = time.minute / 10;         //Digit 1 of the minute
    int minute2 = time.minute % 10;         //Digit 2 of the minute
    int hour, minute;                       //Integers for the hour and minutes
    int AM = 0;                             //AM or PM time
    char c[9] = "Set Time";                 //"Set Time" string
    char timeOfDay[2][3] = {"PM", "AM"};    //"AM","PM" string array
    int i;                                  //Integer to loop through printing the strings on the LCD
    int ccw_prev = ccw;                     //Previous counter clockwise counter
    int cw_prev = cw;                       //Previous clockwise counter
//    Output_Clear();                         //Clears the LCD screen
    ST7735_FillScreen(bgColor);             //Sets background color

    //Displays the text "Set Time"
    for(i = 0; i < 9; i++)
    {
        ST7735_DrawChar(i*STEP + 20, 40, c[i], txtColor, bgColor, 2);
    }

    //Displays the last recorded time
    ST7735_DrawChar(0*STEP + 30, HEIGHT*(1.0/2), hour1 + '0', txtColor, bgColor, 2);
    ST7735_DrawChar(1*STEP + 30, HEIGHT*(1.0/2), hour2 + '0', txtColor, bgColor, 2);
    ST7735_DrawChar(2*STEP + 30, HEIGHT*(1.0/2),':', txtColor, bgColor, 2);
    ST7735_DrawChar(3*STEP + 30, HEIGHT*(1.0/2), minute1 + '0', txtColor, bgColor, 2);
    ST7735_DrawChar(4*STEP + 30, HEIGHT*(1.0/2), minute2 + '0', txtColor, bgColor, 2);

    //Print AM or PM on the screen
    for(i = 0; i < 3; i++)
    {
        ST7735_DrawChar(i*STEP + 80, HEIGHT*(1.0/2), timeOfDay[AM][i], txtColor, bgColor, 2);
    }
    Delay1ms(500);

    //While the RE Button is not pressed, the code will continue to highlight
    //and update the digit one of the hour
    while(REFLAG == 0)
    {
        resetWDCount();
        ST7735_DrawChar(0 + 30, HEIGHT*(1.0/2), hour1 + '0', hlColor, bgColor, 2);
        Delay1ms(500);

        if(cw > cw_prev)            //clockwise
        {
            if(hour1 == 0)
            {
                hour1 = 1;
            }else if(hour1 == 1)
            {
                hour1 = 0;
            }
            ccw_prev = ccw;
            cw_prev = cw;
        }else if(ccw > ccw_prev)    //counter clockwise
        {
            if(hour1 == 0)
            {
                hour1 = 1;
            }else if(hour1 == 1)
            {
                hour1 = 0;
            }
            ccw_prev = ccw;
            cw_prev = cw;
        }
        ST7735_DrawChar(0 + 30, HEIGHT*(1.0/2), hour1 + '0', txtColor, bgColor, 2);
        Delay1ms(500);
    }
    REFLAG = 0;

    //While the RE Button is not pressed, the code will continue to highlight
    //and update the digit two of the hour
    while(REFLAG == 0)
    {
        resetWDCount();
        ST7735_DrawChar(STEP + 30, HEIGHT*(1.0/2), hour2 + '0', hlColor, bgColor, 2);
        Delay1ms(500);
        if(cw > cw_prev)            //clockwise
        {
            hour2++;
            ccw_prev = ccw;
            cw_prev = cw;
            if(hour2 == 10)
            {
                hour2 = 0;
            }
        }else if(ccw > ccw_prev)    //counter clockwise
        {
            hour2--;
            ccw_prev = ccw;
            cw_prev = cw;
            if(hour2 == -1)
            {
                hour2 = 9;
            }
        }
        ST7735_DrawChar(STEP + 30, HEIGHT*(1.0/2), hour2 + '0', txtColor, bgColor, 2);
        Delay1ms(500);
    }
    REFLAG = 0;

    //While the RE Button is not pressed, the code will continue to highlight
    //and update the digit one of the minute
    while(REFLAG == 0)
    {
        resetWDCount();
        ST7735_DrawChar(3*STEP + 30, HEIGHT*(1.0/2), minute1 + '0', hlColor, bgColor, 2);
        Delay1ms(500);

        if(cw > cw_prev)            //clockwise
        {
            minute1++;
            ccw_prev = ccw;
            cw_prev = cw;
            if(minute1 == 6)
            {
                minute1 = 0;
            }
        }else if(ccw > ccw_prev)    //counter clockwise
        {
            ccw_prev = ccw;
            cw_prev = cw;
            minute1--;
            if(minute1 == -1)
            {
                minute1 = 3;
            }
        }
        ST7735_DrawChar(3*STEP + 30, HEIGHT*(1.0/2), minute1 + '0', txtColor, bgColor, 2);
        Delay1ms(500);
    }
    REFLAG = 0;

    //While the RE Button is not pressed, the code will continue to highlight
    //and update the digit two of the minute
    while(REFLAG == 0)
    {
        resetWDCount();
        ST7735_DrawChar(4*STEP + 30, HEIGHT*(1.0/2), minute2 + '0', hlColor, bgColor, 2);
        Delay1ms(500);

        if(cw > cw_prev)            //clockwise
        {
            minute2++;
            ccw_prev = ccw;
            cw_prev = cw;
            if(minute2 == 10)
            {
                minute2 = 0;
            }
        }else if(ccw > ccw_prev)    //counter clockwise
        {
            minute2--;
            ccw_prev = ccw;
            cw_prev = cw;
            if(minute2 == -1)
            {
                minute2 = 9;
            }
        }
        ST7735_DrawChar(4*STEP + 30, HEIGHT*(1.0/2), minute2 + '0', txtColor, bgColor, 2);
        Delay1ms(500);
    }
    REFLAG = 0;

    //While the RE Button is not pressed, the code will continue to highlight
    //and update the AM or PM option
    while(REFLAG == 0)
    {
        resetWDCount();
        for(i = 0; i < 3; i++)
        {
            ST7735_DrawChar(i*STEP + 80, HEIGHT*(1.0/2), timeOfDay[AM][i], hlColor, bgColor, 2);
        }
        Delay1ms(500);
        if(cw > cw_prev)            //clockwise
        {
            if(AM == 0)
            {
                AM = 1;
            }else if(AM == 1)
            {
                AM = 0;
            }
            ccw_prev = ccw;
            cw_prev = cw;
        }else if(ccw > ccw_prev)    //counter clockwise
        {
            if(AM == 0)
            {
                AM = 1;
            }else if(AM == 1)
            {
                AM = 0;
            }
            ccw_prev = ccw;
            cw_prev = cw;
        }
        for(i = 0; i < 3; i++)
        {
            ST7735_DrawChar(i*STEP + 80, HEIGHT*(1.0/2), timeOfDay[AM][i], txtColor, bgColor, 2);
        }
        Delay1ms(500);
    }
    REFLAG = 0;

    delaySeconds(3);                                                                            //DELETE ME LATER
    //Calculates the hour and minute
    hour = hour1 * 10 + hour2;
    minute = minute1 * 10 + minute2;

    //If the data entered agreed with the standard for setting the time
    if(verifyTime(hour, minute))
    {
        //Update Current Time
        time.hour = hour;
        time.minute = minute;
        time.AM = AM;
    }else
    {
        invalidInput();             //Notifies the user that the entered data was invalid
    }
    return;                         //Returns to the settings menu
}

void setDateMenu()                          //MUST READ DATE FROM RTC FIRST!!! FIX ME!!!
{
    int day1 = time.day / 10;               //Digit 1 of the day
    int day2 = time.day % 10;               //Digit 2 of the day
    int month1 = time.month / 10;           //Digit 1 of the month
    int month2 = time.month % 10;           //Digit 2 of the month
    int year1 = time.year / 10;             //Digit 1 of the year
    int year2 = time.year % 10;             //Digit 2 of the year
    int i;                                  //Integer to loop through printing the strings on the LCD
    int ccw_prev = ccw;                     //Previous counter clockwise counter
    int cw_prev = cw;                       //Previous clockwise counter
    int day, year, month;                   //Integers for the year, day, and month
    //Array of strings for the weekdays
    char week[7][10] = {"Sunday   ", "Tuesday  ", "Monday   ", "Wednesday", "Thursday ", "Friday   ", "Saturday "};
    char c[9] = "Set Date";                 //"Set Date" string
    int weekday = 0;                        //Integer for the weekday

//    Output_Clear();                         //Clears the LCD screen
    ST7735_FillScreen(bgColor);             //Sets background color

    //Displays "Set Time" on screen
    for(i = 0; i < 9; i++)
    {
        ST7735_DrawChar(i*STEP + 20, 40, c[i], txtColor, bgColor, 2);
    }

    //Prepares screen for input from the user
    ST7735_DrawChar(0*STEP + 20, HEIGHT*(2.0/4), month1 + '0', txtColor, bgColor, 2);
    ST7735_DrawChar(1*STEP + 20, HEIGHT*(2.0/4), month2 + '0', txtColor, bgColor, 2);
    ST7735_DrawChar(2*STEP + 20, HEIGHT*(2.0/4),'/', txtColor, bgColor, 2);
    ST7735_DrawChar(3*STEP + 20, HEIGHT*(2.0/4), day1 + '0', txtColor, bgColor, 2);
    ST7735_DrawChar(4*STEP + 20, HEIGHT*(2.0/4), day2 + '0', txtColor, bgColor, 2);
    ST7735_DrawChar(5*STEP + 20, HEIGHT*(2.0/4),'/', txtColor, bgColor, 2);
    ST7735_DrawChar(6*STEP + 20, HEIGHT*(2.0/4), year1 + '0', txtColor, bgColor, 2);
    ST7735_DrawChar(7*STEP + 20, HEIGHT*(2.0/4), year2 + '0', txtColor, bgColor, 2);

    //Puts the weekday on the display
    for(i = 0; week[weekday][i] != '\0'; i++)
    {
        ST7735_DrawChar(i*STEP + 30, HEIGHT*(3.0/4), week[weekday][i], txtColor, bgColor, 2);
    }
    Delay1ms(500);

    //While the RE Button is not pressed, the code will continue to highlight
    //and update the digit one of the month
    while(REFLAG == 0)
    {
        resetWDCount();
        ST7735_DrawChar(0 + 20, HEIGHT*(2.0/4), month1 + '0', hlColor, bgColor, 2);
        Delay1ms(500);

        if(cw > cw_prev)            //clockwise
        {
            if(month1 == 0)
            {
                month1 = 1;
            }else if(month1 == 1)
            {
                month1 = 0;
            }
            ccw_prev = ccw;
            cw_prev = cw;
        }else if(ccw > ccw_prev)    //counter clockwise
        {
            if(month1 == 0)
            {
                month1 = 1;
            }else if(month1 == 1)
            {
                month1 = 0;
            }
            ccw_prev = ccw;
            cw_prev = cw;
        }
        ST7735_DrawChar(0 + 20, HEIGHT*(2.0/4), month1 + '0', txtColor, bgColor, 2);
        Delay1ms(500);
    }
    REFLAG = 0;

    //While the RE Button is not pressed, the code will continue to highlight
    //and update the digit two of the month
    while(REFLAG == 0)
    {
        resetWDCount();
        ST7735_DrawChar(STEP + 20, HEIGHT*(2.0/4), month2 + '0', hlColor, bgColor, 2);
        Delay1ms(500);
        if(cw > cw_prev)            //clockwise
        {
            month2++;
            ccw_prev = ccw;
            cw_prev = cw;
            if(month2 == 10)
            {
                month2 = 0;
            }
        }else if(ccw > ccw_prev)     //counter clockwise
        {
            month2--;
            ccw_prev = ccw;
            cw_prev = cw;
            if(month2 == -1)
            {
                month2 = 9;
            }
        }
        ST7735_DrawChar(STEP + 20, HEIGHT*(2.0/4), month2 + '0', txtColor, bgColor, 2);
        Delay1ms(500);
    }
    REFLAG = 0;

    //While the RE Button is not pressed, the code will continue to highlight
    //and update the digit one of the day
    while(REFLAG == 0)
    {
        resetWDCount();
        ST7735_DrawChar(3*STEP + 20, HEIGHT*(2.0/4), day1 + '0', hlColor, bgColor, 2);
        Delay1ms(500);

        if(cw > cw_prev)            //clockwise
        {
            day1++;
            ccw_prev = ccw;
            cw_prev = cw;
            if(day1 == 4)
            {
                day1 = 0;
            }
        }else if(ccw > ccw_prev)    //counter clockwise
        {
            day1--;
            ccw_prev = ccw;
            cw_prev = cw;
            if(day1 == -1)
            {
                day1 = 3;
            }
        }
        ST7735_DrawChar(3*STEP + 20, HEIGHT*(2.0/4), day1 + '0', txtColor, bgColor, 2);
        Delay1ms(500);
    }
    REFLAG = 0;

    //While the RE Button is not pressed, the code will continue to highlight
    //and update the digit two of the day
    while(REFLAG == 0)
    {
        resetWDCount();
        ST7735_DrawChar(4*STEP + 20, HEIGHT*(2.0/4), day2 + '0', hlColor, bgColor, 2);
        Delay1ms(500);

        if(cw > cw_prev)            //clockwise
        {
            day2++;
            ccw_prev = ccw;
            cw_prev = cw;
            if(day2 == 10)
            {
                day2 = 0;
            }
        }else if(ccw > ccw_prev)    //counter clockwise
        {
            day2--;
            ccw_prev = ccw;
            cw_prev = cw;
            if(day2 == -1)
            {
                day2 = 9;
            }
        }
        ST7735_DrawChar(4*STEP + 20, HEIGHT*(2.0/4), day2 + '0', txtColor, bgColor, 2);
        Delay1ms(500);
    }
    REFLAG = 0;

    //While the RE Button is not pressed, the code will continue to highlight
    //and update the digit one of the year
    while(REFLAG == 0)
    {
        resetWDCount();
        ST7735_DrawChar(6*STEP + 20, HEIGHT*(2.0/4), year1 + '0', hlColor, bgColor, 2);
        Delay1ms(500);

        if(cw > cw_prev)                //clockwise
        {
            year1++;
            ccw_prev = ccw;
            cw_prev = cw;
            if(year1 == 10)
            {
                year1 = 0;
            }
        }else if(ccw > ccw_prev)        //counter clockwise
        {
            year1--;
            ccw_prev = ccw;
            cw_prev = cw;
            if(year1 == -1)
            {
                year1 = 9;
            }
        }
        ST7735_DrawChar(6*STEP + 20, HEIGHT*(2.0/4), year1 + '0', txtColor, bgColor, 2);
        Delay1ms(500);

    }
    REFLAG = 0;

    //While the RE Button is not pressed, the code will continue to highlight
    //and update the digit two of the year
    while(REFLAG == 0)
    {
        resetWDCount();
        ST7735_DrawChar(7*STEP + 20, HEIGHT*(2.0/4), year2 + '0', hlColor, bgColor, 2);
        Delay1ms(500);
        if(cw > cw_prev)            //clockwise
        {
            year2++;
            ccw_prev = ccw;
            cw_prev = cw;
            if(year2 == 10)
            {
                year2 = 0;
            }
        }else if(ccw > ccw_prev)    //counter clockwise
        {
            year2--;
            ccw_prev = ccw;
            cw_prev = cw;
            if(year2 == -1)
            {
                year2 = 9;
            }
        }
        ST7735_DrawChar(7*STEP + 20, HEIGHT*(2.0/4), year2 + '0', txtColor, bgColor, 2);
        Delay1ms(500);
    }
    REFLAG = 0;

    //While the RE Button is not pressed, the code will continue to highlight
    //and update the digit one of the hour
    while(REFLAG == 0)
    {
        resetWDCount();
        for(i = 0; week[weekday][i] != '\0'; i++)
        {
            ST7735_DrawChar(i*STEP + 30, HEIGHT*(3.0/4), week[weekday][i], hlColor, bgColor, 2);
        }
        Delay1ms(500);
        if(cw > cw_prev)            //counter clockwise
        {
            weekday++;
            ccw_prev = ccw;
            cw_prev = cw;
            if(weekday == 7)
            {
                weekday = 0;
            }
        }else if(ccw > ccw_prev)    //counter clockwise
        {
            weekday--;
            ccw_prev = ccw;
            cw_prev = cw;
            if(weekday == -1)
            {
                weekday = 6;
            }
        }
        for(i = 0; week[weekday][i] != '\0'; i++)
        {
            ST7735_DrawChar(i*STEP + 30, HEIGHT*(3.0/4), week[weekday][i], txtColor, bgColor, 2);
        }
        Delay1ms(500);
    }
    REFLAG = 0;
    delaySeconds(3);                                                                            //DELETE ME LATER
    //Calculate the month, day, and year
    month = month1 * 10 + month2;
    day = day1 * 10 + day2;
    year = year1* 10 + year2;

    //If the data entered agreed with the standard for setting the time
    if(verifyDate(month, day))
    {
        //Update Current Time
        time.day = day;
        time.month = month;
        time.year = year;
        time.weekday = weekday;
    }else
    {
        invalidInput();             //Notifies the user that the data entered is invalid
    }
}

void setBackgroundColor()
{
    //    ST7735_BLACK
    //    ST7735_BLUE
    //    ST7735_RED
    //    ST7735_GREEN
    //    ST7735_CYAN
    //    ST7735_MAGENTA
    //    ST7735_YELLOW
    //    ST7735_WHITE
}

void playlist()
{
    int s = 9;                      //Size of string "Playlist"
    int s1 = 6;                     //Size of string "Song 1"
    int s2 = 6;                     //Size of string "Song 2"
    int s3 = 6;                     //Size of string "Song 3"
    int s4 = 6;                     //Size of string "Song 4"
    int s5 = 5;                     //Size of string "Song 5"
    int pos = 0;                    //Position of which string is emphasized (blinking) and will be selected
    int i;                          //Integer to loop through printing the strings on the LCD
    int ccw_prev = ccw;             //Previous counter clockwise counter
    int cw_prev = cw;               //Previous clockwise counter
    char c[9] = "Playlist";         //String "Playlist"
    char c1[9] = "Song 1";          //String "Song 1"
    char c2[9] = "Song 2";          //String "Song 2"
    char c3[12] = "Song 3";         //String "Song 3"
    char c4[9] = "Song 4";          //String "Song 4"
    char c5[5] = "Back";            //String "Back"
    int resetFlag = 1;              //Flag to reset the display to page one
    int resetFlag2 = 1;             //Flag to reset the display to page two
    int returnFlag = 0;             //Determines whether to return to the display screen

    //Loops while the RE Button isn't pressed.
    //Once it is, then the highlighted menu option will be selected
    while(REFLAG == 0)
    {
        resetWDCount();
        if(resetFlag)
        {
//            Output_Clear();             //Clears the LCD screen
            ST7735_FillScreen(bgColor); //Sets background color
            resetFlag = 0;
            for(i = 0; i < s; i++)
            {
                ST7735_DrawChar(i*STEP + 20, HEIGHT*(1.0/6) + 10, c[i], txtColor, bgColor, 2);
            }
            for(i = 0; i < s1; i++)
            {
                ST7735_DrawChar(i*STEP, HEIGHT*(2.0/6) + 10, c1[i], txtColor, bgColor, 2);
            }
            for(i = 0; i < s2; i++)
            {
                ST7735_DrawChar(i*STEP, HEIGHT*(3.0/6) + 10, c2[i], txtColor, bgColor, 2);
            }
            for(i = 0; i < s3; i++)
            {
                ST7735_DrawChar(i*STEP, HEIGHT*(4.0/6) + 10, c3[i], txtColor, bgColor, 2);
            }
            for(i = 0; i < s4; i++)
            {
                ST7735_DrawChar(i*STEP, HEIGHT*(5.0/6) + 10, c4[i], txtColor, bgColor, 2);
            }
        }
        //Highlights different options depending on the position of the cursor
        switch(pos)
        {
        case 0:

            for(i = 0; i < s1; i++)
            {
                ST7735_DrawChar(i*STEP, HEIGHT*(2.0/6) + 10, c1[i], hlColor, bgColor, 2);
            }
            Delay1ms(500);
            if(cw > cw_prev)            //clockwise
            {
                pos = 4;
                resetFlag2 = 1;
                ccw_prev = ccw;
                cw_prev = cw;
            }else if(ccw > ccw_prev)    //counter clockwise
            {
                pos++;
                ccw_prev = ccw;
                cw_prev = cw;
            }
            for(i = 0; i < s1; i++)
            {
                ST7735_DrawChar(i*STEP, HEIGHT*(2.0/6) + 10, c1[i], txtColor, bgColor, 2);
            }
            Delay1ms(500);
            break;
        case 1:
            for(i = 0; i < s2; i++)
            {
                ST7735_DrawChar(i*STEP, HEIGHT*(3.0/6) + 10, c2[i], hlColor, bgColor, 2);
            }
            Delay1ms(500);
            if(cw > cw_prev)            //clockwise
            {
                pos--;
                ccw_prev = ccw;
                cw_prev = cw;
            }else if(ccw > ccw_prev)    //counter clockwise
            {
                pos++;
                ccw_prev = ccw;
                cw_prev = cw;
            }
            for(i = 0; i < s2; i++)
            {
                ST7735_DrawChar(i*STEP, HEIGHT*(3.0/6) + 10, c2[i], txtColor, bgColor, 2);
            }
            Delay1ms(500);
            break;
        case 2:
            for(i = 0; i < s3; i++)
            {
                ST7735_DrawChar(i*STEP, HEIGHT*(4.0/6) + 10, c3[i], hlColor, bgColor, 2);
            }
            Delay1ms(500);
            if(cw > cw_prev)            //clockwise
            {
                pos--;
                ccw_prev = ccw;
                cw_prev = cw;
            }else if(ccw > ccw_prev)    //counter clockwise
            {
                pos++;
                ccw_prev = ccw;
                cw_prev = cw;
            }
            for(i = 0; i < s3; i++)
            {
                ST7735_DrawChar(i*STEP, HEIGHT*(4.0/6) + 10, c3[i], txtColor, bgColor, 2);
            }
            Delay1ms(500);
            break;
        case 3:
            for(i = 0; i < s4; i++)
            {
                ST7735_DrawChar(i*STEP, HEIGHT*(5.0/6) + 10, c4[i], hlColor, bgColor, 2);
            }
            Delay1ms(500);
            if(cw > cw_prev)            //clockwise
            {
                pos--;
                ccw_prev = ccw;
                cw_prev = cw;
            }else if(ccw > ccw_prev)    //counter clockwise
            {
                pos++;
                resetFlag2 = 1;
                ccw_prev = ccw;
                cw_prev = cw;
            }
            for(i = 0; i < s4; i++)
            {
                ST7735_DrawChar(i*STEP, HEIGHT*(5.0/6) + 10, c4[i], txtColor, bgColor, 2);
            }
            Delay1ms(500);
            break;
        case 4:
            if(resetFlag2)
            {
                //Output_Clear();             //Clears the LCD screen
                ST7735_FillScreen(bgColor); //Sets background color
                resetFlag2 = 0;             //Flag to reset the display to page two

                //Fills the screen with all the options for the second page of the playlist menu
                for(i = 0; i < s; i++)
                {
                    ST7735_DrawChar(i*STEP + 20, HEIGHT*(1.0/6) + 10, c[i], txtColor, bgColor, 2);
                }
            }
            for(i = 0; i < s5; i++)
            {
                ST7735_DrawChar(i*STEP, HEIGHT*(2.0/6) + 10, c5[i], hlColor, bgColor, 2);
            }
            Delay1ms(500);
            if(cw > cw_prev)            //clockwise
            {
                pos--;
                resetFlag = 1;
                ccw_prev = ccw;
                cw_prev = cw;
            }else if(ccw > ccw_prev)    //counter clockwise
            {
                pos = 0;
                resetFlag = 1;
                ccw_prev = ccw;
                cw_prev = cw;
            }
            for(i = 0; i < s5; i++)
            {
                ST7735_DrawChar(i*STEP, HEIGHT*(2.0/6) + 10, c5[i], txtColor, bgColor, 2);
            }
            Delay1ms(500);
            break;
        default:
            pos = 0;
            break;
        }
    }

    //If RE Button is pressed, the highlighted menu option will be selected
    if(REFLAG)
    {
        REFLAG = 0;                     //RE Button flag
        switch(pos)
        {
        case 0:
            songOne();                  //Song one is selected
            break;
        case 1:
            songTwo();                  //Song two is selected
            break;
        case 2:
            songThree();                //Song three is selected
            break;
        case 3:
            songFour();                 //Song four is selected
            break;
        case 4:
            returnFlag = 1;             //Returns to the display screen
            break;
        default:
            break;
        }
        if(returnFlag)                  //Returns to the display screen
        {
            return;
        }
    }
}

void viewAlarms()
{
    int s = 6;                      //Size of string "Alarms"
    int s1 = 7;                     //Size of string "Alarms 1"
    int s2 = 7;                     //Size of string "Alarms 2"
    int s3 = 7;                     //Size of string "Alarms 3"
    int s4 = 7;                     //Size of string "Alarms 4"
    int s5 = 5;                     //Size of string "Alarms 5"
    int pos = 0;                    //Position of which string is emphasized (blinking) and will be selected
    int ccw_prev = ccw;             //Previous counter clockwise counter
    int cw_prev = cw;               //Previous clockwise counter
    char c[9] = "Alarms";           //String "Alarms 1"
    char c1[9] = "Alarm 1";         //String "Alarms 2"
    char c2[9] = "Alarm 2";         //String "Alarms 3"
    char c3[12] = "Alarm 3";        //String "Alarms 4"
    char c4[9] = "Alarm 4";         //String "Alarms 5"
    char c5[5] = "Back";            //Size of string "Back"
    int i;                          //Integer to loop through printing the strings on the LCD
    int resetFlag = 1;              //Flag to reset the display to page one
    int resetFlag2 = 1;             //Flag to reset the display to page two
    int returnFlag = 0;             //Determines whether to return to the display screen

    //Loops while the RE Button isn't pressed.
    //Once it is, then the highlighted menu option will be selected
    while(REFLAG == 0)
    {
        resetWDCount();
        if(resetFlag)
        {
//            Output_Clear();             //Clears the LCD screen
            ST7735_FillScreen(bgColor); //Sets background color
            resetFlag = 0;              //Flag to reset the display to page one
            //Fills the screen with all the options for the first page of the settings menu
            for(i = 0; i < s; i++)
            {
                ST7735_DrawChar(i*STEP + 20, HEIGHT*(1.0/6) + 10, c[i], txtColor, bgColor, 2);
            }
            for(i = 0; i < s1; i++)
            {
                ST7735_DrawChar(i*STEP, HEIGHT*(2.0/6) + 10, c1[i], txtColor, bgColor, 2);
            }
            for(i = 0; i < s2; i++)
            {
                ST7735_DrawChar(i*STEP, HEIGHT*(3.0/6) + 10, c2[i], txtColor, bgColor, 2);
            }
            for(i = 0; i < s3; i++)
            {
                ST7735_DrawChar(i*STEP, HEIGHT*(4.0/6) + 10, c3[i], txtColor, bgColor, 2);
            }
            for(i = 0; i < s4; i++)
            {
                ST7735_DrawChar(i*STEP, HEIGHT*(5.0/6) + 10, c4[i], txtColor, bgColor, 2);
            }
        }
        switch(pos)
        {
        case 0:
            for(i = 0; i < s1; i++)
            {
                ST7735_DrawChar(i*STEP, HEIGHT*(2.0/6) + 10, c1[i], hlColor, bgColor, 2);
            }
            Delay1ms(500);
            if(cw > cw_prev)            //clockwise
            {
                pos = 4;
                resetFlag2 = 1;

                ccw_prev = ccw;
                cw_prev = cw;
            }else if(ccw > ccw_prev)    //counter clockwise
            {
                pos++;
                ccw_prev = ccw;
                cw_prev = cw;
            }
            for(i = 0; i < s1; i++)
            {
                ST7735_DrawChar(i*STEP, HEIGHT*(2.0/6) + 10, c1[i], txtColor, bgColor, 2);
            }
            Delay1ms(500);
            break;
        case 1:
            for(i = 0; i < s2; i++)
            {
                ST7735_DrawChar(i*STEP, HEIGHT*(3.0/6) + 10, c2[i], hlColor, bgColor, 2);
            }
            Delay1ms(500);
            if(cw > cw_prev)            //clockwise
            {
                pos--;
                ccw_prev = ccw;
                cw_prev = cw;
            }else if(ccw > ccw_prev)    //counter clockwise
            {
                pos++;
                ccw_prev = ccw;
                cw_prev = cw;
            }
            for(i = 0; i < s2; i++)
            {
                ST7735_DrawChar(i*STEP, HEIGHT*(3.0/6) + 10, c2[i], txtColor, bgColor, 2);
            }
            Delay1ms(500);
            break;
        case 2:
            for(i = 0; i < s3; i++)
            {
                ST7735_DrawChar(i*STEP, HEIGHT*(4.0/6) + 10, c3[i], hlColor, bgColor, 2);
            }
            Delay1ms(500);
            if(cw > cw_prev)            //clockwise
            {
                pos--;
                ccw_prev = ccw;
                cw_prev = cw;

            }else if(ccw > ccw_prev)    //counter clockwise
            {
                pos++;
                ccw_prev = ccw;
                cw_prev = cw;
            }
            for(i = 0; i < s3; i++)
            {
                ST7735_DrawChar(i*STEP, HEIGHT*(4.0/6) + 10, c3[i], txtColor, bgColor, 2);
            }
            Delay1ms(500);
            break;
        case 3:
            for(i = 0; i < s4; i++)
            {
                ST7735_DrawChar(i*STEP, HEIGHT*(5.0/6) + 10, c4[i], hlColor, bgColor, 2);
            }
            Delay1ms(500);
            if(cw > cw_prev)            //clockwise
            {
                pos--;
                ccw_prev = ccw;
                cw_prev = cw;
            }else if(ccw > ccw_prev)    //counter clockwise
            {
                pos++;
                resetFlag2 = 1;
                ccw_prev = ccw;
                cw_prev = cw;
            }
            for(i = 0; i < s4; i++)
            {
                ST7735_DrawChar(i*STEP, HEIGHT*(5.0/6) + 10, c4[i], txtColor, bgColor, 2);
            }
            Delay1ms(500);
            break;
        case 4:
            if(resetFlag2)
            {
//                Output_Clear();
                ST7735_FillScreen(bgColor);
                resetFlag2 = 0;
                for(i = 0; i < s; i++)
                {
                    ST7735_DrawChar(i*STEP + 20, HEIGHT*(1.0/6) + 10, c[i], txtColor, bgColor, 2);
                }
            }
            for(i = 0; i < s5; i++)
            {
                ST7735_DrawChar(i*STEP, HEIGHT*(2.0/6) + 10, c5[i], hlColor, bgColor, 2);
            }
            Delay1ms(500);
            if(cw > cw_prev)            //clockwise
            {
                pos--;
                resetFlag = 1;
                ccw_prev = ccw;
                cw_prev = cw;
            }else if(ccw > ccw_prev)    //counter clockwise
            {
                pos = 0;
                resetFlag = 1;
                ccw_prev = ccw;
                cw_prev = cw;
            }
            for(i = 0; i < s5; i++)
            {
                ST7735_DrawChar(i*STEP, HEIGHT*(2.0/6) + 10, c5[i], txtColor, bgColor, 2);
            }
            Delay1ms(500);
            break;
        default:
            pos = 0;
            break;
        }
    }


    if(pos == 4 && returnFlag)
    {
        return;
    }
}

int verifyDate(int month, int day)
{
    if(month == 1 || month == 3 || month == 5 || month == 7 || month == 8 || month == 10 || month == 12)
    {
        if(day >= 1 && day <= 31)
        {
            return 1;
        }else
        {
            return 0;
        }
    }else if(month == 4 || month == 6 || month == 9 || month == 11)
    {
        if(day >= 1 && day <= 30)
        {
            return 1;
        }else
        {
            return 0;
        }
    }else if(month == 2)
    {
        if(day >= 1 && day <= 29)
        {
            return 1;
        }else
        {
            return 0;
        }
    }
    return 0;
}

int verifyTime(int hour, int minute)
{
    if(hour <= 12 && hour > 0)
    {
        if(minute >= 0 && minute <= 59)
        {
            return 1;
        }else
        {
            return 0;
        }
    }
    return 0;
}

void invalidInput()
{
    int i;                          //Integer to loop through printing the strings on the LCD
    int s1 = 8;                     //Size of "Invalid" string
    int s2 = 6;                     //Size of "Input" string
    char c[8] = "Invalid";          //"Invalid" string
    char c2[6] = "Input";           //"Input" string
    for(i = 0; i < s1; i++)
    {
        ST7735_DrawChar(i*STEP + 30, HEIGHT*(2.0/4), c[i], txtColor, bgColor, 2);
    }
    for(i = 0; i < s2; i++)
    {
        ST7735_DrawChar(i*STEP + 30, HEIGHT*(3.0/4), c2[i], txtColor, bgColor, 2);
    }
    delaySeconds(3);
}

void resetdisplayScreen()
{
    int s1 = 5;                     //Size of speed string
    int s2 = 7;                     //Size of time string
    int s3 = 5;                     //Size of date string
    int s4 = 8;                     //Size of weekday string
    int s5 = 12;                    //Size of temperature string
    char c1[5] = "Speed";           //Speed string
    char c2[7] = "Time";            //Time string
    char c3[5] = "Date";            //Date string
    char c4[8] = "Weekday";         //Weekday string
    char c5[12] = "Temperature";    //Temperature string

//    Output_Clear();                 //Clears the output of the LCD screen
    ST7735_FillScreen(bgColor);     //Sets background color
    int i;                          //Integer to loop through printing the strings on the LCD

    //Display the speed, date, time, weekday, and temperature on the screen
    for(i = 0; i < s1; i++)
    {
        ST7735_DrawChar(i*STEP, HEIGHT*(1.0/6) + 10, c1[i], txtColor, bgColor, 2);
    }
    for(i = 0; i < s2; i++)
    {
        ST7735_DrawChar(i*STEP, HEIGHT*(2.0/6) + 10, c2[i], txtColor, bgColor, 2);
    }
    for(i = 0; i < s3; i++)
    {
        ST7735_DrawChar(i*STEP, HEIGHT*(3.0/6) + 10, c3[i], txtColor, bgColor, 2);
    }
    for(i = 0; i < s4; i++)
    {
        ST7735_DrawChar(i*STEP, HEIGHT*(4.0/6) + 10, c4[i], txtColor, bgColor, 2);
    }
    for(i = 0; i < s5; i++)
    {
        ST7735_DrawChar(i*STEP, HEIGHT*(5.0/6) + 10, c5[i], txtColor, bgColor, 2);
    }
}

void songOne()
{

}

void songTwo()
{

}

void songThree()
{

}

void songFour()
{

}

//Altered clock initialization code obtain from Dr.Krug
void init48MHz()
{
    // Configure Flash wait-state to 1 for both banks 0 & 1
    FLCTL->BANK0_RDCTL = (FLCTL->BANK0_RDCTL & ~(FLCTL_BANK0_RDCTL_WAIT_MASK)) |
            FLCTL_BANK0_RDCTL_WAIT_1;
    FLCTL->BANK1_RDCTL = (FLCTL->BANK0_RDCTL & ~(FLCTL_BANK0_RDCTL_WAIT_MASK)) |
            FLCTL_BANK1_RDCTL_WAIT_1;
    //Configure HFXT to use 48MHz crystal, source to MCLK & HSMCLK*
    PJ->SEL0 |= BIT2 | BIT3;                // Configure PJ.2/3 for HFXT function
    PJ->SEL1 &= ~(BIT2 | BIT3); CS->KEY = CS_KEY_VAL ;
    // Unlock CS module for register access
    CS->CTL2 |= CS_CTL2_HFXT_EN | CS_CTL2_HFXTFREQ_6 | CS_CTL2_HFXTDRIVE;
    while(CS->IFG & CS_IFG_HFXTIFG) CS->CLRIFG |= CS_CLRIFG_CLR_HFXTIFG;
    /* Select MCLK & HSMCLK = HFXT, no divider */
    CS->CTL1 = CS->CTL1 & ~(CS_CTL1_SELM_MASK | CS_CTL1_DIVM_MASK | CS_CTL1_SELS_MASK |
            CS_CTL1_DIVHS_MASK) | CS_CTL1_SELM__HFXTCLK | CS_CTL1_SELS__HFXTCLK
                    | 0b010 << 20  //set HSMCLK to /4
                    | 0b010 << 28; //set SMCLK to /4
    CS->KEY = 0;                            // Lock CS module from unintended accesses
}

void initAlarmTimes()
{
    int i;
    //Current Time
    time.day = 0;
    time.month = 0;
    time.year = 0;
    time.hour = 0;
    time.minute = 0;
    time.weekday = 0;
    time.AM = 0;
    time.H24 = 0;

    //Saved alarm times
    for(i = 0; i < 5;i++)
    {
        alarm[i].day = 0;
        alarm[i].month = 0;
        alarm[i].year = 0;
        alarm[i].hour = 0;
        alarm[i].minute = 0;
        alarm[i].weekday = 0;
        alarm[i].AM = 0;
        alarm[i].H24 = 0;
        alarm[i].alarm = "None";
    }

}

void delaySeconds(int i)
{
    Delay1ms(i*1000);
}





void saveAlarm(void)
{
    int i;
    int flag = 1;

    for(i = 0; i < 5; i++)
    {
        if(alarm[i].hour == 0 && alarm[i].minute == 0 && alarm[i].second == 0 && flag == 1)
        {
            alarm[i].weekday = timeDateReadback[3];
            alarm[i].day = timeDateReadback[4];
            alarm[i].month = timeDateReadback[5];
            alarm[i].year = timeDateReadback[6];
            alarm[i].hour = timeDateReadback[2];
            alarm[i].minute = timeDateReadback[1];
            alarm[i].second = timeDateReadback[0];
            flag = 0;
            alarm[i].alarm = "none";
        }
    }

    if(flag)
    {
        // Shifting array elements to the right by 1
        for(i = 0; i < 4; i++)
        {
            alarm[i].weekday = alarm[i + 1].weekday;
            alarm[i].day = alarm[i + 1].day;
            alarm[i].month = alarm[i + 1].month;
            alarm[i].year = alarm[i + 1].year;
            alarm[i].hour = alarm[i + 1].hour;
            alarm[i].minute = alarm[i + 1].minute;
            alarm[i].second = alarm[i + 1].second;
        }

        // Retrieving the most recent time when '*' was pressed
        alarm[4].hour = timeDateReadback[2];
        alarm[4].minute = timeDateReadback[1];
        alarm[4].second = timeDateReadback[0];
    }
    save_to_flash();
}

void printTime(void)
{
    int i;
    char hour[2], minute[2], second[2];
    ST7735_SetCursor(1, 4);

    read_from_flash();
    printf(" Previous time(s) \n    * was pressed\n");
    printf("-----------------------------\n");

    for(i = 4; i >= 0; i--)
    {
        if(alarm[i].hour != 255)
        {
            if(alarm[i].minute < 10 && alarm[i].second > 9)
            {
                sprintf(hour, "%x", alarm[i].hour);
                sprintf(minute, "%x", alarm[i].minute);
                sprintf(second, "%x", alarm[i].second);
                if(alarm[i].AM)
                {
                    printf(" %s:0%s:%s AM", hour, minute, second);
                }else
                {
                    printf(" %s:0%s:%s PM", hour, minute, second);
                }
            }
            else if(alarm[i].second < 10 && alarm[i].minute > 9)
            {
                sprintf(hour, "%x", alarm[i].hour);
                sprintf(minute, "%x", alarm[i].minute);
                sprintf(second, "%x", alarm[i].second);
                if(alarm[i].AM)
                {
                    printf(" %s:%s:0%s AM", hour, minute, second);
                }else
                {
                    printf(" %s:%s:0%s PM", hour, minute, second);
                }
            }
            else if(alarm[i].second < 10 && alarm[i].minute < 10)
            {
                sprintf(hour, "%x", alarm[i].hour);
                sprintf(minute, "%x", alarm[i].minute);
                sprintf(second, "%x", alarm[i].second);
                if(alarm[i].AM)
                {
                    printf(" %s:0%s:0%s AM", hour, minute, second);
                }else
                {
                    printf(" %s:0%s:0%s PM", hour, minute, second);
                }
            }
        }
        else
        {
            sprintf(hour, "%x", alarm[i].hour);
            sprintf(minute, "%x", alarm[i].minute);
            sprintf(second, "%x", alarm[i].second);
            if(alarm[i].AM)
            {
                printf(" %s:%s:%s AM", hour, minute, second);
            }else
            {
                printf(" %s:%s:%s PM", hour, minute, second);
            }
        }
        display_date(i);
    }
}


void display_date(int i)
{
    char date[2], month[2], year[2];

    sprintf(date, "%x", alarm[i].day);
    sprintf(month, "%x", alarm[i].month);
    sprintf(year, "%x", alarm[i].year);

    printf(" %s/%s/%s\n", month, date, year);
}

void clockInit48MHzXTL(void)
{ // sets the clock module to use the external 4MHz crystal
    /* Configuring pins for peripheral/crystal usage */
    MAP_GPIO_setAsPeripheralModuleFunctionOutputPin(
            GPIO_PORT_PJ,
            GPIO_PIN3 | GPIO_PIN2,
            GPIO_PRIMARY_MODULE_FUNCTION);
    //enables getMCLK, getSMCLK to know externally set frequencies
    CS_setExternalClockSourceFrequency(32000, 48000000);
    /* Starting HFXT in non-bypass mode without a timeout. Before we start
     * we have to change VCORE to 1 to support the 48MHz frequency */
    MAP_PCM_setCoreVoltageLevel(PCM_VCORE1);
    MAP_FlashCtl_setWaitState(FLASH_BANK0, 2);
    MAP_FlashCtl_setWaitState(FLASH_BANK1, 2);
    CS_startHFXT(false); // false means that there are no timeouts set, will return when stable
    /* Initializing MCLK to HFXT (effectively 48MHz) */
    MAP_CS_initClockSignal(CS_MCLK, CS_HFXTCLK_SELECT, CS_CLOCK_DIVIDER_1);
}

void save_to_flash(void)
{
    uint8_t i; // index
    uint32_t byte_length = sizeof(alarm[0]) * 5;
    // halting the watch dog is done in the system_msp432p401r.c startup
    addr_pointer = CALIBRATION_START + 4;  // point to address in flash for saving data

    for(i = 0; i < byte_length; i++)
    {
        // read values in flash before programming
        read_back_data[i] = *addr_pointer++;
    }
    /* Unprotecting Info Bank 0, Sector 0  */
    MAP_FlashCtl_unprotectSector(FLASH_INFO_MEMORY_SPACE_BANK0,FLASH_SECTOR0);
    /* Erase the flash sector starting CALIBRATION_START.  */
    while(!MAP_FlashCtl_eraseSector(CALIBRATION_START));

    /* Program the flash with the new data. */
    //while (!MAP_FlashCtl_programMemory(flashTime,(void*) CALIBRATION_START+4, 8 * 70)); // leave first 4 bytes unprogrammed
    while (!MAP_FlashCtl_programMemory(alarm, (void*) CALIBRATION_START + 4, byte_length)); // leave first 4 bytes unprogrammed

    /* Setting the sector back to protected  */
    MAP_FlashCtl_protectSector(FLASH_INFO_MEMORY_SPACE_BANK0,FLASH_SECTOR0);

    //    addr_pointer = CALIBRATION_START + 4;  // point to address in flash for saved data
    //
    //    for(i = 0 ; i < byte_length; i++)
    //    {
    //        // read values in flash after programming
    //        read_back_data[i] = *addr_pointer++;
    //    }
    //
    //
    //    for(i = 0; i < 5; i++)
    //    {
    //
    //        alarm[i].hour = read_back_data[(16 * i) + 4];
    //        alarm[i].minute = read_back_data[(16 * i) + 5];
    //        alarm[i].second = read_back_data[(16 * i)];
    //
    //        alarm[i].weekday = read_back_data[(16 * i) + 6];
    //        alarm[i].day = read_back_data[(16 * i) + 1];
    //        alarm[i].month = read_back_data[(16 * i) + 2];
    //        alarm[i].year = read_back_data[(16 * i) + 3];
    //        alarm[i].alarm = read_back_data[(16 * i) + 9];
    //        alarm[i].AM = read_back_data[(16 * i) + 7];
    //        alarm[i].H24 = read_back_data[(16 * i) + 8];
    //    }
}

void read_from_flash()
{
    uint8_t i; // index
    uint32_t byte_length = sizeof(alarm[0]) * 5;

    addr_pointer = CALIBRATION_START + 4;  // point to address in flash for saved data

    for(i = 0 ; i < byte_length; i++)
    {
        // read values in flash after programming
        read_back_data[i] = *addr_pointer++;
    }


    for(i = 0; i < 5; i++)
    {

        alarm[i].hour = read_back_data[(16 * i) + 4];
        alarm[i].minute = read_back_data[(16 * i) + 5];
        alarm[i].second = read_back_data[(16 * i)];

        alarm[i].weekday = read_back_data[(16 * i) + 6];
        alarm[i].day = read_back_data[(16 * i) + 1];
        alarm[i].month = read_back_data[(16 * i) + 2];
        alarm[i].year = read_back_data[(16 * i) + 3];
        alarm[i].alarm = read_back_data[(16 * i) + 9];
        alarm[i].AM = read_back_data[(16 * i) + 7];
        alarm[i].H24 = read_back_data[(16 * i) + 8];
    }
}


//
//#define BTTN_PORT P5
//#define BUTTON BIT0
//
//#define TRUE 1
//#define FALSE 0
//
//
//#define WDResetCount 3

//uint8_t button_pressed(void);
//void PORT5_IRQHandler(void);
//void enable_ButtonInterrupt(void);
//void button_initialization(void);
//void init48MHz(void);
//void WDInit();
//
//uint8_t pressed = FALSE;
//uint8_t WDCount;

///**
// * main.c
// */
//void main(void)
//{
//    uint16_t BLACK = ST7735_Color565(0,0, 0); // Used for LCD background color
//    WDCount = 0;
//    uint8_t WDCount_Prev = 5;
//
//    WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;     // stop watchdog timer
//
//    button_initialization();
//    enable_ButtonInterrupt();
//    initSysTick();
//    init48MHz();
//
//
//    Output_On();                        // Turns on LCD display
//    ST7735_InitR(INITR_BLACKTAB);       // Initialize LCD
//    //Output_Clear();                     // Clears output
//    ST7735_FillScreen(BLACK);           // Fills background color
//    WDInit();
//    NVIC_EnableIRQ(PORT5_IRQn);
//    NVIC->ISER[0] = 1 <<((WDT_A_IRQn) & 31);
//    __enable_interrupt(); // Enables the use of interrupts
//
//    P2->SEL0 &=~ BIT0;
//    P2->SEL1 &=~ BIT0;
//    P2->DIR |= BIT0;
//    P2->OUT |= BIT0;
//    __delay_cycles(3000000);
//    P2->OUT &=~ BIT0;
//
//    while(1)
//    {
//        if(WDCount != WDCount_Prev)
//        {
//            WDCount_Prev = WDCount;
//            ST7735_SetCursor(1, 4);
//            switch(WDCount)
//            {
//            case 0:
//                printf("Time (sec): 0\n", WDCount);
//                break;
//            case 1:
//                printf("Time (sec): 1\n", WDCount);
//                break;
//            case 2:
//                printf("Time (sec): 2\n", WDCount);
//                break;
//            case 3:
//                printf("Time (sec): 3\n", WDCount);
//                break;
//            case 4:
//                printf("Time (sec): 4\n", WDCount);
//                break;
//            default:
//                break;
//            }
//
//        }
//        //      ST7735_SetCursor(1, 4);
//        //      printf("Time (sec): %d\n", WDCount);
//        //
//        //      if(pressed)
//        //      {
//        //          pressed = FALSE;
//        //          printf("Button interrupts works\n");
//        //      }
//    }
//
//
//}




//void button_initialization(void)
//{
//    // Sets Port 5 as a GPIO
//    BTTN_PORT->SEL0 &= ~BUTTON;
//    BTTN_PORT->SEL1 &= ~BUTTON;
//
//    // Sets BUTTON as inputs
//    BTTN_PORT->DIR &= ~BUTTON;
//
//    // Enable resistor
//    BTTN_PORT->REN |= BUTTON;
//    // Set resistor as a pull-up
//    BTTN_PORT->OUT |= BUTTON;
//}

//void enable_ButtonInterrupt(void)
//{
//    BTTN_PORT->IES |= BUTTON;
//    BTTN_PORT->IE  |= BUTTON;
//    BTTN_PORT->IFG = 0; // Clears falgs
//}

//void PORT5_IRQHandler(void)
//{
//    if(BTTN_PORT->IFG & BUTTON)
//        if(button_pressed())
//        {
//            WDCount = 0;
//            WDT_A->CTL=0x5A00 // WatchdogPassword
//                    |1<<5 //Set to ACLK
//                    |1<<4 //Set to interval mode
//                    |1<<3 // Clear Timer
//                    |4; //Set to 2^15 interval (1 seconds)
//        }
//
//    BTTN_PORT->IFG = 0; // Clears flags
//}

//uint8_t button_pressed(void)
//{
//    // Checks for button press
//    if(!(BTTN_PORT->IN & BUTTON))
//    {
//        Delay1ms(40); // Waits 40ms before checking button for press
//
//        // Checks to see if the button is still pressed
//        if(!(BTTN_PORT->IN & BUTTON))
//            return TRUE;
//    }
//    return FALSE;
//}

void WDInit()
{
    WDT_A->CTL=0x5A00 // WatchdogPassword
            |1<<5 //Set to ACLK
            |1<<4 //Set to interval mode
            |1<<3 // Clear Timer
            |4; //Set to 2^15 interval (1 seconds)
}

void WDT_A_IRQHandler(void)
{
    WDCount++;
    if(WDCount == WDResetCount)
    {
        WDT_A->CTL=0x5A00 // WatchdogPassword
                |1<<5 //Set to ACLK
                |0<<4 //Set to Watchdog mode
                |1<<3 // Clear Timer
                |4; //Set to 2^15 interval (1 seconds)
    }
}


void resetWDCount()
{
    WDInit();
    WDCount = 0;
}
