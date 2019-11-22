/*
 * --- Slave Code ---
 */

#include "msp.h"
#include <stdio.h>
#include <stdlib.h>
#include "notes.h"

NOTE songPicked[6][389];

void SetupTimer32s();
void setupPins();

int alarmFlag = 0;
int song = 3;
int stopSong = 0;
int playAllFlag = 0;
int note = 0;       //The note in the music sequence we are on
int breath = 0;     //Take a breath after each note.  This creates separation
int bpm = 180;
int cutTime = 0;
float bps;


void initMarioTheme();
void initTetrisTheme();
void initPokemonTheme();
void initLoZTheme();
void playMario();
void playLoZ();
void playTetris();
void playPokemon();
void initAllSongs();
void playAll();
void initAlarm1(); //Shadow of the Colossus
void initAlarm2(); //Undertale
void endSong();
void init48MHz();
void DisableTimer32s();






// Definitions
#define LED2_RED BIT0
#define LED2_GREEN BIT1
#define LED2_BLUE BIT2
#define SLAVE_ADDRESS 0x48

// Global Variable
char RXData;



void main(void)
{
    WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;     // stop watchdog timer

    // ---------- Code was copied from Lecture 7 by Dr. Krug ----------
    // ----------------------------------------------------------------
    P2->DIR = LED2_RED | LED2_GREEN | LED2_BLUE;
    P2->OUT = 0x00;

    P1->SEL0 |= BIT6 | BIT7; // P1.6 and P1.7 as UCB0SDA and UCB0SCL

    EUSCI_B0->CTLW0 |= EUSCI_B_CTLW0_SWRST; // Hold EUSCI_B0 module in reset state
    EUSCI_B0->CTLW0 |= EUSCI_B_CTLW0_MODE_3|EUSCI_B_CTLW0_SYNC;
    EUSCI_B0->I2COA0 = SLAVE_ADDRESS | EUSCI_B_I2COA0_OAEN;
    EUSCI_B0->CTLW0 &= ~EUSCI_B_CTLW0_SWRST; // Clear SWRST to resume operation
    EUSCI_B0->IFG &= ~EUSCI_B_IFG_RXIFG0; // Clear EUSCI_B0 RX interrupt flag
    EUSCI_B0->IE |= EUSCI_B_IE_RXIE0; // Enable EUSCI_B0 RX interrupt

    NVIC->ISER[0] = 0x00100000; // EUSCI_B0 interrupt is enabled in NVIC
    __enable_irq(); // All interrupts are enabled


    init48MHz();
    initMarioTheme();
    initTetrisTheme();
    initPokemonTheme();
    initLoZTheme();
    initAlarm1();
    initAlarm2();
    //initAllSongs();
    //SetupTimer32s();                                //Initializes Timer32_1 as a non-interrupt timer and Timer32_2 as a interrupt timers.  Also initializes TimerA and P2.4 for music generation.
    //    setupPins();
    //playAll();

    __enable_interrupt();                           //Enable all interrupts for MSP432 (that are turned on)



    srand(5); //seed random number generator

    SCB->SCR |= SCB_SCR_SLEEPONEXIT_Msk; // Sleep on exit
    __sleep(); // enter LPM0
}

/******************************************
 * Function: Interrupt handler for I2C
 *    Input: none
 *   Output: none
 ******************************************/
void EUSCIB0_IRQHandler(void)
{
    int temp;
    uint32_t status = EUSCI_B0->IFG; // Get EUSCI_B0 interrupt flag
    EUSCI_B0->IFG &= ~EUSCI_B_IFG_RXIFG0; // Clear EUSCI_B0 RX interrupt flag

    if(status & EUSCI_B_IFG_RXIFG0) // Check if receive interrupt occurs
    {
        RXData = EUSCI_B0->RXBUF; // Load current RXData value to transmit buffer
        //        if(RXData == '0')
        //            P2->OUT ^= LED2_RED;
        //        else if(RXData == '1')
        //            P2->OUT ^= LED2_GREEN;
        //        else if(RXData == '2')
        //            P2->OUT ^= LED2_BLUE;

            temp = RXData - '0';
            if(0 <= temp && 5>= temp)
            {
                song = temp;
                stopSong = 0;
                SetupTimer32s();
                note = 0;
            }else if(temp == 6)
            {
                note = 0;
                stopSong = 0;
                DisableTimer32s();
            }else if(temp == 7)
            {
                alarmFlag = 1;
            }else if(temp == 8)
            {
                stopSong = 0;
                alarmFlag = 0;
            }

    }
}


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


void playMario()
{
    stopSong = 0;
    song = 0;
    //    SetupTimer32s();
}

void playLoZ()
{
    stopSong = 0;
    song = 1;
    //    SetupTimer32s();
}

void playTetris()
{
    stopSong = 0;
    song = 2;
    //    SetupTimer32s();
}

void playPokemon()
{
    stopSong = 0;
    song = 3;
    //    SetupTimer32s();
}

void playAlarm1()
{
    stopSong = 0;
    song = 4;
}

void playAlarm2()
{
    stopSong = 0;
    song = 5;
}



void initAllSongs()
{
    //int i;
    //initialize Mario
    //    for(i = 0; i <= 389; i++)
    //    {
    //        songPicked[0][i].note = mario[i].note;
    //        songPicked[0][i].length = mario[i].length;
    //        songPicked[0][i].slur = mario[i].slur;
    //        songPicked[0][i].accent = mario[i].accent;
    //        songPicked[0][i].end = mario[i].end;
    //    }
    //
    //    //initialize Legend of Zelda
    //    for(i = 0; i < 237; i++)
    //    {
    //        songPicked[1][i].note = LoZ[i].note;
    //        songPicked[1][i].length = LoZ[i].length;
    //        songPicked[1][i].slur = LoZ[i].slur;
    //        songPicked[1][i].accent = LoZ[i].accent;
    //        songPicked[1][i].end = LoZ[i].end;
    //    }
    //
    //    //initialize Tetris
    //    for(i = 0; i < 87; i++)
    //    {
    //        songPicked[2][i].note = Tetris[i].note;
    //        songPicked[2][i].length = Tetris[i].length;
    //        songPicked[2][i].slur = Tetris[i].slur;
    //        songPicked[2][i].accent = Tetris[i].accent;
    //        songPicked[2][i].end = Tetris[i].end;
    //    }
    //
    //    //initialize Pokemon
    //    for(i = 0; i < 153; i++)
    //    {
    //        songPicked[3][i].note = Pokemon[i].note;
    //        songPicked[3][i].length = Pokemon[i].length;
    //        songPicked[3][i].slur = Pokemon[i].slur;
    //        songPicked[3][i].accent = Pokemon[i].accent;
    //        songPicked[3][i].end = Pokemon[i].end;
    //    }
}

void playAll()
{

    stopSong = 0;
    switch(song)
    {
    case 0:
        song++;
        break;
    case 1:
        song++;
        break;
    case 2:
        song++;
        break;
    case 3:
        song++;
        break;
    case 4:
        song++;
        break;
    case 5:
        song = 0;
        break;
    default:
        song = 0;
        break;
    }
}

/*-------------------------------------------------------------------------------------------------------------------------------
 *
 * void T32_INT2_IRQHandler()
 *
 * Interrupt Handler for Timer 2.  The name of this function is set in startup_msp432p401r_ccs.c
 *
 * This handler clears the status of the interrupt for Timer32_2
 *
 * Sets up the next note to play in sequence and loads it into TimerA for play back at that frequency.
 * Enables a new Timer32 value to interrupt after the note is complete.
 *
-------------------------------------------------------------------------------------------------------------------------------*/


void T32_INT2_IRQHandler()
{
    switch(song)
    {
    case 0:
        bpm = 100;
        cutTime = 1;
        if(cutTime)
            bpm *=2;
        bps = bpm/60.0;
        break;
    case 1:
        bpm = 130;
        cutTime = 0;
        bps = bpm/60.0;
        break;
    case 2:
        cutTime = 0;
        if(note >= 46)
        {
            bpm = 220;
            bps = bpm/60.0;

        }else
        {
            bpm = 205;
            bps = bpm/60.0;
        }
        break;
    case 3:
        cutTime = 0;
        if(note >= 35)
        {
            bpm = 120;
            bps = bpm/60.0;

        }else
        {
            bpm = 180;
            bps = bpm/60.0;
        }
        break;
    case 4:
        bpm = 105;
        cutTime = 0;
        bps = bpm/60.0;
        break;
    case 5:
        bpm = 240;
        cutTime = 0;
        bps = bpm/60.0;
        break;
    default:
        song = 0;
        break;
    }
    TIMER32_2->INTCLR = 1;                                      //Clear interrupt flag so it does not interrupt again immediately.
    if(breath) {                                                //Provides separation between notes
        TIMER_A0->CCR[0] = 0;                                   //Set output of TimerA to 0
        TIMER_A0->CCR[1] = 0;
        TIMER_A0->CCR[2] = 0;
        TIMER32_2->LOAD = (48000000 * BREATH_TIME) / bps;                          //Load in breath time to interrupt again
        breath = 0;                                             //Next Timer32 interrupt is no longer a breath, but is a note
        //Turns off LED
        P2 -> OUT &= ~(BIT0 | BIT1 | BIT2);
    }
    else {                                                      //If not a breath (a note)

        TIMER32_2->LOAD = ((48000000 * songPicked[song][note].length) / bps)- 1;     //Load into interrupt count down the length of this note

        if(songPicked[song][note].note == REST) {              //If note is actually a rest, load in nothing to TimerA
            //   TIMER_A0->CCR[0] = 0;
            TIMER_A0->CCR[1] = 0;
            TIMER_A0->CCR[2] = 0;
        }
        else {
            TIMER_A0->CCR[0] = 3000000 * 4/ songPicked[song][note].note;  //Math in an interrupt is bad behavior, but shows how things are happening.  This takes our clock and divides by the frequency of this note to get the period.
            TIMER_A0->CCR[1] = 1500000 * 4/ songPicked[song][note].note;  //50% duty cycle
            TIMER_A0->CCR[2] = TIMER_A0->CCR[0];                        //Had this in here for fun with interrupts.  Not used right now
            if(songPicked[song][note].accent)
            {
                __delay_cycles(2400000);
                TIMER_A0->CCR[0] = 0;  //Math in an interrupt is bad behavior, but shows how things are happening.  This takes our clock and divides by the frequency of this note to get the period.
                TIMER_A0->CCR[1] = 0;  //50% duty cycle
                TIMER_A0->CCR[2] = TIMER_A0->CCR[0];                        //Had this in here for fun with interrupts.  Not used right now

            }
        }
        note = note + 1;                                                //Next note
        if(songPicked[song][note].end) {                                          //Go back to the beginning if at the end
            note = 0;
            if(playAllFlag && !stopSong)
            {
                playAll();
            }else if(alarmFlag && !stopSong)
            {
                DisableTimer32s();
            }
        }
        if(!songPicked[song][note].slur){
            breath = 1;                                             //Next time through should be a breath for separation.
        }else
        {
            breath = 0;
        }
        if(stopSong && !alarmFlag)
        {
            DisableTimer32s();
        }else if(stopSong && alarmFlag && (song == 4 || song == 5))
        {
            DisableTimer32s();
        }
    }
}


/*-------------------------------------------------------------------------------------------------------------------------------
 *
 * void TA0_N_IRQHandler()
 *
 * Interrupt Handler for Timer A0.  The name of this function is set in startup_msp432p401r_ccs.c
 *
 * This handler clears the status of the interrupt for Timer32_A0 CCTL 1 and 2
 *
 * Toggles P2.0 for every interrupt (LED2 Red) through cooperation with Timer_A0
 * Turns on when CCTL1 interrupts.  Turns off with CCTL2 interrupts.
 *
-------------------------------------------------------------------------------------------------------------------------------*/

void TA0_N_IRQHandler()
{
    if(TIMER_A0->CCTL[1] & BIT0) {                  //If CCTL1 is the reason for the interrupt (BIT0 holds the flag)
    }
    if(TIMER_A0->CCTL[2] & BIT0) {                  //If CCTL1 is the reason for the interrupt (BIT0 holds the flag)
    }
}
/*-------------------------------------------------------------------------------------------------------------------------------
 *
 * void SetupTimer32s()
 *
 * Configures Timer32_1 as a single shot (runs once) timer that does not interrupt so the value must be monitored.
 * Configures Timer32_2 as a single shot (runs once) timer that does interrupt and will run the interrupt handler 1 second
 * after this function is called (and the master interrupt is enabled).
 *
-------------------------------------------------------------------------------------------------------------------------------*/

void SetupTimer32s()
{
    TIMER32_1->CONTROL = 0b11000011;                //Sets timer 1 for Enabled, Periodic, No Interrupt, No Prescaler, 32 bit mode, One Shot Mode.  See 589 of the reference manual
    TIMER32_2->CONTROL = 0b11100011;                //Sets timer 2 for Enabled, Periodic, With Interrupt, No Prescaler, 32 bit mode, One Shot Mode.  See 589 of the reference manual
    NVIC_EnableIRQ(T32_INT2_IRQn);                  //Enable Timer32_2 interrupt.  Look at msp.h if you want to see what all these are called.
    TIMER32_2->LOAD = 48000000 - 1;                  //Set to a count down of 1 second on 48 MHz clock

    TIMER_A0->CCR[0] = 0;                           // Turn off timerA to start
    TIMER_A0->CCTL[1] = 0b0000000011110100;         // Setup Timer A0_1 Reset/Set, Interrupt, No Output
    TIMER_A0->CCR[1] = 0;                           // Turn off timerA to start
    TIMER_A0->CCTL[2] = 0b0000000011110100;         // Setup Timer A0_2 Reset/Set, Interrupt, No Output
    TIMER_A0->CCR[2] = 0;                           // Turn off timerA to start
    TIMER_A0->CTL = 0b0000001000010100;             // Count Up mode using SMCLK, Clears, Clear Interrupt Flag

    NVIC_EnableIRQ(TA0_N_IRQn);                     // Enable interrupts for CCTL1-6 (if on)

    P2->SEL0 |= BIT4;                               // Setup the P2.4 to be an output for the song.  This should drive a sounder.
    P2->SEL1 &= ~BIT4;
    P2->DIR |= BIT4;

}

void DisableTimer32s()
{
    TIMER32_1->CONTROL = 0;                //Sets timer 1 for Enabled, Periodic, No Interrupt, No Prescaler, 32 bit mode, One Shot Mode.  See 589 of the reference manual
    TIMER32_2->CONTROL = 0;                //Sets timer 2 for Enabled, Periodic, With Interrupt, No Prescaler, 32 bit mode, One Shot Mode.  See 589 of the reference manual
    NVIC_EnableIRQ(T32_INT2_IRQn);                  //Enable Timer32_2 interrupt.  Look at msp.h if you want to see what all these are called.
    TIMER32_2->LOAD = 48000000 - 1;                  //Set to a count down of 1 second on 48 MHz clock

    TIMER_A0->CCR[0] = 0;                           // Turn off timerA to start
    TIMER_A0->CCTL[1] = 0;         // Setup Timer A0_1 Reset/Set, Interrupt, No Output
    TIMER_A0->CCR[1] = 0;                           // Turn off timerA to start
    TIMER_A0->CCTL[2] = 0;         // Setup Timer A0_2 Reset/Set, Interrupt, No Output
    TIMER_A0->CCR[2] = 0;                           // Turn off timerA to start
    TIMER_A0->CTL = 0;             // Count Up mode using SMCLK, Clears, Clear Interrupt Flag

    NVIC_EnableIRQ(TA0_N_IRQn);                     // Enable interrupts for CCTL1-6 (if on)

    P2->SEL0 |= BIT4;                               // Setup the P2.4 to be an output for the song.  This should drive a sounder.
    P2->SEL1 &= ~BIT4;
    P2->DIR |= BIT4;

}

//Sets up LED pins and button pins
void setupPins(){
    //    P1 -> SEL0 &= ~(BIT1|BIT4);
    //    P1 -> SEL1 &= ~(BIT1|BIT4);
    //    P1 -> DIR &= ~(BIT1|BIT4);
    //    P1 -> REN |= (BIT1|BIT4);
    //    P1 -> OUT |= (BIT1|BIT4);
    //    P1 -> IE |= (BIT1|BIT4);
    //    NVIC_EnableIRQ(PORT1_IRQn);
    //
    //    P1->SEL0 &= ~BIT0;
    //    P1->SEL1 &= ~BIT0;
    //    P1->DIR  |=  BIT0;
    //    P1->OUT  &= ~BIT0;
    //
    //    //LED RED
    //    P2->SEL0 &= ~BIT0;
    //    P2->SEL1 &= ~BIT0;
    //    P2->DIR |= BIT0;
    //    P2 -> OUT &= ~BIT0;
    //    //LED GREEN
    //    P2->SEL0 &= ~BIT1;
    //    P2->SEL1 &= ~BIT1;
    //    P2->DIR |= BIT1;
    //    P2 -> OUT &= ~BIT1;
    //    //LED BLUE
    //    P2->SEL0 &= ~BIT2;
    //    P2->SEL1 &= ~BIT2;
    //    P2->DIR |= BIT2;
    //    P2 -> OUT &= ~BIT2;
}

////handles the button interrupts
//void PORT1_IRQHandler(void){
//    if(P1 -> IFG & BIT1){
//        P1 -> IFG &= ~BIT1 ;
//        bps++;
//    }
//    if(P1 -> IFG & BIT4){
//        P1 -> IFG &= ~BIT4 ;
//        bps--;
//    }
//}

void initLoZTheme()
{
    //MEASURE 1
    songPicked[1][0].note = A4;
    songPicked[1][0].length = HALF;
    songPicked[1][1].note = REST;
    songPicked[1][1].length = ETRIPLET + ETRIPLET;
    songPicked[1][2].note = A4;
    songPicked[1][2].length = ETRIPLET;
    songPicked[1][3].note = A4;
    songPicked[1][3].length = ETRIPLET;
    songPicked[1][4].note = A4;
    songPicked[1][4].length = ETRIPLET;
    songPicked[1][5].note = A4;
    songPicked[1][5].length = ETRIPLET;

    //MEASURE 2
    songPicked[1][6].note = A4;
    songPicked[1][6].length = ETRIPLET + ETRIPLET;
    songPicked[1][7].note = G4;
    songPicked[1][7].length = ETRIPLET;
    songPicked[1][8].note = A4;
    songPicked[1][8].length = QUARTER;
    songPicked[1][9].note = REST;
    songPicked[1][9].length = ETRIPLET + ETRIPLET;
    songPicked[1][10].note = A4;
    songPicked[1][10].length = ETRIPLET;
    songPicked[1][11].note = A4;
    songPicked[1][11].length = ETRIPLET;
    songPicked[1][12].note = A4;
    songPicked[1][12].length = ETRIPLET;
    songPicked[1][13].note = A4;
    songPicked[1][13].length = ETRIPLET;

    //MEASURE 3
    songPicked[1][14].note = A4;
    songPicked[1][14].length = ETRIPLET + ETRIPLET;
    songPicked[1][15].note = G4;
    songPicked[1][15].length = ETRIPLET;
    songPicked[1][16].note = A4;
    songPicked[1][16].length = QUARTER;
    songPicked[1][17].note = REST;
    songPicked[1][17].length = ETRIPLET + ETRIPLET;
    songPicked[1][18].note = A4;
    songPicked[1][18].length = ETRIPLET;
    songPicked[1][19].note = A4;
    songPicked[1][19].length = ETRIPLET;
    songPicked[1][20].note = A4;
    songPicked[1][20].length = ETRIPLET;
    songPicked[1][21].note = A4;
    songPicked[1][21].length = ETRIPLET;

    //MEASURE 4
    songPicked[1][22].note = A4;
    songPicked[1][22].length = EIGHTH;
    songPicked[1][23].note = E4;
    songPicked[1][23].length = SIXTEENTH;
    songPicked[1][24].note = E4;
    songPicked[1][24].length = SIXTEENTH;
    songPicked[1][25].note = E4;
    songPicked[1][25].length = EIGHTH;
    songPicked[1][26].note = E4;
    songPicked[1][26].length = SIXTEENTH;
    songPicked[1][27].note = E4;
    songPicked[1][27].length = SIXTEENTH;
    songPicked[1][28].note = E4;
    songPicked[1][28].length = EIGHTH;
    songPicked[1][29].note = E4;
    songPicked[1][29].length = SIXTEENTH;
    songPicked[1][30].note = E4;
    songPicked[1][30].length = SIXTEENTH;
    songPicked[1][31].note = E4;
    songPicked[1][31].length = EIGHTH;
    songPicked[1][32].note = E4;
    songPicked[1][32].length = EIGHTH;

    //REPEAT
    //MEASURE 5
    songPicked[1][33].note = A4;
    songPicked[1][33].length = QUARTER;
    songPicked[1][34].note = E4;
    songPicked[1][34].length = QUARTER * DOTTED;
    songPicked[1][35].note = A4;
    songPicked[1][35].length = EIGHTH;
    songPicked[1][36].note = A4;
    songPicked[1][36].length = SIXTEENTH;
    songPicked[1][37].note = B4;
    songPicked[1][37].length = SIXTEENTH;
    songPicked[1][38].note = Cs5;
    songPicked[1][38].length = SIXTEENTH;
    songPicked[1][39].note = D5;
    songPicked[1][39].length = SIXTEENTH;

    //MEASURE 6
    songPicked[1][40].note = E5;
    songPicked[1][40].length = HALF;
    songPicked[1][41].note = REST;
    songPicked[1][41].length = EIGHTH;
    songPicked[1][42].note = E5;
    songPicked[1][42].length = EIGHTH;
    songPicked[1][43].note = E5;
    songPicked[1][43].length = ETRIPLET;
    songPicked[1][44].note = F5;
    songPicked[1][44].length = ETRIPLET;
    songPicked[1][45].note = G5;
    songPicked[1][45].length = ETRIPLET;

    //MEASURE 7
    songPicked[1][46].note = A5;
    songPicked[1][46].length = HALF;
    songPicked[1][47].note = REST;
    songPicked[1][47].length = EIGHTH;
    songPicked[1][48].note = A5;
    songPicked[1][48].length = EIGHTH;
    songPicked[1][49].note = A5;
    songPicked[1][49].length = ETRIPLET;
    songPicked[1][50].note = G5;
    songPicked[1][50].length = ETRIPLET;
    songPicked[1][51].note = F5;
    songPicked[1][51].length = ETRIPLET;

    //MEAUSURE 8
    songPicked[1][52].note = G5;
    songPicked[1][52].length = EIGHTH * DOTTED;
    songPicked[1][53].note = F5;
    songPicked[1][53].length = SIXTEENTH;
    songPicked[1][54].note = E5;
    songPicked[1][54].length = HALF;
    songPicked[1][55].note = E5;
    songPicked[1][55].length = QUARTER;

    //MEASURE 9
    songPicked[1][56].note = D5;
    songPicked[1][56].length = EIGHTH;
    songPicked[1][57].note = D5;
    songPicked[1][57].length = SIXTEENTH;
    songPicked[1][58].note = E5;
    songPicked[1][58].length = SIXTEENTH;
    songPicked[1][59].note = F5;
    songPicked[1][59].length = HALF;
    songPicked[1][60].note = E5;
    songPicked[1][60].length = EIGHTH;
    songPicked[1][61].note = D5;
    songPicked[1][61].length = EIGHTH;

    //MEASURE 10
    songPicked[1][62].note = C5;
    songPicked[1][62].length = EIGHTH;
    songPicked[1][63].note = C5;
    songPicked[1][63].length = SIXTEENTH;
    songPicked[1][64].note = D5;
    songPicked[1][64].length = SIXTEENTH;
    songPicked[1][65].note = E5;
    songPicked[1][65].length = HALF;
    songPicked[1][66].note = D5;
    songPicked[1][66].length = EIGHTH;
    songPicked[1][67].note = C5;
    songPicked[1][67].length = EIGHTH;

    //MEASURE 11
    songPicked[1][68].note = B5;
    songPicked[1][68].length = EIGHTH;
    songPicked[1][69].note = B5;
    songPicked[1][69].length = SIXTEENTH;
    songPicked[1][70].note = C5;
    songPicked[1][70].length = SIXTEENTH;
    songPicked[1][71].note = D5;
    songPicked[1][71].length = HALF;
    songPicked[1][72].note = F5;
    songPicked[1][72].length = QUARTER;

    //MEASURE 12
    songPicked[1][73].note = E5;
    songPicked[1][73].length = EIGHTH;
    songPicked[1][74].note = E4;
    songPicked[1][74].length = SIXTEENTH;
    songPicked[1][75].note = E4;
    songPicked[1][75].length = SIXTEENTH;
    songPicked[1][76].note = E4;
    songPicked[1][76].length = EIGHTH;
    songPicked[1][77].note = E4;
    songPicked[1][77].length = SIXTEENTH;
    songPicked[1][78].note = E4;
    songPicked[1][78].length = SIXTEENTH;
    songPicked[1][79].note = E4;
    songPicked[1][79].length = EIGHTH;
    songPicked[1][80].note = E4;
    songPicked[1][80].length = SIXTEENTH;
    songPicked[1][81].note = E4;
    songPicked[1][81].length = SIXTEENTH;
    songPicked[1][82].note = E4;
    songPicked[1][82].length = EIGHTH;
    songPicked[1][83].note = E4;
    songPicked[1][83].length = EIGHTH;

    //MEASURE 13
    songPicked[1][84].note = A4;
    songPicked[1][84].length = QUARTER;
    songPicked[1][85].note = E4;
    songPicked[1][85].length = QUARTER * DOTTED;
    songPicked[1][86].note = A4;
    songPicked[1][86].length = EIGHTH;
    songPicked[1][87].note = A4;
    songPicked[1][87].length = SIXTEENTH;
    songPicked[1][88].note = B4;
    songPicked[1][88].length = SIXTEENTH;
    songPicked[1][89].note = Cs5;
    songPicked[1][89].length = SIXTEENTH;
    songPicked[1][90].note = D5;
    songPicked[1][90].length = SIXTEENTH;

    //MEASURE 14
    songPicked[1][91].note = E5;
    songPicked[1][91].length = HALF;
    songPicked[1][92].note = REST;
    songPicked[1][92].length = EIGHTH;
    songPicked[1][93].note = E5;
    songPicked[1][93].length = EIGHTH;
    songPicked[1][94].note = E5;
    songPicked[1][94].length = ETRIPLET;
    songPicked[1][95].note = F5;
    songPicked[1][95].length = ETRIPLET;
    songPicked[1][96].note = G5;
    songPicked[1][96].length = ETRIPLET;

    //MEASURE 15
    songPicked[1][97].note = A5;
    songPicked[1][97].length = HALF;
    songPicked[1][98].note = REST;
    songPicked[1][98].length = QUARTER;
    songPicked[1][99].note = C6;
    songPicked[1][99].length = QUARTER;

    //MEASURE 16
    songPicked[1][100].note = B5;
    songPicked[1][100].length = QUARTER;
    songPicked[1][101].note = G5;
    songPicked[1][101].length = HALF;
    songPicked[1][102].note = E5;
    songPicked[1][102].length = QUARTER;

    //MEASURE 17
    songPicked[1][103].note = F5;
    songPicked[1][103].length = HALF * DOTTED;
    songPicked[1][104].note = A5;
    songPicked[1][104].length = QUARTER;

    //MEASURE 18
    songPicked[1][105].note = G5;
    songPicked[1][105].length = QUARTER;
    songPicked[1][106].note = E5;
    songPicked[1][106].length = HALF;
    songPicked[1][107].note = E5;
    songPicked[1][107].length = QUARTER;

    //MEASURE 19
    songPicked[1][108].note = F5;
    songPicked[1][108].length = HALF * DOTTED;
    songPicked[1][109].note = A5;
    songPicked[1][109].length = QUARTER;

    //MEASURE 20
    songPicked[1][110].note = Gs5;
    songPicked[1][110].length = QUARTER;
    songPicked[1][111].note = E5;
    songPicked[1][111].length = HALF;
    songPicked[1][112].note = Cs5;
    songPicked[1][112].length = QUARTER;

    //MEASURE 21
    songPicked[1][113].note = D5;
    songPicked[1][113].length = HALF * DOTTED;
    songPicked[1][114].note = F5;
    songPicked[1][114].length = QUARTER;

    //MEASURE 22
    songPicked[1][115].note = E5;
    songPicked[1][115].length = QUARTER;
    songPicked[1][116].note = C5;
    songPicked[1][116].length = HALF;
    songPicked[1][117].note = A4;
    songPicked[1][117].length = QUARTER;

    //MEASURE 23
    songPicked[1][118].note = B4;
    songPicked[1][118].length = EIGHTH;
    songPicked[1][119].note = B4;
    songPicked[1][119].length = SIXTEENTH;
    songPicked[1][120].note = C5;
    songPicked[1][120].length = SIXTEENTH;
    songPicked[1][121].note = D5;
    songPicked[1][121].length = HALF;
    songPicked[1][122].note = F5;
    songPicked[1][122].length = QUARTER;

    //MEASURE 24
    songPicked[1][123].note = E5;
    songPicked[1][123].length = EIGHTH;
    songPicked[1][124].note = E4;
    songPicked[1][124].length = SIXTEENTH;
    songPicked[1][125].note = E4;
    songPicked[1][125].length = SIXTEENTH;
    songPicked[1][126].note = E4;
    songPicked[1][126].length = EIGHTH;
    songPicked[1][127].note = E4;
    songPicked[1][127].length = SIXTEENTH;
    songPicked[1][128].note = E4;
    songPicked[1][128].length = SIXTEENTH;
    songPicked[1][129].note = E4;
    songPicked[1][129].length = EIGHTH;
    songPicked[1][130].note = E4;
    songPicked[1][130].length = SIXTEENTH;
    songPicked[1][131].note = E4;
    songPicked[1][131].length = SIXTEENTH;
    songPicked[1][132].note = E4;
    songPicked[1][132].length = EIGHTH;
    songPicked[1][133].note = E4;
    songPicked[1][133].length = EIGHTH;

    //START REPEAT
    //MEASURE 5
    songPicked[1][134].note = A4;
    songPicked[1][134].length = QUARTER;
    songPicked[1][135].note = E4;
    songPicked[1][135].length = QUARTER * DOTTED;
    songPicked[1][136].note = A4;
    songPicked[1][136].length = EIGHTH;
    songPicked[1][137].note = A4;
    songPicked[1][137].length = SIXTEENTH;
    songPicked[1][138].note = B4;
    songPicked[1][138].length = SIXTEENTH;
    songPicked[1][139].note = Cs5;
    songPicked[1][139].length = SIXTEENTH;
    songPicked[1][140].note = D5;
    songPicked[1][140].length = SIXTEENTH;

    //MEASURE 6
    songPicked[1][141].note = E5;
    songPicked[1][141].length = HALF;
    songPicked[1][142].note = REST;
    songPicked[1][142].length = EIGHTH;
    songPicked[1][143].note = E5;
    songPicked[1][143].length = EIGHTH;
    songPicked[1][144].note = E5;
    songPicked[1][144].length = ETRIPLET;
    songPicked[1][145].note = F5;
    songPicked[1][145].length = ETRIPLET;
    songPicked[1][146].note = G5;
    songPicked[1][146].length = ETRIPLET;

    //MEASURE 7
    songPicked[1][147].note = A5;
    songPicked[1][147].length = HALF;
    songPicked[1][148].note = REST;
    songPicked[1][148].length = EIGHTH;
    songPicked[1][149].note = A5;
    songPicked[1][149].length = EIGHTH;
    songPicked[1][150].note = A5;
    songPicked[1][150].length = ETRIPLET;
    songPicked[1][151].note = G5;
    songPicked[1][151].length = ETRIPLET;
    songPicked[1][152].note = F5;
    songPicked[1][152].length = ETRIPLET;

    //MEAUSURE 8
    songPicked[1][153].note = G5;
    songPicked[1][153].length = EIGHTH * DOTTED;
    songPicked[1][154].note = F5;
    songPicked[1][154].length = SIXTEENTH;
    songPicked[1][155].note = E5;
    songPicked[1][155].length = HALF;
    songPicked[1][156].note = E5;
    songPicked[1][156].length = QUARTER;

    //MEASURE 9
    songPicked[1][157].note = D5;
    songPicked[1][157].length = EIGHTH;
    songPicked[1][158].note = D5;
    songPicked[1][158].length = SIXTEENTH;
    songPicked[1][159].note = E5;
    songPicked[1][159].length = SIXTEENTH;
    songPicked[1][160].note = F5;
    songPicked[1][160].length = HALF;
    songPicked[1][161].note = E5;
    songPicked[1][161].length = EIGHTH;
    songPicked[1][162].note = D5;
    songPicked[1][162].length = EIGHTH;

    //MEASURE 10
    songPicked[1][163].note = C5;
    songPicked[1][163].length = EIGHTH;
    songPicked[1][164].note = C5;
    songPicked[1][164].length = SIXTEENTH;
    songPicked[1][165].note = D5;
    songPicked[1][165].length = SIXTEENTH;
    songPicked[1][166].note = E5;
    songPicked[1][166].length = HALF;
    songPicked[1][167].note = D5;
    songPicked[1][167].length = EIGHTH;
    songPicked[1][168].note = C5;
    songPicked[1][168].length = EIGHTH;

    //MEASURE 11
    songPicked[1][169].note = B5;
    songPicked[1][169].length = EIGHTH;
    songPicked[1][170].note = B5;
    songPicked[1][170].length = SIXTEENTH;
    songPicked[1][171].note = C5;
    songPicked[1][171].length = SIXTEENTH;
    songPicked[1][172].note = D5;
    songPicked[1][172].length = HALF;
    songPicked[1][173].note = F5;
    songPicked[1][173].length = QUARTER;

    //MEASURE 12
    songPicked[1][174].note = E5;
    songPicked[1][174].length = EIGHTH;
    songPicked[1][175].note = E4;
    songPicked[1][175].length = SIXTEENTH;
    songPicked[1][176].note = E4;
    songPicked[1][176].length = SIXTEENTH;
    songPicked[1][177].note = E4;
    songPicked[1][177].length = EIGHTH;
    songPicked[1][178].note = E4;
    songPicked[1][178].length = SIXTEENTH;
    songPicked[1][179].note = E4;
    songPicked[1][179].length = SIXTEENTH;
    songPicked[1][180].note = E4;
    songPicked[1][180].length = EIGHTH;
    songPicked[1][181].note = E4;
    songPicked[1][181].length = SIXTEENTH;
    songPicked[1][182].note = E4;
    songPicked[1][182].length = SIXTEENTH;
    songPicked[1][183].note = E4;
    songPicked[1][183].length = EIGHTH;
    songPicked[1][184].note = E4;
    songPicked[1][184].length = EIGHTH;

    //MEASURE 13
    songPicked[1][185].note = A4;
    songPicked[1][185].length = QUARTER;
    songPicked[1][186].note = E4;
    songPicked[1][186].length = QUARTER * DOTTED;
    songPicked[1][187].note = A4;
    songPicked[1][187].length = EIGHTH;
    songPicked[1][188].note = A4;
    songPicked[1][188].length = SIXTEENTH;
    songPicked[1][189].note = B4;
    songPicked[1][189].length = SIXTEENTH;
    songPicked[1][190].note = Cs5;
    songPicked[1][190].length = SIXTEENTH;
    songPicked[1][191].note = D5;
    songPicked[1][191].length = SIXTEENTH;

    //MEASURE 14
    songPicked[1][192].note = E5;
    songPicked[1][192].length = HALF;
    songPicked[1][193].note = REST;
    songPicked[1][193].length = EIGHTH;
    songPicked[1][194].note = E5;
    songPicked[1][194].length = EIGHTH;
    songPicked[1][195].note = E5;
    songPicked[1][195].length = ETRIPLET;
    songPicked[1][196].note = F5;
    songPicked[1][196].length = ETRIPLET;
    songPicked[1][197].note = G5;
    songPicked[1][197].length = ETRIPLET;

    //MEASURE 15
    songPicked[1][198].note = A5;
    songPicked[1][198].length = HALF;
    songPicked[1][199].note = REST;
    songPicked[1][199].length = QUARTER;
    songPicked[1][200].note = C6;
    songPicked[1][200].length = QUARTER;

    //MEASURE 16
    songPicked[1][201].note = B5;
    songPicked[1][201].length = QUARTER;
    songPicked[1][202].note = G5;
    songPicked[1][202].length = HALF;
    songPicked[1][203].note = E5;
    songPicked[1][203].length = QUARTER;

    //MEASURE 17
    songPicked[1][204].note = F5;
    songPicked[1][204].length = HALF * DOTTED;
    songPicked[1][205].note = A5;
    songPicked[1][205].length = QUARTER;

    //MEASURE 18
    songPicked[1][206].note = G5;
    songPicked[1][206].length = QUARTER;
    songPicked[1][207].note = E5;
    songPicked[1][207].length = HALF;
    songPicked[1][208].note = E5;
    songPicked[1][208].length = QUARTER;

    //MEASURE 19
    songPicked[1][209].note = F5;
    songPicked[1][209].length = HALF * DOTTED;
    songPicked[1][210].note = A5;
    songPicked[1][210].length = QUARTER;

    //MEASURE 20
    songPicked[1][211].note = Gs5;
    songPicked[1][211].length = QUARTER;
    songPicked[1][212].note = E5;
    songPicked[1][212].length = HALF;
    songPicked[1][213].note = Cs5;
    songPicked[1][213].length = QUARTER;

    //MEASURE 21
    songPicked[1][214].note = D5;
    songPicked[1][214].length = HALF * DOTTED;
    songPicked[1][215].note = F5;
    songPicked[1][215].length = QUARTER;

    //MEASURE 22
    songPicked[1][216].note = E5;
    songPicked[1][216].length = QUARTER;
    songPicked[1][217].note = C5;
    songPicked[1][217].length = HALF;
    songPicked[1][218].note = A4;
    songPicked[1][218].length = QUARTER;

    //MEASURE 23
    songPicked[1][219].note = B4;
    songPicked[1][219].length = EIGHTH;
    songPicked[1][220].note = B4;
    songPicked[1][220].length = SIXTEENTH;
    songPicked[1][221].note = C5;
    songPicked[1][221].length = SIXTEENTH;
    songPicked[1][222].note = D5;
    songPicked[1][222].length = HALF;
    songPicked[1][223].note = F5;
    songPicked[1][223].length = QUARTER;

    //MEASURE 24
    songPicked[1][224].note = E5;
    songPicked[1][224].length = EIGHTH;
    songPicked[1][225].note = E4;
    songPicked[1][225].length = SIXTEENTH;
    songPicked[1][226].note = E4;
    songPicked[1][226].length = SIXTEENTH;
    songPicked[1][227].note = E4;
    songPicked[1][227].length = EIGHTH;
    songPicked[1][228].note = E4;
    songPicked[1][228].length = SIXTEENTH;
    songPicked[1][229].note = E4;
    songPicked[1][229].length = SIXTEENTH;
    songPicked[1][230].note = E4;
    songPicked[1][230].length = EIGHTH;
    songPicked[1][231].note = E4;
    songPicked[1][231].length = SIXTEENTH;
    songPicked[1][232].note = E4;
    songPicked[1][232].length = SIXTEENTH;
    songPicked[1][233].note = E4;
    songPicked[1][233].length = EIGHTH;
    songPicked[1][234].note = E4;
    songPicked[1][234].length = EIGHTH;

    //END
    songPicked[1][235].note = REST;
    songPicked[1][235].length = WHOLE;
    songPicked[1][236].note = REST;
    songPicked[1][236].length = WHOLE;
    songPicked[1][236].end = 1;
}

void initMarioTheme()
{
    //Measure 1
    songPicked[0][0].note = E5;
    songPicked[0][0].length = EIGHTH;
    songPicked[0][1].note = E5;
    songPicked[0][1].length = EIGHTH;
    songPicked[0][2].note = REST;
    songPicked[0][2].length = EIGHTH;
    songPicked[0][3].note = E5;
    songPicked[0][3].length = EIGHTH;
    songPicked[0][4].note = REST;
    songPicked[0][4].length = EIGHTH;
    songPicked[0][5].note = C5;
    songPicked[0][5].length = EIGHTH;
    songPicked[0][6].note = E5;
    songPicked[0][6].length = QUARTER;


    //Measure 2
    songPicked[0][7].note = G5;
    songPicked[0][7].length = QUARTER;
    songPicked[0][8].note = REST;
    songPicked[0][8].length = QUARTER;
    songPicked[0][9].note = G4;
    songPicked[0][9].length = QUARTER;
    songPicked[0][10].note = REST;
    songPicked[0][10].length = QUARTER;

    //MEASURE 3
    songPicked[0][11].note = C5;
    songPicked[0][11].length = QUARTER * DOTTED;
    songPicked[0][12].note = G4;
    songPicked[0][12].length = EIGHTH;
    songPicked[0][13].note = REST;
    songPicked[0][13].length = QUARTER;
    songPicked[0][14].note = E4;
    songPicked[0][14].length = QUARTER;
    songPicked[0][14].slur = 1;

    //MEASURE 4
    songPicked[0][15].note = E4;
    songPicked[0][15].length = EIGHTH;
    songPicked[0][16].note = A4;
    songPicked[0][16].length = QUARTER;
    songPicked[0][17].note = B4;
    songPicked[0][17].length = EIGHTH;
    songPicked[0][18].note = B4;
    songPicked[0][18].length = EIGHTH;
    songPicked[0][19].note = Bb4;
    songPicked[0][19].length = EIGHTH;
    songPicked[0][20].note = A4;
    songPicked[0][20].length = QUARTER;

    //MEASURE 5
    songPicked[0][21].note = G4;
    songPicked[0][21].length = QTRIPLET;
    songPicked[0][22].note = E5;
    songPicked[0][22].length = QTRIPLET;
    songPicked[0][23].note = G5;
    songPicked[0][23].length = QTRIPLET;
    songPicked[0][24].note = A5;
    songPicked[0][24].length = QUARTER;
    songPicked[0][25].note = F5;
    songPicked[0][25].length = EIGHTH;
    songPicked[0][26].note = G5;
    songPicked[0][26].length = EIGHTH;

    //MEASURE 6
    songPicked[0][27].note = REST;
    songPicked[0][27].length = EIGHTH;
    songPicked[0][28].note = E5;
    songPicked[0][28].length = QUARTER;
    songPicked[0][29].note = C5;
    songPicked[0][29].length = EIGHTH;
    songPicked[0][30].note = D5;
    songPicked[0][30].length = EIGHTH;
    songPicked[0][31].note = B4;
    songPicked[0][31].length = QUARTER;
    songPicked[0][32].note = REST;
    songPicked[0][32].length = EIGHTH;

    //MEASURE 7
    songPicked[0][33].note = C5;
    songPicked[0][33].length = QUARTER * DOTTED;
    songPicked[0][34].note = G4;
    songPicked[0][34].length = EIGHTH;
    songPicked[0][35].note = REST;
    songPicked[0][35].length = QUARTER;
    songPicked[0][36].note = E4;
    songPicked[0][36].length = QUARTER;
    songPicked[0][36].slur = 1;

    //MEASURE 8
    songPicked[0][37].note = E4;
    songPicked[0][37].length = EIGHTH;
    songPicked[0][38].note = A4;
    songPicked[0][38].length = QUARTER;
    songPicked[0][39].note = B4;
    songPicked[0][39].length = QUARTER;
    songPicked[0][40].note = Bb4;
    songPicked[0][40].length = EIGHTH;
    songPicked[0][41].note = A4;
    songPicked[0][41].length = QUARTER;

    //MEASURE 9
    songPicked[0][42].note = G4;
    songPicked[0][42].length = QTRIPLET;
    songPicked[0][43].note = E5;
    songPicked[0][43].length = QTRIPLET;
    songPicked[0][44].note = G5;
    songPicked[0][44].length = QTRIPLET;
    songPicked[0][45].note = A5;
    songPicked[0][45].length = QUARTER;
    songPicked[0][46].note = F5;
    songPicked[0][46].length = EIGHTH;
    songPicked[0][47].note = G5;
    songPicked[0][47].length = EIGHTH;

    //MEASURE 10
    songPicked[0][48].note = REST;
    songPicked[0][48].length = EIGHTH;
    songPicked[0][49].note = E5;
    songPicked[0][49].length = QUARTER;
    songPicked[0][50].note = C5;
    songPicked[0][50].length = EIGHTH;
    songPicked[0][51].note = D5;
    songPicked[0][51].length = EIGHTH;
    songPicked[0][52].note = B4;
    songPicked[0][52].length = QUARTER;
    songPicked[0][53].note = REST;
    songPicked[0][53].length = EIGHTH;

    //REPEAT
    //MEASURE 11
    songPicked[0][54].note = REST;
    songPicked[0][54].length = QUARTER;
    songPicked[0][55].note = G5;
    songPicked[0][55].length = EIGHTH;
    songPicked[0][56].note = Gb5;
    songPicked[0][56].length = EIGHTH;
    songPicked[0][57].note = F5;
    songPicked[0][57].length = EIGHTH;
    songPicked[0][58].note = Eb5;
    songPicked[0][58].length = QUARTER;
    songPicked[0][59].note = E5;
    songPicked[0][59].length = EIGHTH;

    //MEASURE 12
    songPicked[0][60].note = REST;
    songPicked[0][60].length = EIGHTH;
    songPicked[0][61].note = Ab4;
    songPicked[0][61].length = ETRIPLET;
    songPicked[0][62].note = A4;
    songPicked[0][62].length = ETRIPLET;
    songPicked[0][63].note = C5;
    songPicked[0][63].length = ETRIPLET;
    songPicked[0][64].note = REST;
    songPicked[0][64].length = EIGHTH;
    songPicked[0][65].note = A4;
    songPicked[0][65].length = ETRIPLET;
    songPicked[0][66].note = C5;
    songPicked[0][66].length = ETRIPLET;
    songPicked[0][67].note = D5;
    songPicked[0][67].length = ETRIPLET;

    //MEASURE 13
    songPicked[0][68].note = REST;
    songPicked[0][68].length = QUARTER;
    songPicked[0][69].note = G5;
    songPicked[0][69].length = EIGHTH;
    songPicked[0][70].note = Gb5;
    songPicked[0][70].length = EIGHTH;
    songPicked[0][71].note = F5;
    songPicked[0][71].length = EIGHTH;
    songPicked[0][72].note = Eb5;
    songPicked[0][72].length = QUARTER;
    songPicked[0][73].note = E5;
    songPicked[0][73].length = EIGHTH;

    //MEASURE 14
    songPicked[0][74].note = REST;
    songPicked[0][74].length = EIGHTH;
    songPicked[0][75].note = C6;
    songPicked[0][75].length = EIGHTH;
    songPicked[0][76].note = REST;
    songPicked[0][76].length = EIGHTH;
    songPicked[0][77].note = C6;
    songPicked[0][77].length = EIGHTH;
    songPicked[0][78].note = C6;
    songPicked[0][78].length = HALF;

    //MEASURE 15
    songPicked[0][79].note = REST;
    songPicked[0][79].length = QUARTER;
    songPicked[0][80].note = G5;
    songPicked[0][80].length = EIGHTH;
    songPicked[0][81].note = Gb5;
    songPicked[0][81].length = EIGHTH;
    songPicked[0][82].note = F5;
    songPicked[0][82].length = EIGHTH;
    songPicked[0][83].note = Eb5;
    songPicked[0][83].length = QUARTER;
    songPicked[0][84].note = E5;
    songPicked[0][84].length = EIGHTH;

    //MEASURE 16
    songPicked[0][85].note = REST;
    songPicked[0][85].length = EIGHTH;
    songPicked[0][86].note = Ab4;
    songPicked[0][86].length = ETRIPLET;
    songPicked[0][87].note = A4;
    songPicked[0][87].length = ETRIPLET;
    songPicked[0][88].note = C5;
    songPicked[0][88].length = ETRIPLET;
    songPicked[0][89].note = REST;
    songPicked[0][89].length = EIGHTH;
    songPicked[0][90].note = A4;
    songPicked[0][90].length = ETRIPLET;
    songPicked[0][91].note = C5;
    songPicked[0][91].length = ETRIPLET;
    songPicked[0][92].note = D5;
    songPicked[0][92].length = ETRIPLET;

    //MEASURE 17
    songPicked[0][93].note = REST;
    songPicked[0][93].length = QUARTER;
    songPicked[0][94].note = Eb5;
    songPicked[0][94].length = QUARTER;
    songPicked[0][95].note = REST;
    songPicked[0][95].length = EIGHTH;
    songPicked[0][95].note = D5;
    songPicked[0][96].length = QUARTER;
    songPicked[0][97].note = REST;
    songPicked[0][97].length = EIGHTH;

    //MEASURE 18
    songPicked[0][98].note = C5;
    songPicked[0][98].length = HALF;
    songPicked[0][99].note = REST;
    songPicked[0][99].length = HALF;


    //START REPEAT 1
    //MEASURE 11
    songPicked[0][100].note = REST;
    songPicked[0][100].length = QUARTER;
    songPicked[0][101].note = G5;
    songPicked[0][101].length = EIGHTH;
    songPicked[0][102].note = Gb5;
    songPicked[0][102].length = EIGHTH;
    songPicked[0][103].note = F5;
    songPicked[0][103].length = EIGHTH;
    songPicked[0][104].note = Eb5;
    songPicked[0][104].length = QUARTER;
    songPicked[0][105].note = E5;
    songPicked[0][105].length = EIGHTH;

    //MEASURE 12
    songPicked[0][106].note = REST;
    songPicked[0][106].length = EIGHTH;
    songPicked[0][107].note = Ab4;
    songPicked[0][107].length = ETRIPLET;
    songPicked[0][108].note = A4;
    songPicked[0][108].length = ETRIPLET;
    songPicked[0][109].note = C5;
    songPicked[0][109].length = ETRIPLET;
    songPicked[0][110].note = REST;
    songPicked[0][110].length = EIGHTH;
    songPicked[0][111].note = A4;
    songPicked[0][111].length = ETRIPLET;
    songPicked[0][112].note = C5;
    songPicked[0][112].length = ETRIPLET;
    songPicked[0][113].note = D5;
    songPicked[0][113].length = ETRIPLET;

    //MEASURE 13
    songPicked[0][114].note = REST;
    songPicked[0][114].length = QUARTER;
    songPicked[0][115].note = G5;
    songPicked[0][115].length = EIGHTH;
    songPicked[0][116].note = Gb5;
    songPicked[0][116].length = EIGHTH;
    songPicked[0][117].note = F5;
    songPicked[0][117].length = EIGHTH;
    songPicked[0][118].note = Eb5;
    songPicked[0][118].length = QUARTER;
    songPicked[0][119].note = E5;
    songPicked[0][119].length = EIGHTH;

    //MEASURE 14
    songPicked[0][120].note = REST;
    songPicked[0][120].length = EIGHTH;
    songPicked[0][121].note = C6;
    songPicked[0][121].length = EIGHTH;
    songPicked[0][122].note = REST;
    songPicked[0][122].length = EIGHTH;
    songPicked[0][123].note = C6;
    songPicked[0][123].length = EIGHTH;
    songPicked[0][124].note = C6;
    songPicked[0][124].length = HALF;

    //MEASURE 15
    songPicked[0][125].note = REST;
    songPicked[0][125].length = QUARTER;
    songPicked[0][126].note = G5;
    songPicked[0][126].length = EIGHTH;
    songPicked[0][127].note = Gb5;
    songPicked[0][127].length = EIGHTH;
    songPicked[0][128].note = F5;
    songPicked[0][128].length = EIGHTH;
    songPicked[0][129].note = Eb5;
    songPicked[0][129].length = QUARTER;
    songPicked[0][130].note = E5;
    songPicked[0][130].length = EIGHTH;

    //MEASURE 16
    songPicked[0][131].note = REST;
    songPicked[0][131].length = EIGHTH;
    songPicked[0][132].note = Ab4;
    songPicked[0][132].length = ETRIPLET;
    songPicked[0][133].note = A4;
    songPicked[0][133].length = ETRIPLET;
    songPicked[0][134].note = C5;
    songPicked[0][134].length = ETRIPLET;
    songPicked[0][135].note = REST;
    songPicked[0][135].length = EIGHTH;
    songPicked[0][136].note = A4;
    songPicked[0][136].length = ETRIPLET;
    songPicked[0][137].note = C5;
    songPicked[0][137].length = ETRIPLET;
    songPicked[0][138].note = D5;
    songPicked[0][138].length = ETRIPLET;

    //MEASURE 17
    songPicked[0][139].note = REST;
    songPicked[0][139].length = QUARTER;
    songPicked[0][140].note = Eb5;
    songPicked[0][140].length = QUARTER;
    songPicked[0][141].note = REST;
    songPicked[0][141].length = EIGHTH;
    songPicked[0][142].note = D5;
    songPicked[0][142].length = QUARTER;
    songPicked[0][143].note = REST;
    songPicked[0][143].length = EIGHTH;

    //MEASURE 18
    songPicked[0][144].note = C5;
    songPicked[0][144].length = HALF;
    songPicked[0][145].note = REST;
    songPicked[0][145].length = HALF;

    //END REPEAT 1
    //MEASURE 19
    songPicked[0][146].note = C5;
    songPicked[0][146].length = EIGHTH;
    songPicked[0][147].note = C5;
    songPicked[0][147].length = QUARTER;
    songPicked[0][148].note = C5;
    songPicked[0][148].length = EIGHTH;
    songPicked[0][149].note = REST;
    songPicked[0][149].length = EIGHTH;
    songPicked[0][150].note = C5;
    songPicked[0][150].length = EIGHTH;
    songPicked[0][151].note = D5;
    songPicked[0][151].length = QUARTER;

    //MEASURE 20
    songPicked[0][152].note = E5;
    songPicked[0][152].length = EIGHTH;
    songPicked[0][153].note = C5;
    songPicked[0][153].length = QUARTER;
    songPicked[0][154].note = A4;
    songPicked[0][154].length = EIGHTH;
    songPicked[0][155].note = G4;
    songPicked[0][155].length = HALF;

    //MEASURE 21
    songPicked[0][156].note = C5;
    songPicked[0][156].length = EIGHTH;
    songPicked[0][157].note = C5;
    songPicked[0][157].length = QUARTER;
    songPicked[0][158].note = C5;
    songPicked[0][158].length = EIGHTH;
    songPicked[0][159].note = REST;
    songPicked[0][159].length = EIGHTH;
    songPicked[0][160].note = C5;
    songPicked[0][160].length = ETRIPLET;
    songPicked[0][161].note = D5;
    songPicked[0][161].length = ETRIPLET;
    songPicked[0][162].note = E5;
    songPicked[0][162].length = ETRIPLET;

    //MEASURE 22
    songPicked[0][163].note = REST;
    songPicked[0][163].length = WHOLE;

    //MEASURE 23
    songPicked[0][164].note = C5;
    songPicked[0][164].length = EIGHTH;
    songPicked[0][165].note = C5;
    songPicked[0][165].length = QUARTER;
    songPicked[0][166].note = C5;
    songPicked[0][166].length = EIGHTH;
    songPicked[0][167].note = REST;
    songPicked[0][167].length = EIGHTH;
    songPicked[0][168].note = C5;
    songPicked[0][168].length = EIGHTH;
    songPicked[0][169].note = D5;
    songPicked[0][169].length = QUARTER;

    //MEASURE 24
    songPicked[0][170].note = E5;
    songPicked[0][170].length = EIGHTH;
    songPicked[0][171].note = C5;
    songPicked[0][171].length = QUARTER;
    songPicked[0][172].note = A4;
    songPicked[0][172].length = EIGHTH;
    songPicked[0][173].note = G4;
    songPicked[0][173].length = HALF;

    //MEASURE 25
    songPicked[0][174].note = E5;
    songPicked[0][174].length = EIGHTH;
    songPicked[0][175].note = E5;
    songPicked[0][175].length = EIGHTH;
    songPicked[0][176].note = REST;
    songPicked[0][176].length = EIGHTH;
    songPicked[0][177].note = E5;
    songPicked[0][177].length = EIGHTH;
    songPicked[0][178].note = REST;
    songPicked[0][178].length = EIGHTH;
    songPicked[0][179].note = C5;
    songPicked[0][179].length = EIGHTH;
    songPicked[0][180].note = E5;
    songPicked[0][180].length = QUARTER;

    //MEASURE 26
    songPicked[0][181].note = G5;
    songPicked[0][181].length = QUARTER;
    songPicked[0][182].note = REST;
    songPicked[0][182].length = QUARTER;
    songPicked[0][183].note = G4;
    songPicked[0][183].length = QUARTER;
    songPicked[0][184].note = REST;
    songPicked[0][184].length = QUARTER;

    //MEASURE 27
    songPicked[0][185].note = C5;
    songPicked[0][185].length = QUARTER * DOTTED;
    songPicked[0][186].note = G4;
    songPicked[0][186].length = EIGHTH;
    songPicked[0][187].note = REST;
    songPicked[0][187].length = QUARTER;
    songPicked[0][188].note = E4;
    songPicked[0][188].length = QUARTER;
    songPicked[0][188].slur = 1;

    //MEASURE 28
    songPicked[0][189].note = E4;
    songPicked[0][189].length = EIGHTH;
    songPicked[0][190].note = A4;
    songPicked[0][190].length = QUARTER;
    songPicked[0][191].note = B4;
    songPicked[0][191].length = EIGHTH;
    songPicked[0][192].note = B4;
    songPicked[0][192].length = EIGHTH;
    songPicked[0][193].note = Bb4;
    songPicked[0][193].length = EIGHTH;
    songPicked[0][194].note = A4;
    songPicked[0][194].length = QUARTER;

    //MEASURE 29
    songPicked[0][195].note = G4;
    songPicked[0][195].length = QTRIPLET;
    songPicked[0][196].note = E5;
    songPicked[0][196].length = QTRIPLET;
    songPicked[0][197].note = G5;
    songPicked[0][197].length = QTRIPLET;
    songPicked[0][198].note = A5;
    songPicked[0][198].length = QUARTER;
    songPicked[0][199].note = F5;
    songPicked[0][199].length = EIGHTH;
    songPicked[0][200].note = G5;
    songPicked[0][200].length = EIGHTH;

    //MEASURE 30
    songPicked[0][201].note = REST;
    songPicked[0][201].length = EIGHTH;
    songPicked[0][202].note = E5;
    songPicked[0][202].length = QUARTER;
    songPicked[0][203].note = C5;
    songPicked[0][203].length = EIGHTH;
    songPicked[0][204].note = D5;
    songPicked[0][204].length = EIGHTH;
    songPicked[0][205].note = B4;
    songPicked[0][205].length = QUARTER;
    songPicked[0][206].note = REST;
    songPicked[0][206].length = EIGHTH;

    //MEASURE 31
    songPicked[0][207].note = C5;
    songPicked[0][207].length = QUARTER * DOTTED;
    songPicked[0][208].note = G4;
    songPicked[0][208].length = EIGHTH;
    songPicked[0][209].note = REST;
    songPicked[0][209].length = QUARTER;
    songPicked[0][210].note = E4;
    songPicked[0][210].length = QUARTER;
    songPicked[0][210].slur = 1;

    //MEASURE 32
    songPicked[0][211].note = E4;
    songPicked[0][211].length = EIGHTH;
    songPicked[0][212].note = A4;
    songPicked[0][212].length = QUARTER;
    songPicked[0][213].note = B4;
    songPicked[0][213].length = QUARTER;
    songPicked[0][214].note = Bb4;
    songPicked[0][214].length = EIGHTH;
    songPicked[0][215].note = A4;
    songPicked[0][215].length = QUARTER;

    //MEASURE 33
    songPicked[0][216].note = G4;
    songPicked[0][216].length = QTRIPLET;
    songPicked[0][217].note = E5;
    songPicked[0][217].length = QTRIPLET;
    songPicked[0][218].note = G5;
    songPicked[0][218].length = QTRIPLET;
    songPicked[0][219].note = A5;
    songPicked[0][219].length = QUARTER;
    songPicked[0][220].note = F5;
    songPicked[0][220].length = EIGHTH;
    songPicked[0][221].note = G5;
    songPicked[0][221].length = EIGHTH;

    //MEASURE 34
    songPicked[0][222].note = REST;
    songPicked[0][222].length = EIGHTH;
    songPicked[0][223].note = E5;
    songPicked[0][223].length = QUARTER;
    songPicked[0][224].note = C5;
    songPicked[0][224].length = EIGHTH;
    songPicked[0][225].note = D5;
    songPicked[0][225].length = EIGHTH;
    songPicked[0][226].note = B4;
    songPicked[0][226].length = QUARTER;
    songPicked[0][227].note = REST;
    songPicked[0][227].length = EIGHTH;

    //REAPEAT 2
    //MEASURE 35
    songPicked[0][228].note = E5;
    songPicked[0][228].length = EIGHTH;
    songPicked[0][229].note = C5;
    songPicked[0][229].length = QUARTER;
    songPicked[0][230].note = G4;
    songPicked[0][230].length = EIGHTH;
    songPicked[0][231].note = REST;
    songPicked[0][231].length = QUARTER;
    songPicked[0][232].note = Ab4;
    songPicked[0][232].length = QUARTER;

    //MEASURE 36
    songPicked[0][233].note = A4;
    songPicked[0][233].length = EIGHTH;
    songPicked[0][234].note = F5;
    songPicked[0][234].length = QUARTER;
    songPicked[0][235].note = F5;
    songPicked[0][235].length = EIGHTH;
    songPicked[0][236].note = A4;
    songPicked[0][236].length = HALF;

    //MWASURE 37
    songPicked[0][237].note = B4;
    songPicked[0][237].length = QTRIPLET;
    songPicked[0][238].note = A5;
    songPicked[0][238].length = QTRIPLET;
    songPicked[0][239].note = A5;
    songPicked[0][239].length = QTRIPLET;
    songPicked[0][240].note = A5;
    songPicked[0][240].length = QTRIPLET;
    songPicked[0][241].note = G5;
    songPicked[0][241].length = QTRIPLET;
    songPicked[0][242].note = F5;
    songPicked[0][242].length = QTRIPLET;

    //MEASURE 38
    songPicked[0][243].note = E5;
    songPicked[0][243].length = EIGHTH;
    songPicked[0][244].note = C5;
    songPicked[0][244].length = QUARTER;
    songPicked[0][245].note = A4;
    songPicked[0][245].length = EIGHTH;
    songPicked[0][246].note = G4;
    songPicked[0][246].length = HALF;

    //MEASURE 39
    songPicked[0][247].note = E5;
    songPicked[0][247].length = EIGHTH;
    songPicked[0][248].note = C5;
    songPicked[0][248].length = QUARTER;
    songPicked[0][249].note = G4;
    songPicked[0][249].length = EIGHTH;
    songPicked[0][250].note = REST;
    songPicked[0][250].length = QUARTER;
    songPicked[0][251].note = Ab4;
    songPicked[0][251].length = QUARTER;

    //MEASURE 40
    songPicked[0][252].note = A4;
    songPicked[0][252].length = EIGHTH;
    songPicked[0][253].note = F5;
    songPicked[0][253].length = QUARTER;
    songPicked[0][254].note = F5;
    songPicked[0][254].length = EIGHTH;
    songPicked[0][255].note = A4;
    songPicked[0][255].length = HALF;

    //MEASURE 41
    songPicked[0][256].note = B4;
    songPicked[0][256].length = QTRIPLET;
    songPicked[0][257].note = F5;
    songPicked[0][257].length = QTRIPLET;
    songPicked[0][258].note = F5;
    songPicked[0][258].length = QTRIPLET;
    songPicked[0][259].note = F5;
    songPicked[0][259].length = QTRIPLET;
    songPicked[0][260].note = E5;
    songPicked[0][260].length = QTRIPLET;
    songPicked[0][261].note = D5;
    songPicked[0][261].length = QTRIPLET;

    //MEASURE 42
    songPicked[0][262].note = C5;
    songPicked[0][262].length = HALF;
    songPicked[0][263].note = C4;
    songPicked[0][263].length = HALF;

    //REPEAT STARTS HERE
    //MEASURE 35
    songPicked[0][264].note = E5;
    songPicked[0][264].length = EIGHTH;
    songPicked[0][265].note = C5;
    songPicked[0][265].length = QUARTER;
    songPicked[0][266].note = G4;
    songPicked[0][266].length = EIGHTH;
    songPicked[0][267].note = REST;
    songPicked[0][267].length = QUARTER;
    songPicked[0][268].note = Ab4;
    songPicked[0][268].length = QUARTER;

    //MEASURE 36
    songPicked[0][269].note = A4;
    songPicked[0][269].length = EIGHTH;
    songPicked[0][270].note = F5;
    songPicked[0][270].length = QUARTER;
    songPicked[0][271].note = F5;
    songPicked[0][271].length = EIGHTH;
    songPicked[0][272].note = A4;
    songPicked[0][272].length = HALF;

    //MWASURE 37
    songPicked[0][273].note = B4;
    songPicked[0][273].length = QTRIPLET;
    songPicked[0][274].note = A5;
    songPicked[0][274].length = QTRIPLET;
    songPicked[0][275].note = A5;
    songPicked[0][275].length = QTRIPLET;
    songPicked[0][276].note = A5;
    songPicked[0][276].length = QTRIPLET;
    songPicked[0][277].note = G5;
    songPicked[0][277].length = QTRIPLET;
    songPicked[0][278].note = F5;
    songPicked[0][278].length = QTRIPLET;

    //MEASURE 38
    songPicked[0][279].note = E5;
    songPicked[0][279].length = EIGHTH;
    songPicked[0][280].note = C5;
    songPicked[0][280].length = QUARTER;
    songPicked[0][281].note = A4;
    songPicked[0][281].length = EIGHTH;
    songPicked[0][282].note = G4;
    songPicked[0][282].length = HALF;

    //MEASURE 39
    songPicked[0][283].note = E5;
    songPicked[0][283].length = EIGHTH;
    songPicked[0][284].note = C5;
    songPicked[0][284].length = QUARTER;
    songPicked[0][285].note = G4;
    songPicked[0][285].length = EIGHTH;
    songPicked[0][286].note = REST;
    songPicked[0][286].length = QUARTER;
    songPicked[0][287].note = Ab4;
    songPicked[0][287].length = QUARTER;

    //MEASURE 40
    songPicked[0][288].note = A4;
    songPicked[0][288].length = EIGHTH;
    songPicked[0][289].note = F5;
    songPicked[0][289].length = QUARTER;
    songPicked[0][290].note = F5;
    songPicked[0][290].length = EIGHTH;
    songPicked[0][291].note = A4;
    songPicked[0][291].length = HALF;

    //MEASURE 41
    songPicked[0][292].note = B4;
    songPicked[0][292].length = QTRIPLET;
    songPicked[0][293].note = F5;
    songPicked[0][293].length = QTRIPLET;
    songPicked[0][294].note = F5;
    songPicked[0][294].length = QTRIPLET;
    songPicked[0][295].note = F5;
    songPicked[0][295].length = QTRIPLET;
    songPicked[0][296].note = E5;
    songPicked[0][296].length = QTRIPLET;
    songPicked[0][297].note = D5;
    songPicked[0][297].length = QTRIPLET;

    //MEASURE 42
    songPicked[0][298].note = C5;
    songPicked[0][298].length = HALF;
    songPicked[0][299].note = C4;
    songPicked[0][299].length = HALF;

    //END REPEAT 2
    //MEASURE 43
    songPicked[0][300].note = C5;
    songPicked[0][300].length = EIGHTH;
    songPicked[0][301].note = C5;
    songPicked[0][301].length = QUARTER;
    songPicked[0][302].note = C5;
    songPicked[0][302].length = EIGHTH;
    songPicked[0][303].note = REST;
    songPicked[0][303].length = EIGHTH;
    songPicked[0][304].note = C5;
    songPicked[0][304].length = EIGHTH;
    songPicked[0][305].note = D5;
    songPicked[0][305].length = QUARTER;

    //MEASURE 44
    songPicked[0][306].note = E5;
    songPicked[0][306].length = EIGHTH;
    songPicked[0][307].note = C5;
    songPicked[0][307].length = QUARTER;
    songPicked[0][308].note = A4;
    songPicked[0][308].length = EIGHTH;
    songPicked[0][309].note = G4;
    songPicked[0][309].length = HALF;

    //MEASURE 45
    songPicked[0][310].note = C5;
    songPicked[0][310].length = EIGHTH;
    songPicked[0][311].note = C5;
    songPicked[0][311].length = QUARTER;
    songPicked[0][312].note = C5;
    songPicked[0][312].length = EIGHTH;
    songPicked[0][313].note = REST;
    songPicked[0][313].length = EIGHTH;
    songPicked[0][314].note = C5;
    songPicked[0][314].length = ETRIPLET;
    songPicked[0][315].note = D5;
    songPicked[0][315].length = ETRIPLET;
    songPicked[0][316].note = E5;
    songPicked[0][316].length = ETRIPLET;

    //MEASURE 46
    songPicked[0][317].note = REST;
    songPicked[0][317].length = WHOLE;

    //MEASURE 47
    songPicked[0][318].note = C5;
    songPicked[0][318].length = EIGHTH;
    songPicked[0][319].note = C5;
    songPicked[0][319].length = QUARTER;
    songPicked[0][320].note = C5;
    songPicked[0][320].length = EIGHTH;
    songPicked[0][321].note = REST;
    songPicked[0][321].length = EIGHTH;
    songPicked[0][322].note = C5;
    songPicked[0][322].length = EIGHTH;
    songPicked[0][323].note = D5;
    songPicked[0][323].length = QUARTER;

    //MEASURE 48
    songPicked[0][324].note = E5;
    songPicked[0][324].length = EIGHTH;
    songPicked[0][325].note = C5;
    songPicked[0][325].length = QUARTER;
    songPicked[0][326].note = A4;
    songPicked[0][326].length = EIGHTH;
    songPicked[0][327].note = G4;
    songPicked[0][327].length = HALF;

    //MEASURE 49
    songPicked[0][328].note = E5;
    songPicked[0][328].length = EIGHTH;
    songPicked[0][329].note = E5;
    songPicked[0][329].length = EIGHTH;
    songPicked[0][330].note = REST;
    songPicked[0][330].length = EIGHTH;
    songPicked[0][331].note = E5;
    songPicked[0][331].length = EIGHTH;
    songPicked[0][332].note = REST;
    songPicked[0][332].length = EIGHTH;
    songPicked[0][333].note = C5;
    songPicked[0][333].length = EIGHTH;
    songPicked[0][334].note = E5;
    songPicked[0][334].length = QUARTER;

    //MEASURE 50
    songPicked[0][335].note = G5;
    songPicked[0][335].length = QUARTER;
    songPicked[0][336].note = REST;
    songPicked[0][336].length = QUARTER;
    songPicked[0][337].note = G4;
    songPicked[0][337].length = QUARTER;
    songPicked[0][338].note = REST;
    songPicked[0][338].length = QUARTER;

    //MEASURE 51
    songPicked[0][339].note = E5;
    songPicked[0][339].length = EIGHTH;
    songPicked[0][340].note = C5;
    songPicked[0][340].length = QUARTER;
    songPicked[0][341].note = G4;
    songPicked[0][341].length = EIGHTH;
    songPicked[0][342].note = REST;
    songPicked[0][342].length = QUARTER;
    songPicked[0][343].note = Ab4;
    songPicked[0][343].length = QUARTER;

    //MEASURE 52
    songPicked[0][344].note = A4;
    songPicked[0][344].length = EIGHTH;
    songPicked[0][345].note = F5;
    songPicked[0][345].length = QUARTER;
    songPicked[0][346].note = F5;
    songPicked[0][346].length = EIGHTH;
    songPicked[0][347].note = A4;
    songPicked[0][347].length = HALF;

    //MEASURE 53
    songPicked[0][348].note = B4;
    songPicked[0][348].length = QTRIPLET;
    songPicked[0][349].note = A5;
    songPicked[0][349].length = QTRIPLET;
    songPicked[0][350].note = A5;
    songPicked[0][350].length = QTRIPLET;
    songPicked[0][351].note = A5;
    songPicked[0][351].length = QTRIPLET;
    songPicked[0][352].note = G5;
    songPicked[0][352].length = QTRIPLET;
    songPicked[0][353].note = F5;
    songPicked[0][353].length = QTRIPLET;

    //MEASURE 54
    songPicked[0][354].note = E5;
    songPicked[0][354].length = EIGHTH;
    songPicked[0][355].note = C5;
    songPicked[0][355].length = QUARTER;
    songPicked[0][356].note = A4;
    songPicked[0][356].length = EIGHTH;
    songPicked[0][357].note = G4;
    songPicked[0][357].length = HALF;

    //MEASURE 55
    songPicked[0][358].note = E5;
    songPicked[0][358].length = EIGHTH;
    songPicked[0][359].note = C5;
    songPicked[0][359].length = QUARTER;
    songPicked[0][360].note = G4;
    songPicked[0][360].length = EIGHTH;
    songPicked[0][361].note = REST;
    songPicked[0][361].length = QUARTER;
    songPicked[0][362].note = Ab4;
    songPicked[0][362].length = QUARTER;

    //MEASURE 56
    songPicked[0][363].note = A4;
    songPicked[0][363].length = EIGHTH;
    songPicked[0][364].note = F5;
    songPicked[0][364].length = QUARTER;
    songPicked[0][365].note = F5;
    songPicked[0][365].length = EIGHTH;
    songPicked[0][366].note = A4;
    songPicked[0][366].length = HALF;

    //MEASURE 57
    songPicked[0][367].note = B4;
    songPicked[0][367].length = QTRIPLET;
    songPicked[0][368].note = F5;
    songPicked[0][368].length = QTRIPLET;
    songPicked[0][369].note = F5;
    songPicked[0][369].length = QTRIPLET;
    songPicked[0][370].note = F5;
    songPicked[0][370].length = QTRIPLET;
    songPicked[0][371].note = E5;
    songPicked[0][371].length = QTRIPLET;
    songPicked[0][372].note = D5;
    songPicked[0][372].length = QTRIPLET;

    //MEASURE 58
    songPicked[0][373].note = C5;
    songPicked[0][373].length = HALF;
    songPicked[0][374].note = C4;
    songPicked[0][374].length = HALF;

    //MEASURE 59
    songPicked[0][375].note = C5;
    songPicked[0][375].length = QUARTER * DOTTED;
    songPicked[0][376].note = G4;
    songPicked[0][376].length = EIGHTH;
    songPicked[0][376].slur = 1;
    songPicked[0][377].note = G4;
    songPicked[0][377].length = QUARTER;
    songPicked[0][378].note = F4;
    songPicked[0][378].length = QUARTER;

    //MEASURE 60
    songPicked[0][379].note = A4;
    songPicked[0][379].length = QTRIPLET;
    songPicked[0][380].note = B4;
    songPicked[0][380].length = QTRIPLET;
    songPicked[0][381].note = A4;
    songPicked[0][381].length = QTRIPLET;
    songPicked[0][382].note = Ab4;
    songPicked[0][382].length = QTRIPLET;
    songPicked[0][383].note = Bb4;
    songPicked[0][383].length = QTRIPLET;
    songPicked[0][384].note = A4;
    songPicked[0][384].length = QTRIPLET;

    //MEASURE 61
    songPicked[0][385].note = G4;
    songPicked[0][385].length = WHOLE;

    //END
    songPicked[0][386].note = REST;
    songPicked[0][386].length = WHOLE;
    songPicked[0][387].note = REST;
    songPicked[0][387].length = WHOLE;
    songPicked[0][388].note = REST;
    songPicked[0][388].length = WHOLE;

    songPicked[0][388].end = 1;
}

void initTetrisTheme()
{
    //MEASURE 1
    songPicked[2][0].note = E6;
    songPicked[2][0].length = QUARTER;
    songPicked[2][1].note = B5;
    songPicked[2][1].length = EIGHTH;
    songPicked[2][2].note = C6;
    songPicked[2][2].length = EIGHTH;
    songPicked[2][3].note = D6;
    songPicked[2][3].length = QUARTER;
    songPicked[2][4].note = C6;
    songPicked[2][4].length = EIGHTH;
    songPicked[2][5].note = B5;
    songPicked[2][5].length = EIGHTH;

    //MEASURE 2
    songPicked[2][6].note = A5;
    songPicked[2][6].length = QUARTER;
    songPicked[2][7].note = A5;
    songPicked[2][7].length = EIGHTH;
    songPicked[2][8].note = C6;
    songPicked[2][8].length = EIGHTH;
    songPicked[2][9].note = E6;
    songPicked[2][9].length = QUARTER;
    songPicked[2][10].note = D6;
    songPicked[2][10].length = EIGHTH;
    songPicked[2][11].note = C6;
    songPicked[2][11].length = EIGHTH;

    //MEASURE 3
    songPicked[2][12].note = B5;
    songPicked[2][12].length = QUARTER;
    songPicked[2][13].note = B5;
    songPicked[2][13].length = EIGHTH;
    songPicked[2][14].note = C6;
    songPicked[2][14].length = EIGHTH;
    songPicked[2][15].note = D6;
    songPicked[2][15].length = QUARTER;
    songPicked[2][16].note = E6;
    songPicked[2][16].length = QUARTER;

    //MEASURE 4
    songPicked[2][17].note = C6;
    songPicked[2][17].length = QUARTER;
    songPicked[2][18].note = A5;
    songPicked[2][18].length = QUARTER;
    songPicked[2][19].note = A5;
    songPicked[2][19].length = QUARTER;
    songPicked[2][20].note = A5;
    songPicked[2][20].length = QUARTER;

    //MEASURE 5
    songPicked[2][21].note = REST;
    songPicked[2][21].length = EIGHTH;
    songPicked[2][22].note = D6;
    songPicked[2][22].length = QUARTER;
    songPicked[2][23].note = F6;
    songPicked[2][23].length = EIGHTH;
    songPicked[2][24].note = A6;
    songPicked[2][24].length = QUARTER;
    songPicked[2][25].note = G6;
    songPicked[2][25].length = EIGHTH;
    songPicked[2][26].note = F6;
    songPicked[2][26].length = EIGHTH;

    //MEASURE 6
    songPicked[2][27].note = E6;
    songPicked[2][27].length = QUARTER * DOTTED;
    songPicked[2][28].note = C6;
    songPicked[2][28].length = EIGHTH;
    songPicked[2][29].note = E6;
    songPicked[2][29].length = QUARTER;
    songPicked[2][30].note = D6;
    songPicked[2][30].length = EIGHTH;
    songPicked[2][31].note = C6;
    songPicked[2][31].length = EIGHTH;

    //MEASURE 7
    songPicked[2][32].note = B5;
    songPicked[2][32].length = EIGHTH;
    songPicked[2][33].note = REST;
    songPicked[2][33].length = EIGHTH;
    songPicked[2][34].note = B5;
    songPicked[2][34].length = EIGHTH;
    songPicked[2][35].note = C6;
    songPicked[2][35].length = EIGHTH;
    songPicked[2][36].note = D6;
    songPicked[2][36].length = QUARTER;
    songPicked[2][37].note = E6;
    songPicked[2][37].length = QUARTER;

    //MEASURE 8
    songPicked[2][38].note = C6;
    songPicked[2][38].length = EIGHTH;
    songPicked[2][39].note = A5;
    songPicked[2][39].length = EIGHTH;
    songPicked[2][40].note = E5;
    songPicked[2][40].length = EIGHTH;
    songPicked[2][41].note = C5;
    songPicked[2][41].length = EIGHTH;
    songPicked[2][42].note = B4;
    songPicked[2][42].length = EIGHTH;
    songPicked[2][43].note = F4;
    songPicked[2][43].length = EIGHTH;
    songPicked[2][44].note = D4;
    songPicked[2][44].length = EIGHTH;
    songPicked[2][45].note = B3;
    songPicked[2][45].length = EIGHTH;

    //MEASURE 9
    songPicked[2][46].note = E5;
    songPicked[2][46].length = QUARTER;
    songPicked[2][47].note = B4;
    songPicked[2][47].length = EIGHTH;
    songPicked[2][48].note = C5;
    songPicked[2][48].length = EIGHTH;
    songPicked[2][49].note = D5;
    songPicked[2][49].length = QUARTER;
    songPicked[2][50].note = C5;
    songPicked[2][50].length = EIGHTH;
    songPicked[2][51].note = B4;
    songPicked[2][51].length = EIGHTH;

    //MEASURE 10
    songPicked[2][52].note = A4;
    songPicked[2][52].length = QUARTER;
    songPicked[2][53].note = A4;
    songPicked[2][53].length = EIGHTH;
    songPicked[2][54].note = C5;
    songPicked[2][54].length = EIGHTH;
    songPicked[2][55].note = E5;
    songPicked[2][55].length = QUARTER;
    songPicked[2][56].note = D5;
    songPicked[2][56].length = EIGHTH;
    songPicked[2][57].note = C5;
    songPicked[2][57].length = EIGHTH;

    //MEASURE 11
    songPicked[2][58].note = B4;
    songPicked[2][58].length = QUARTER * DOTTED;
    songPicked[2][59].note = C5;
    songPicked[2][59].length = EIGHTH;
    songPicked[2][60].note = D5;
    songPicked[2][60].length = QUARTER;
    songPicked[2][61].note = E5;
    songPicked[2][61].length = QUARTER;

    //MEASURE 12
    songPicked[2][62].note = C5;
    songPicked[2][62].length = QUARTER;
    songPicked[2][63].note = A4;
    songPicked[2][63].length = QUARTER;
    songPicked[2][64].note = A4;
    songPicked[2][64].length = HALF;

    //MEASURE 13
    songPicked[2][65].note = REST;
    songPicked[2][65].length = EIGHTH;
    songPicked[2][66].note = D5;
    songPicked[2][66].length = QUARTER;
    songPicked[2][67].note = F5;
    songPicked[2][67].length = EIGHTH;
    songPicked[2][68].note = A5;
    songPicked[2][68].length = QUARTER;
    songPicked[2][69].note = G5;
    songPicked[2][69].length = EIGHTH;
    songPicked[2][70].note = F5;
    songPicked[2][70].length = EIGHTH;

    //MEASURE 14
    songPicked[2][71].note = E5;
    songPicked[2][71].length = QUARTER * DOTTED;
    songPicked[2][72].note = C5;
    songPicked[2][72].length = EIGHTH;
    songPicked[2][73].note = E5;
    songPicked[2][73].length = QUARTER;
    songPicked[2][74].note = D5;
    songPicked[2][74].length = EIGHTH;
    songPicked[2][75].note = C5;
    songPicked[2][75].length = EIGHTH;

    //MEASURE 15
    songPicked[2][76].note = B4;
    songPicked[2][76].length = QUARTER * DOTTED;
    songPicked[2][77].note = C5;
    songPicked[2][77].length = EIGHTH;
    songPicked[2][78].note = D5;
    songPicked[2][78].length = QUARTER;
    songPicked[2][79].note = E5;
    songPicked[2][79].length = QUARTER;

    //MEASURE 16
    songPicked[2][80].note = C5;
    songPicked[2][80].length = QUARTER;
    songPicked[2][81].note = A4;
    songPicked[2][81].length = QUARTER;
    songPicked[2][82].note = A4;
    songPicked[2][82].length = EIGHTH;
    songPicked[2][83].note = REST;
    songPicked[2][83].length = EIGHTH;
    songPicked[2][84].note = REST;
    songPicked[2][84].length = QUARTER;

    //MEASURE 17
    songPicked[2][85].note = REST;
    songPicked[2][85].length = WHOLE;

    //MEASURE 18
    songPicked[2][86].note = REST;
    songPicked[2][86].length = WHOLE;

    //END
    songPicked[2][86].end     = 1;
}

void initPokemonTheme()
{
    //MEASURE 1
    songPicked[3][0].note = D4;
    songPicked[3][0].length = EIGHTH;
    songPicked[3][1].note = D4;
    songPicked[3][1].length = EIGHTH;
    songPicked[3][2].note = A4;
    songPicked[3][2].length = QUARTER;
    songPicked[3][3].note = D4;
    songPicked[3][3].length = EIGHTH;
    songPicked[3][4].note = D4;
    songPicked[3][4].length = EIGHTH;
    songPicked[3][5].note = Bb4;
    songPicked[3][5].length = QUARTER;

    //MEASURE 2
    songPicked[3][6].note = D4;
    songPicked[3][6].length = EIGHTH;
    songPicked[3][7].note = D4;
    songPicked[3][7].length = EIGHTH;
    songPicked[3][8].note = A4;
    songPicked[3][8].length = QUARTER;
    songPicked[3][9].note = D4;
    songPicked[3][9].length = EIGHTH;
    songPicked[3][10].note = D4;
    songPicked[3][10].length = EIGHTH;
    songPicked[3][11].note = Fs4;
    songPicked[3][11].length = QUARTER;

    //MEASURE 3
    songPicked[3][12].note = D4;
    songPicked[3][12].length = EIGHTH;
    songPicked[3][13].note = D4;
    songPicked[3][13].length = EIGHTH;
    songPicked[3][14].note = A4;
    songPicked[3][14].length = QUARTER;
    songPicked[3][15].note = D4;
    songPicked[3][15].length = EIGHTH;
    songPicked[3][16].note = D4;
    songPicked[3][16].length = EIGHTH;
    songPicked[3][17].note = Cs5;
    songPicked[3][17].length = QUARTER;

    //MEASURE 4
    songPicked[3][18].note = D5;
    songPicked[3][18].length = HALF;
    songPicked[3][19].note = A3;
    songPicked[3][19].length = HALF;

    //MEASURE 5
    songPicked[3][20].note = C5;
    songPicked[3][20].length = HALF;
    songPicked[3][21].note = C4;
    songPicked[3][21].length = HALF;

    //MEASURE 6
    songPicked[3][22].note = D4;
    songPicked[3][22].length = EIGHTH;
    songPicked[3][23].note = D4;
    songPicked[3][23].length = EIGHTH;
    songPicked[3][24].note = A4;
    songPicked[3][24].length = QUARTER;
    songPicked[3][25].note = D4;
    songPicked[3][25].length = EIGHTH;
    songPicked[3][26].note = D4;
    songPicked[3][26].length = EIGHTH;
    songPicked[3][27].note = Bb4;
    songPicked[3][27].length = QUARTER;

    //MEASURE 7
    songPicked[3][28].note = D4;
    songPicked[3][28].length = EIGHTH;
    songPicked[3][29].note = D4;
    songPicked[3][29].length = EIGHTH;
    songPicked[3][30].note = A4;
    songPicked[3][30].length = QUARTER;
    songPicked[3][31].note = D4;
    songPicked[3][31].length = EIGHTH;
    songPicked[3][32].note = D4;
    songPicked[3][32].length = EIGHTH;
    songPicked[3][33].note = Cs5;
    songPicked[3][33].length = QUARTER;

    //MEASURE 8
    songPicked[3][34].note = D5;
    songPicked[3][34].length = WHOLE;

    //MEASURE 9
    //CHANGE BPM TO 120
    songPicked[3][35].note = D6;
    songPicked[3][35].length = HALF;
    songPicked[3][36].note = REST;
    songPicked[3][36].length = QUARTER;
    songPicked[3][37].note = G3;
    songPicked[3][37].length = SIXTEENTH;
    songPicked[3][38].note = B3;
    songPicked[3][38].length = SIXTEENTH;
    songPicked[3][39].note = D4;
    songPicked[3][39].length = SIXTEENTH;
    songPicked[3][40].note = Fs4;
    songPicked[3][40].length = SIXTEENTH;

    //MEASURE 10
    songPicked[3][41].note = G4;
    songPicked[3][41].length = QUARTER;
    songPicked[3][41].accent = 1;
    songPicked[3][42].note = G4;
    songPicked[3][42].length = QUARTER * DOTTED;
    songPicked[3][42].accent = 1;
    songPicked[3][43].note = G4;
    songPicked[3][43].length = SIXTEENTH;
    songPicked[3][43].accent = 1;
    songPicked[3][44].note = G4;
    songPicked[3][44].length = SIXTEENTH;
    songPicked[3][44].accent = 1;
    songPicked[3][45].note = G4;
    songPicked[3][45].length = QUARTER;
    songPicked[3][45].accent = 1;

    //MEASURE 11
    songPicked[3][46].note = G4;
    songPicked[3][46].length = QUARTER;
    songPicked[3][46].accent = 1;
    songPicked[3][47].note = G4;
    songPicked[3][47].length = QUARTER;
    songPicked[3][47].accent = 1;
    songPicked[3][48].note = F4;
    songPicked[3][48].length = ETRIPLET;
    songPicked[3][48].accent = 1;
    songPicked[3][49].note = F4;
    songPicked[3][49].length = ETRIPLET;
    songPicked[3][49].accent = 1;
    songPicked[3][50].note = F4;
    songPicked[3][50].length = ETRIPLET;
    songPicked[3][50].accent = 1;
    songPicked[3][51].note = F4;
    songPicked[3][51].length = ETRIPLET;
    songPicked[3][51].accent = 1;
    songPicked[3][52].note = F4;
    songPicked[3][52].length = ETRIPLET;
    songPicked[3][52].accent = 1;
    songPicked[3][53].note = Fs4;
    songPicked[3][53].length = ETRIPLET;
    songPicked[3][53].accent = 1;

    //MEASURE 12
    songPicked[3][54].note = G4;
    songPicked[3][54].length = QUARTER * DOTTED;
    songPicked[3][55].note = B4;
    songPicked[3][55].length = EIGHTH;
    songPicked[3][56].note = D5;
    songPicked[3][56].length = HALF;

    //MEASURE 13
    songPicked[3][57].note = A4;
    songPicked[3][57].length = QUARTER * DOTTED;
    songPicked[3][58].note = A4;
    songPicked[3][58].length = EIGHTH;
    songPicked[3][59].note = F5;
    songPicked[3][59].length = QUARTER * DOTTED;
    songPicked[3][60].note = E5;
    songPicked[3][60].length = SIXTEENTH;
    songPicked[3][61].note = Eb5;
    songPicked[3][61].length = SIXTEENTH;

    //MEASURE 14
    songPicked[3][62].note = D5;
    songPicked[3][62].length = HALF;
    songPicked[3][63].note = F4;
    songPicked[3][63].length = QUARTER * DOTTED;
    songPicked[3][64].note = E4;
    songPicked[3][64].length = SIXTEENTH;
    songPicked[3][65].note = Eb4;
    songPicked[3][65].length = SIXTEENTH;

    //MEASURE 15
    songPicked[3][66].note = D4;
    songPicked[3][66].length = HALF;
    songPicked[3][67].note = C4;
    songPicked[3][67].length = QTRIPLET;
    songPicked[3][68].note = B3;
    songPicked[3][68].length = QTRIPLET;
    songPicked[3][69].note = C4;
    songPicked[3][69].length = QTRIPLET;

    //MEASURE 16
    songPicked[3][70].note = G4;
    songPicked[3][70].length = QUARTER * DOTTED;
    songPicked[3][71].note = B4;
    songPicked[3][71].length = EIGHTH;
    songPicked[3][72].note = D5;
    songPicked[3][72].length = HALF;

    //MEASURE 17
    songPicked[3][73].note = A4;
    songPicked[3][73].length = HALF;
    songPicked[3][74].note = C5;
    songPicked[3][74].length = QTRIPLET;
    songPicked[3][75].note = A4;
    songPicked[3][75].length = QTRIPLET;
    songPicked[3][76].note = C5;
    songPicked[3][76].length = QTRIPLET;

    //MEASURE 18
    songPicked[3][77].note = D5;
    songPicked[3][77].length = HALF;
    songPicked[3][78].note = F4;
    songPicked[3][78].length = QTRIPLET;
    songPicked[3][79].note = E4;
    songPicked[3][79].length = QTRIPLET;
    songPicked[3][80].note = C4;
    songPicked[3][80].length = QTRIPLET;

    //MEASURE 19
    songPicked[3][81].note = D4;
    songPicked[3][81].length = HALF;
    songPicked[3][82].note = REST;
    songPicked[3][82].length = EIGHTH;
    songPicked[3][83].note = B3;
    songPicked[3][83].length = ETRIPLET;
    songPicked[3][84].note = C4;
    songPicked[3][84].length = ETRIPLET;
    songPicked[3][85].note = D4;
    songPicked[3][85].length = ETRIPLET;

    //MEASURE 20
    songPicked[3][86].note = G4;
    songPicked[3][86].length = QUARTER * DOTTED;
    songPicked[3][87].note = B4;
    songPicked[3][87].length = EIGHTH;
    songPicked[3][88].note = D5;
    songPicked[3][88].length = HALF;

    //MEASURE 21
    songPicked[3][89].note = A4;
    songPicked[3][89].length = QUARTER * DOTTED;
    songPicked[3][90].note = A4;
    songPicked[3][90].length = EIGHTH;
    songPicked[3][91].note = F5;
    songPicked[3][91].length = QUARTER * DOTTED;
    songPicked[3][92].note = E5;
    songPicked[3][92].length = SIXTEENTH;
    songPicked[3][93].note = Eb5;
    songPicked[3][93].length = SIXTEENTH;

    //MEASURE 22
    songPicked[3][94].note = D5;
    songPicked[3][94].length = HALF;
    songPicked[3][95].note = F4;
    songPicked[3][95].length = QUARTER * DOTTED;
    songPicked[3][96].note = E4;
    songPicked[3][96].length = SIXTEENTH;
    songPicked[3][97].note = Eb4;
    songPicked[3][97].length = SIXTEENTH;

    //MEASURE 23
    songPicked[3][98].note = D4;
    songPicked[3][98].length = HALF;
    songPicked[3][99].note = C4;
    songPicked[3][99].length = QTRIPLET;
    songPicked[3][100].note = B3;
    songPicked[3][100].length = QTRIPLET;
    songPicked[3][101].note = C4;
    songPicked[3][101].length = QTRIPLET;

    //MEASURE 24
    songPicked[3][102].note = G4;
    songPicked[3][102].length = QUARTER * DOTTED;
    songPicked[3][103].note = B4;
    songPicked[3][103].length = EIGHTH;
    songPicked[3][104].note = D5;
    songPicked[3][104].length = HALF;

    //MEASURE 25
    songPicked[3][105].note = A4;
    songPicked[3][105].length = HALF;
    songPicked[3][106].note = F5;
    songPicked[3][106].length = QTRIPLET;
    songPicked[3][107].note = E5;
    songPicked[3][107].length = QTRIPLET;
    songPicked[3][108].note = F5;
    songPicked[3][108].length = QTRIPLET;

    //MEASURE 26
    songPicked[3][109].note = G5;
    songPicked[3][109].length = QUARTER * DOTTED;
    songPicked[3][110].note = Bb5;
    songPicked[3][110].length = EIGHTH;
    songPicked[3][111].note = G5;
    songPicked[3][111].length = HALF;
    songPicked[3][111].slur = 1;

    //MEASURE 27
    songPicked[3][112].note = G5;
    songPicked[3][112].length = HALF;
    songPicked[3][113].note = A5;
    songPicked[3][113].length = HALF;

    //MEASURE 28
    songPicked[3][114].note = Bb5;
    songPicked[3][114].length = QUARTER * DOTTED;
    songPicked[3][115].note = F5;
    songPicked[3][115].length = EIGHTH;
    songPicked[3][116].note = F5;
    songPicked[3][116].length = HALF;

    //MEASURE 29
    songPicked[3][117].note = F4;
    songPicked[3][117].length = HALF;
    songPicked[3][118].note = Bb5;
    songPicked[3][118].length = QUARTER;
    songPicked[3][119].note = B5;
    songPicked[3][119].length = QUARTER;

    //MEASURE 30
    songPicked[3][120].note = C6;
    songPicked[3][120].length = QUARTER * DOTTED;
    songPicked[3][121].note = G5;
    songPicked[3][121].length = EIGHTH;
    songPicked[3][122].note = G5;
    songPicked[3][122].length = HALF;

    //MEASURE 31
    songPicked[3][123].note = G4;
    songPicked[3][123].length = HALF;
    songPicked[3][124].note = C6;
    songPicked[3][124].length = QUARTER;
    songPicked[3][125].note = Cs6;
    songPicked[3][125].length = QUARTER;

    //MEASURE 32
    songPicked[3][126].note = D6;
    songPicked[3][126].length = EIGHTH;
    songPicked[3][127].note = D5;
    songPicked[3][127].length = QUARTER;
    songPicked[3][128].note = D5;
    songPicked[3][128].length = QUARTER;
    songPicked[3][129].note = D5;
    songPicked[3][129].length = QUARTER;
    songPicked[3][130].note = D5;
    songPicked[3][130].length = SIXTEENTH;
    songPicked[3][131].note = D5;
    songPicked[3][131].length = SIXTEENTH;

    //MEASURE 33
    songPicked[3][132].note = D5;
    songPicked[3][132].length = EIGHTH;
    songPicked[3][133].note = D5;
    songPicked[3][133].length = QUARTER;
    songPicked[3][134].note = D5;
    songPicked[3][134].length = EIGHTH;
    songPicked[3][135].note = C6;
    songPicked[3][135].length = QTRIPLET;
    songPicked[3][136].note = C6;
    songPicked[3][136].length = QTRIPLET;
    songPicked[3][137].note = Cs6;
    songPicked[3][137].length = QTRIPLET;

    //MEASURE 34
    songPicked[3][138].note = D6;
    songPicked[3][138].length = EIGHTH;
    songPicked[3][139].note = D5;
    songPicked[3][139].length = QUARTER;
    songPicked[3][140].note = D5;
    songPicked[3][140].length = QUARTER;
    songPicked[3][141].note = D5;
    songPicked[3][141].length = QUARTER;
    songPicked[3][142].note = D5;
    songPicked[3][142].length = SIXTEENTH;
    songPicked[3][143].note = D5;
    songPicked[3][143].length = SIXTEENTH;

    //MEASURE 35
    songPicked[3][144].note = D5;
    songPicked[3][144].length = EIGHTH;
    songPicked[3][145].note = D5;
    songPicked[3][145].length = QUARTER;
    songPicked[3][146].note = D5;
    songPicked[3][146].length = EIGHTH;
    songPicked[3][147].note = C6;
    songPicked[3][147].length = QTRIPLET;
    songPicked[3][148].note = C6;
    songPicked[3][148].length = QTRIPLET;
    songPicked[3][149].note = B5;
    songPicked[3][149].length = QTRIPLET;

    //MEASURE 36
    songPicked[3][150].note = G5;
    songPicked[3][150].length = WHOLE;

    //END
    songPicked[3][151].note = REST;
    songPicked[3][151].length = WHOLE;
    songPicked[3][152].note = REST;
    songPicked[3][152].length = WHOLE;
    songPicked[3][152].end = 1;
}

void initAlarm1() //Shadow of the Colossus
{
    //6/8 Time Signature
    //105bpm
    //MEASURE 1
    songPicked[4][0].note = D4;
    songPicked[4][0].length = SIXTEENTH;
    songPicked[4][1].note = Bb3;
    songPicked[4][1].length = SIXTEENTH;
    songPicked[4][2].note = E4;
    songPicked[4][2].length = SIXTEENTH;
    songPicked[4][3].note = Bb3;
    songPicked[4][3].length = SIXTEENTH;
    songPicked[4][4].note = F4;
    songPicked[4][4].length = SIXTEENTH;
    songPicked[4][5].note = Bb3;
    songPicked[4][5].length = SIXTEENTH;
    songPicked[4][6].note = G4;
    songPicked[4][6].length = SIXTEENTH;
    songPicked[4][7].note = Bb3;
    songPicked[4][7].length = SIXTEENTH;
    songPicked[4][8].note = F4;
    songPicked[4][8].length = SIXTEENTH;
    songPicked[4][9].note = Bb3;
    songPicked[4][9].length = SIXTEENTH;
    songPicked[4][10].note = E4;
    songPicked[4][10].length = SIXTEENTH;
    songPicked[4][11].note = Bb3;
    songPicked[4][11].length = SIXTEENTH;

    //MEASURE 2
    songPicked[4][12].note = D4;
    songPicked[4][12].length = SIXTEENTH;
    songPicked[4][13].note = Bb3;
    songPicked[4][13].length = SIXTEENTH;
    songPicked[4][14].note = E4;
    songPicked[4][14].length = SIXTEENTH;
    songPicked[4][15].note = Bb3;
    songPicked[4][15].length = SIXTEENTH;
    songPicked[4][16].note = F4;
    songPicked[4][16].length = SIXTEENTH;
    songPicked[4][17].note = Bb3;
    songPicked[4][17].length = SIXTEENTH;
    songPicked[4][18].note = G4;
    songPicked[4][18].length = SIXTEENTH;
    songPicked[4][19].note = Bb3;
    songPicked[4][19].length = SIXTEENTH;
    songPicked[4][20].note = F4;
    songPicked[4][20].length = SIXTEENTH;
    songPicked[4][21].note = Bb3;
    songPicked[4][21].length = SIXTEENTH;
    songPicked[4][22].note = E4;
    songPicked[4][22].length = SIXTEENTH;
    songPicked[4][23].note = Bb3;
    songPicked[4][23].length = SIXTEENTH;

    //MEASURE 3
    songPicked[4][24].note = D4;
    songPicked[4][24].length = SIXTEENTH;
    songPicked[4][25].note = Bb3;
    songPicked[4][25].length = SIXTEENTH;
    songPicked[4][26].note = E4;
    songPicked[4][26].length = SIXTEENTH;
    songPicked[4][27].note = Bb3;
    songPicked[4][27].length = SIXTEENTH;
    songPicked[4][28].note = F4;
    songPicked[4][28].length = SIXTEENTH;
    songPicked[4][29].note = Bb3;
    songPicked[4][29].length = SIXTEENTH;
    songPicked[4][30].note = G4;
    songPicked[4][30].length = SIXTEENTH;
    songPicked[4][31].note = Bb3;
    songPicked[4][31].length = SIXTEENTH;
    songPicked[4][32].note = F4;
    songPicked[4][32].length = SIXTEENTH;
    songPicked[4][33].note = Bb3;
    songPicked[4][33].length = SIXTEENTH;
    songPicked[4][34].note = E4;
    songPicked[4][34].length = SIXTEENTH;
    songPicked[4][35].note = Bb3;
    songPicked[4][35].length = SIXTEENTH;

    //MEASURE 4
    songPicked[4][36].note = A4;
    songPicked[4][36].length = SIXTEENTH;
    songPicked[4][37].note = Bb3;
    songPicked[4][37].length = SIXTEENTH;
    songPicked[4][38].note = G4;
    songPicked[4][38].length = SIXTEENTH;
    songPicked[4][39].note = Bb3;
    songPicked[4][39].length = SIXTEENTH;
    songPicked[4][40].note = F4;
    songPicked[4][40].length = SIXTEENTH;
    songPicked[4][41].note = Bb3;
    songPicked[4][41].length = SIXTEENTH;
    songPicked[4][42].note = E4;
    songPicked[4][42].length = SIXTEENTH;
    songPicked[4][43].note = Bb3;
    songPicked[4][43].length = SIXTEENTH;
    songPicked[4][44].note = F4;
    songPicked[4][44].length = SIXTEENTH;
    songPicked[4][45].note = Bb3;
    songPicked[4][45].length = SIXTEENTH;
    songPicked[4][46].note = G4;
    songPicked[4][46].length = SIXTEENTH;
    songPicked[4][47].note = Bb3;
    songPicked[4][47].length = SIXTEENTH;


    //MEASURE 5
    songPicked[4][48].note = D4;
    songPicked[4][48].length = SIXTEENTH;
    songPicked[4][49].note = Bb3;
    songPicked[4][49].length = SIXTEENTH;
    songPicked[4][50].note = E4;
    songPicked[4][50].length = SIXTEENTH;
    songPicked[4][51].note = Bb3;
    songPicked[4][51].length = SIXTEENTH;
    songPicked[4][52].note = F4;
    songPicked[4][52].length = SIXTEENTH;
    songPicked[4][53].note = Bb3;
    songPicked[4][53].length = SIXTEENTH;
    songPicked[4][54].note = G4;
    songPicked[4][54].length = SIXTEENTH;
    songPicked[4][55].note = Bb3;
    songPicked[4][55].length = SIXTEENTH;
    songPicked[4][56].note = F4;
    songPicked[4][56].length = SIXTEENTH;
    songPicked[4][57].note = Bb3;
    songPicked[4][57].length = SIXTEENTH;
    songPicked[4][58].note = E4;
    songPicked[4][58].length = SIXTEENTH;
    songPicked[4][59].note = Bb3;
    songPicked[4][59].length = SIXTEENTH;

    //MEASURE 6
    songPicked[4][60].note = D4;
    songPicked[4][60].length = SIXTEENTH;
    songPicked[4][61].note = Bb3;
    songPicked[4][61].length = SIXTEENTH;
    songPicked[4][62].note = E4;
    songPicked[4][62].length = SIXTEENTH;
    songPicked[4][63].note = Bb3;
    songPicked[4][63].length = SIXTEENTH;
    songPicked[4][64].note = F4;
    songPicked[4][64].length = SIXTEENTH;
    songPicked[4][65].note = Bb3;
    songPicked[4][65].length = SIXTEENTH;
    songPicked[4][66].note = G4;
    songPicked[4][66].length = SIXTEENTH;
    songPicked[4][67].note = Bb3;
    songPicked[4][67].length = SIXTEENTH;
    songPicked[4][68].note = F4;
    songPicked[4][68].length = SIXTEENTH;
    songPicked[4][69].note = Bb3;
    songPicked[4][69].length = SIXTEENTH;
    songPicked[4][70].note = E4;
    songPicked[4][70].length = SIXTEENTH;
    songPicked[4][71].note = Bb3;
    songPicked[4][71].length = SIXTEENTH;

    //MEASURE 7
    songPicked[4][72].note = D4;
    songPicked[4][72].length = SIXTEENTH;
    songPicked[4][73].note = Bb3;
    songPicked[4][73].length = SIXTEENTH;
    songPicked[4][74].note = E4;
    songPicked[4][74].length = SIXTEENTH;
    songPicked[4][75].note = Bb3;
    songPicked[4][75].length = SIXTEENTH;
    songPicked[4][76].note = F4;
    songPicked[4][76].length = SIXTEENTH;
    songPicked[4][77].note = Bb3;
    songPicked[4][77].length = SIXTEENTH;
    songPicked[4][78].note = G4;
    songPicked[4][78].length = SIXTEENTH;
    songPicked[4][79].note = Bb3;
    songPicked[4][79].length = SIXTEENTH;
    songPicked[4][80].note = F4;
    songPicked[4][80].length = SIXTEENTH;
    songPicked[4][81].note = Bb3;
    songPicked[4][81].length = SIXTEENTH;
    songPicked[4][82].note = E4;
    songPicked[4][82].length = SIXTEENTH;
    songPicked[4][83].note = Bb3;
    songPicked[4][83].length = SIXTEENTH;

    //MEASURE 8
    songPicked[4][84].note = A4;
    songPicked[4][84].length = SIXTEENTH;
    songPicked[4][85].note = Bb3;
    songPicked[4][85].length = SIXTEENTH;
    songPicked[4][86].note = G4;
    songPicked[4][86].length = SIXTEENTH;
    songPicked[4][87].note = Bb3;
    songPicked[4][87].length = SIXTEENTH;
    songPicked[4][88].note = F4;
    songPicked[4][88].length = SIXTEENTH;
    songPicked[4][89].note = Bb3;
    songPicked[4][89].length = SIXTEENTH;
    songPicked[4][90].note = E4;
    songPicked[4][90].length = SIXTEENTH;
    songPicked[4][91].note = Bb3;
    songPicked[4][91].length = SIXTEENTH;
    songPicked[4][92].note = F4;
    songPicked[4][92].length = SIXTEENTH;
    songPicked[4][93].note = Bb3;
    songPicked[4][93].length = SIXTEENTH;
    songPicked[4][94].note = G4;
    songPicked[4][94].length = SIXTEENTH;
    songPicked[4][95].note = Bb3;
    songPicked[4][95].length = SIXTEENTH;

    //REPEAT
    //MEASURE 9
    songPicked[4][96].note = F4;
    songPicked[4][96].length = EIGHTH * DOTTED;
    songPicked[4][96].slur = 1;
    songPicked[4][97].note = A4;
    songPicked[4][97].length = EIGHTH * DOTTED;
    songPicked[4][98].note = E4;
    songPicked[4][98].length = EIGHTH * DOTTED;
    songPicked[4][98].slur = 1;
    songPicked[4][99].note = G4;
    songPicked[4][99].length = EIGHTH * DOTTED;

    //MEASURE 10
    songPicked[4][100].note = F4;
    songPicked[4][100].length = EIGHTH * DOTTED;
    songPicked[4][100].slur = 1;
    songPicked[4][101].note = C5;
    songPicked[4][101].length = EIGHTH * DOTTED;
    songPicked[4][102].note = Bb4;
    songPicked[4][102].length = EIGHTH;
    songPicked[4][103].note = A4;
    songPicked[4][103].length = SIXTEENTH;
    songPicked[4][104].note = G4;
    songPicked[4][104].length = SIXTEENTH;
    songPicked[4][105].note = A4;
    songPicked[4][105].length = EIGHTH;

    //MEASURE 11
    songPicked[4][106].note = F4;
    songPicked[4][106].length = EIGHTH * DOTTED;
    songPicked[4][106].slur = 1;
    songPicked[4][107].note = A4;
    songPicked[4][107].length = EIGHTH * DOTTED;
    songPicked[4][108].note = E4;
    songPicked[4][108].length = EIGHTH * DOTTED;
    songPicked[4][108].slur = 1;
    songPicked[4][109].note = G4;
    songPicked[4][109].length = EIGHTH * DOTTED;

    //MEASURE 12
    songPicked[4][110].note = F4;
    songPicked[4][110].length = EIGHTH * DOTTED;
    songPicked[4][110].slur = 1;
    songPicked[4][111].note = C5;
    songPicked[4][111].length = EIGHTH * DOTTED;
    songPicked[4][112].note = Bb4;
    songPicked[4][112].length = EIGHTH;
    songPicked[4][113].note = A4;
    songPicked[4][113].length = SIXTEENTH;
    songPicked[4][114].note = G4;
    songPicked[4][114].length = SIXTEENTH;
    songPicked[4][115].note = A4;
    songPicked[4][115].length = EIGHTH;

    //MEASURE 13
    songPicked[4][116].note = E4;
    songPicked[4][116].length = EIGHTH * DOTTED;
    songPicked[4][117].note = F4;
    songPicked[4][117].length = SIXTEENTH;
    songPicked[4][118].note = E4;
    songPicked[4][118].length = EIGHTH;
    songPicked[4][119].note = A4;
    songPicked[4][119].length = QUARTER * DOTTED;

    //MEASURE 14
    songPicked[4][120].note = E4;
    songPicked[4][120].length = EIGHTH * DOTTED;
    songPicked[4][121].note = F4;
    songPicked[4][121].length = SIXTEENTH;
    songPicked[4][122].note = E4;
    songPicked[4][122].length = EIGHTH;
    songPicked[4][123].note = Gs4;
    songPicked[4][123].length = QUARTER * DOTTED;

    //MEASURE 15
    songPicked[4][124].note = E4;
    songPicked[4][124].length = EIGHTH * DOTTED;
    songPicked[4][125].note = F4;
    songPicked[4][125].length = SIXTEENTH;
    songPicked[4][126].note = E4;
    songPicked[4][126].length = EIGHTH;
    songPicked[4][127].note = A4;
    songPicked[4][127].length = QUARTER * DOTTED;

    //MEASURE 16
    songPicked[4][128].note = C5;
    songPicked[4][128].length = EIGHTH;
    songPicked[4][129].note = Bb4;
    songPicked[4][129].length = SIXTEENTH;
    songPicked[4][130].note = A4;
    songPicked[4][130].length = SIXTEENTH;
    songPicked[4][131].note = G4;
    songPicked[4][131].length = SIXTEENTH;
    songPicked[4][132].note = F4;
    songPicked[4][132].length = SIXTEENTH;
    songPicked[4][133].note = E4;
    songPicked[4][133].length = EIGHTH;
    songPicked[4][134].note = F4;
    songPicked[4][134].length = EIGHTH;
    songPicked[4][135].note = Ds4;
    songPicked[4][135].length = EIGHTH;

    //REPEAT STARTS
    //MEASURE 9
    songPicked[4][136].note = F4;
    songPicked[4][136].length = EIGHTH * DOTTED;
    songPicked[4][136].slur = 1;
    songPicked[4][137].note = A4;
    songPicked[4][137].length = EIGHTH * DOTTED;
    songPicked[4][138].note = E4;
    songPicked[4][138].length = EIGHTH * DOTTED;
    songPicked[4][138].slur = 1;
    songPicked[4][139].note = G4;
    songPicked[4][139].length = EIGHTH * DOTTED;

    //MEASURE 10
    songPicked[4][140].note = F4;
    songPicked[4][140].length = EIGHTH * DOTTED;
    songPicked[4][140].slur = 1;
    songPicked[4][141].note = C5;
    songPicked[4][141].length = EIGHTH * DOTTED;
    songPicked[4][142].note = Bb4;
    songPicked[4][142].length = EIGHTH;
    songPicked[4][143].note = A4;
    songPicked[4][143].length = SIXTEENTH;
    songPicked[4][144].note = G4;
    songPicked[4][144].length = SIXTEENTH;
    songPicked[4][145].note = A4;
    songPicked[4][145].length = EIGHTH;

    //MEASURE 11
    songPicked[4][146].note = F4;
    songPicked[4][146].length = EIGHTH * DOTTED;
    songPicked[4][146].slur = 1;
    songPicked[4][147].note = A4;
    songPicked[4][147].length = EIGHTH * DOTTED;
    songPicked[4][148].note = E4;
    songPicked[4][148].length = EIGHTH * DOTTED;
    songPicked[4][148].slur = 1;
    songPicked[4][149].note = G4;
    songPicked[4][149].length = EIGHTH * DOTTED;

    //MEASURE 12
    songPicked[4][150].note = F4;
    songPicked[4][150].length = EIGHTH * DOTTED;
    songPicked[4][150].slur = 1;
    songPicked[4][151].note = C5;
    songPicked[4][151].length = EIGHTH * DOTTED;
    songPicked[4][152].note = Bb4;
    songPicked[4][152].length = EIGHTH;
    songPicked[4][153].note = A4;
    songPicked[4][153].length = SIXTEENTH;
    songPicked[4][154].note = G4;
    songPicked[4][154].length = SIXTEENTH;
    songPicked[4][155].note = A4;
    songPicked[4][155].length = EIGHTH;

    //MEASURE 13
    songPicked[4][156].note = E4;
    songPicked[4][156].length = EIGHTH * DOTTED;
    songPicked[4][157].note = F4;
    songPicked[4][157].length = SIXTEENTH;
    songPicked[4][158].note = E4;
    songPicked[4][158].length = EIGHTH;
    songPicked[4][159].note = A4;
    songPicked[4][159].length = QUARTER * DOTTED;

    //MEASURE 14
    songPicked[4][160].note = E4;
    songPicked[4][160].length = EIGHTH * DOTTED;
    songPicked[4][161].note = F4;
    songPicked[4][161].length = SIXTEENTH;
    songPicked[4][162].note = E4;
    songPicked[4][162].length = EIGHTH;
    songPicked[4][163].note = Gs4;
    songPicked[4][163].length = QUARTER * DOTTED;

    //MEASURE 15
    songPicked[4][164].note = E4;
    songPicked[4][164].length = EIGHTH * DOTTED;
    songPicked[4][165].note = F4;
    songPicked[4][165].length = SIXTEENTH;
    songPicked[4][166].note = E4;
    songPicked[4][166].length = EIGHTH;
    songPicked[4][167].note = A4;
    songPicked[4][167].length = QUARTER * DOTTED;

    //MEASURE 16
    songPicked[4][168].note = C5;
    songPicked[4][168].length = EIGHTH;
    songPicked[4][169].note = Bb4;
    songPicked[4][169].length = SIXTEENTH;
    songPicked[4][170].note = A4;
    songPicked[4][170].length = SIXTEENTH;
    songPicked[4][171].note = G4;
    songPicked[4][171].length = SIXTEENTH;
    songPicked[4][172].note = F4;
    songPicked[4][172].length = SIXTEENTH;
    songPicked[4][173].note = E4;
    songPicked[4][173].length = EIGHTH;
    songPicked[4][174].note = F4;
    songPicked[4][174].length = EIGHTH;
    songPicked[4][175].note = Ds4;
    songPicked[4][175].length = EIGHTH;

    //MEASURE 17
    songPicked[4][176].note = REST;
    songPicked[4][176].length = HALF * DOTTED;

    //REPEAT ENDS
    //END
    songPicked[4][176].end = 1;
}

void initAlarm2()
{
    //240 bpm
    //3/4 Time Signature

    //MEASURE 1
    songPicked[5][0].note = Bb3;
    songPicked[5][0].length = EIGHTH;
    songPicked[5][0].accent = 1;
    songPicked[5][1].note = Db4;
    songPicked[5][1].length = EIGHTH;
    songPicked[5][1].accent = 1;
    songPicked[5][2].note = C4;
    songPicked[5][2].length = EIGHTH;
    songPicked[5][2].accent = 1;
    songPicked[5][3].note = Db4;
    songPicked[5][3].length = EIGHTH;
    songPicked[5][3].accent = 1;
    songPicked[5][4].note = Bb3;
    songPicked[5][4].length = QUARTER;
    songPicked[5][4].accent = 1;

    //MEASURE 2
    songPicked[5][5].note = Bb3;
    songPicked[5][5].length = EIGHTH;
    songPicked[5][5].accent = 1;
    songPicked[5][6].note = C4;
    songPicked[5][6].length = EIGHTH;
    songPicked[5][6].accent = 1;
    songPicked[5][7].note = Db4;
    songPicked[5][7].length = EIGHTH;
    songPicked[5][7].accent = 1;
    songPicked[5][8].note = Ab4;
    songPicked[5][8].length = EIGHTH;
    songPicked[5][8].accent = 1;
    songPicked[5][9].note = F4;
    songPicked[5][9].length = QUARTER;
    songPicked[5][9].accent = 1;

    //MEASURE 3
    songPicked[5][10].note = Bb3;
    songPicked[5][10].length = EIGHTH;
    songPicked[5][10].accent = 1;
    songPicked[5][11].note = Db4;
    songPicked[5][11].length = EIGHTH;
    songPicked[5][11].accent = 1;
    songPicked[5][12].note = C4;
    songPicked[5][12].length = EIGHTH;
    songPicked[5][12].accent = 1;
    songPicked[5][13].note = Db4;
    songPicked[5][13].length = EIGHTH;
    songPicked[5][13].accent = 1;
    songPicked[5][14].note = Bb3;
    songPicked[5][14].length = QUARTER;
    songPicked[5][14].accent = 1;

    //MEASURE 4
    songPicked[5][15].note = Bb3;
    songPicked[5][15].length = EIGHTH;
    songPicked[5][15].accent = 1;
    songPicked[5][16].note = C4;
    songPicked[5][16].length = EIGHTH;
    songPicked[5][16].accent = 1;
    songPicked[5][17].note = Db4;
    songPicked[5][17].length = EIGHTH;
    songPicked[5][17].accent = 1;
    songPicked[5][18].note = Ab4;
    songPicked[5][18].length = EIGHTH;
    songPicked[5][18].accent = 1;
    songPicked[5][19].note = F4;
    songPicked[5][19].length = QUARTER;
    songPicked[5][19].accent = 1;

    //MEASURE 5
    songPicked[5][20].note = Bb3;
    songPicked[5][20].length = EIGHTH;
    songPicked[5][20].accent = 1;
    songPicked[5][21].note = Db4;
    songPicked[5][21].length = EIGHTH;
    songPicked[5][21].accent = 1;
    songPicked[5][22].note = C4;
    songPicked[5][22].length = EIGHTH;
    songPicked[5][22].accent = 1;
    songPicked[5][23].note = Db4;
    songPicked[5][23].length = EIGHTH;
    songPicked[5][23].accent = 1;
    songPicked[5][24].note = Bb3;
    songPicked[5][24].length = QUARTER;
    songPicked[5][24].accent = 1;

    //MEASURE 6
    songPicked[5][25].note = Bb3;
    songPicked[5][25].length = EIGHTH;
    songPicked[5][25].accent = 1;
    songPicked[5][26].note = C4;
    songPicked[5][26].length = EIGHTH;
    songPicked[5][26].accent = 1;
    songPicked[5][27].note = Db4;
    songPicked[5][27].length = EIGHTH;
    songPicked[5][27].accent = 1;
    songPicked[5][28].note = Ab4;
    songPicked[5][28].length = EIGHTH;
    songPicked[5][28].accent = 1;
    songPicked[5][29].note = F4;
    songPicked[5][29].length = QUARTER;
    songPicked[5][29].accent = 1;

    //MEASURE 7
    songPicked[5][30].note = Eb4;
    songPicked[5][30].length = QUARTER;
    songPicked[5][31].note = C5;
    songPicked[5][31].length = QUARTER;
    songPicked[5][32].note = Bb4;
    songPicked[5][32].length = QUARTER;

    //MEASURE 8
    songPicked[5][33].note = Ab4;
    songPicked[5][33].length = EIGHTH;
    songPicked[5][34].note = Gb4;
    songPicked[5][34].length = EIGHTH;
    songPicked[5][35].note = F4;
    songPicked[5][35].length = QUARTER;
    songPicked[5][36].note = Ab4;
    songPicked[5][36].length = QUARTER;

    //MEASURE 9
    songPicked[5][37].note = Bb3;
    songPicked[5][37].length = EIGHTH;
    songPicked[5][37].accent = 1;
    songPicked[5][38].note = Db4;
    songPicked[5][38].length = EIGHTH;
    songPicked[5][38].accent = 1;
    songPicked[5][39].note = C4;
    songPicked[5][39].length = EIGHTH;
    songPicked[5][39].accent = 1;
    songPicked[5][40].note = Db4;
    songPicked[5][40].length = EIGHTH;
    songPicked[5][40].accent = 1;
    songPicked[5][41].note = Bb3;
    songPicked[5][41].length = QUARTER;
    songPicked[5][41].accent = 1;

    //MEASURE 10
    songPicked[5][42].note = Bb3;
    songPicked[5][42].length = EIGHTH;
    songPicked[5][42].accent = 1;
    songPicked[5][43].note = C4;
    songPicked[5][43].length = EIGHTH;
    songPicked[5][43].accent = 1;
    songPicked[5][44].note = Db4;
    songPicked[5][44].length = EIGHTH;
    songPicked[5][44].accent = 1;
    songPicked[5][45].note = Ab4;
    songPicked[5][45].length = EIGHTH;
    songPicked[5][45].accent = 1;
    songPicked[5][46].note = F4;
    songPicked[5][46].length = QUARTER;
    songPicked[5][46].accent = 1;

    //MEASURE 11
    songPicked[5][47].note = Bb3;
    songPicked[5][47].length = EIGHTH;
    songPicked[5][47].accent = 1;
    songPicked[5][48].note = Db4;
    songPicked[5][48].length = EIGHTH;
    songPicked[5][48].accent = 1;
    songPicked[5][49].note = C4;
    songPicked[5][49].length = EIGHTH;
    songPicked[5][49].accent = 1;
    songPicked[5][50].note = Db4;
    songPicked[5][50].length = EIGHTH;
    songPicked[5][50].accent = 1;
    songPicked[5][51].note = Bb3;
    songPicked[5][51].length = QUARTER;
    songPicked[5][51].accent = 1;

    //MEASURE 12
    songPicked[5][52].note = Bb3;
    songPicked[5][52].length = EIGHTH;
    songPicked[5][52].accent = 1;
    songPicked[5][53].note = C4;
    songPicked[5][53].length = EIGHTH;
    songPicked[5][53].accent = 1;
    songPicked[5][54].note = Db4;
    songPicked[5][54].length = EIGHTH;
    songPicked[5][54].accent = 1;
    songPicked[5][55].note = Ab4;
    songPicked[5][55].length = EIGHTH;
    songPicked[5][55].accent = 1;
    songPicked[5][56].note = F4;
    songPicked[5][56].length = QUARTER;
    songPicked[5][56].accent = 1;

    //MEASURE 13
    songPicked[5][57].note = Db5;
    songPicked[5][57].length = HALF * DOTTED;
    songPicked[5][57].slur = 1;

    //MEASURE 14
    songPicked[5][58].note = Db5;
    songPicked[5][58].length = QUARTER;
    songPicked[5][59].note = Db5;
    songPicked[5][59].length = QUARTER;
    songPicked[5][60].note = Db5;
    songPicked[5][60].length = QUARTER;

    //MEASURE 15
    songPicked[5][61].note = Eb5;
    songPicked[5][61].length = QUARTER;
    songPicked[5][62].note = C6;
    songPicked[5][62].length = QUARTER;
    songPicked[5][63].note = Bb5;
    songPicked[5][63].length = QUARTER;

    //MEASURE 16
    songPicked[5][64].note = Ab5;
    songPicked[5][64].length = EIGHTH;
    songPicked[5][65].note = Gb5;
    songPicked[5][65].length = EIGHTH;
    songPicked[5][66].note = F5;
    songPicked[5][66].length = QUARTER;
    songPicked[5][67].note = Ab5;
    songPicked[5][67].length = QUARTER;

    //MEASURE 17
    songPicked[5][68].note = Bb5;
    songPicked[5][68].length = HALF * DOTTED;
    songPicked[5][68].slur = 1;

    //MEASURE 18
    songPicked[5][69].note = Bb5;
    songPicked[5][69].length = QUARTER;
    songPicked[5][70].note = F5;
    songPicked[5][70].length = QUARTER;
    songPicked[5][71].note = Ab5;
    songPicked[5][71].length = QUARTER;

    //MEASURE 19
    songPicked[5][72].note = Bb5;
    songPicked[5][72].length = HALF;
    songPicked[5][73].note = Db6;
    songPicked[5][73].length = QUARTER;

    //MEASURE 20
    songPicked[5][74].note = Eb6;
    songPicked[5][74].length = EIGHTH;
    songPicked[5][75].note = Db6;
    songPicked[5][75].length = EIGHTH;
    songPicked[5][76].note = C6;
    songPicked[5][76].length = QUARTER;
    songPicked[5][77].note = Db6;
    songPicked[5][77].length = QUARTER;

    //MEASURE 21
    songPicked[5][78].note = Bb5;
    songPicked[5][78].length = HALF * DOTTED;
    songPicked[5][78].slur = 1;

    //MEASURE 22
    songPicked[5][79].note = Bb5;
    songPicked[5][79].length = HALF * DOTTED;
    songPicked[5][78].slur = 1;

    //MEASURE 23
    songPicked[5][80].note = REST;
    songPicked[5][80].length = HALF * DOTTED;
    songPicked[5][80].slur = 1;

    //END
    songPicked[5][80].end = 1;
}

void endSong()
{
    stopSong = 1;
}



