/*        File: Timer A
 *      Author: Adan Pantoja
 * Description: Contains different functions that can be used with Timer A.
 */

#include "msp.h"

#define TA0_PORT P2
#define TA1_PORT P7
#define TACLR BIT2


/* --- Hall Effect Sensor macros --- */
#define HE_PIN BIT4 // H.E. sensor pin (TA0.1)

#define ACLK 32000     // Speed of ACLK
#define DIAMETER 1.951 // Diameter of magnet holder (in inches)
#define FPH_MPH 5280   // Conversion from feet-per-hour to miles-per-hour
#define pi 3.141592654

/* --- Ultrasonic Sensor macros --- */
#define USS_TRIG_PIN BIT7 // TA1.1
#define USS_ECHO_PIN BIT6 // TA1.2



/* ---- Function Prototypes ---- */
void calculate_RPM(void);
void calculate_MPH(void);


/* --- Global Variables --- */

// Hall Effect Sensor
volatile int   Period = 0;  // Holds the period between magnets
volatile float timePerCycle = 1.0 / ACLK; // Timer per 1 cycle @ 32kHz clk speed
volatile float HC_time = 0; // Time it takes for half rotation
volatile float FC_time = 0; // Time it takes for a full rotation
volatile float Freq = 0;    // Frequency of motor
extern volatile float RPM = 0; // Revolutions-per-minute of motor

volatile float diameter_feet = 0; // Convert diameter from inches to feet
volatile float circum = 0;  // Circumference of magnet holder
volatile float feetPerMin = 0;
volatile float feetPerHour = 0;
volatile int   MPH = 0; // Miles-per-hour

uint8_t STARTUP_FLAG = 1; // Used as a start-up flag

// Ultrasonic sensor
volatile unsigned int currentedge;

/* ----------------------------------------------------------
 * Function: Initializes Timer A so it can be used with program
 *    Input: N/A
 *   Output: N/A
 * ---------------------------------------------------------- */
void TimerA_Initialization(void)
{
    // Set H.E. and U.S.S. pins to be used a Timer A
    TA0_PORT->SEL0 |=  (HE_PIN);
    TA0_PORT->SEL1 &= ~(HE_PIN);

    // Set ECHO as input.  - Receiving sound pulse
    TA0_PORT->DIR &=  ~(HE_PIN);

    // Enable pull-up resistor
    TA0_PORT->REN |= HE_PIN;
    TA0_PORT->OUT |= HE_PIN;


    // Sets period for timer(s)
    TIMER_A0->CCR[0]  = 65535-1;


    /* ---- Timer A registers for Hall Effect Sensor ----
     *   BITS       Description
     * -------------------------------
     * 15 - 12      Capture on falling edge
     * 11 -  8      Capture Mode
     *  7 -  4      Reset/set | Interrupt disabled
     *  3 -  0      - - -
     * ------------------------------- */
    TIMER_A0->CCTL[1] = 0x81F0;
    TIMER_A0->CCR[1]  = 0; // Duty cycle

    /* ---- Timer A register ----
     *   BITS       Description
     * -------------------------------
     * 15 - 12      - - -
     * 11 -  8      ACLK
     *  7 -  4      Count in continuous mode
     *  3 -  0      - - -
     * ------------------------------- */
    TIMER_A0->CTL = 0x0120;
}

/* ----------------------------------------------------------
 * Function: Timer A1 TRIG USS setup
 *    Input: N/A
 *   Output: N/A
 * ---------------------------------------------------------- */
void USS_TRIG_Initialization(void)
{
    // Set H.E. and U.S.S. pins to be used a Timer A
    TA1_PORT->SEL0 |=  (USS_TRIG_PIN | USS_ECHO_PIN);
    TA1_PORT->SEL1 &= ~(USS_TRIG_PIN | USS_ECHO_PIN);

    TA1_PORT->DIR |=  (USS_TRIG_PIN);
    // Set ECHO as input.  - Receiving sound pulse
    TA1_PORT->DIR &=  ~(USS_ECHO_PIN);

    // Sets period for timer(s)
    TIMER_A1->CCR[0]  = 65535-1;

    /* ---- Timer A registers for TRIG (USS) ----
     *   BITS       Description
     * -------------------------------
     * 15 - 12      - - - -
     * 11 -  8      Capture Mode
     *  7 -  4      Reset/set | Interrupt disabled
     *  3 -  0      - - -
     * ------------------------------- */
    TIMER_A1->CCTL[1] = 0x1E0;
    TIMER_A1->CCR[1]  = 30; // Duty cycle

    /* ---- Timer A registers for ECHO (USS) ----
     *   BITS       Description
     * -------------------------------
     * 15 - 12      Capture on rising + falling
     * 11 -  8      Capture Mode
     *  7 -  4      Reset/set | Interrupt enabled
     *  3 -  0      - - -
     * ------------------------------- */
    TIMER_A1->CCTL[2] = 0xC1F0;

    /* ---- Timer A register ----
     *   BITS       Description
     * -------------------------------
     * 15 - 12      - - -
     * 11 -  8      ACLK
     *  7 -  4      Count in continuous mode
     *  3 -  0      - - -
     * ------------------------------- */
    TIMER_A1->CTL = 0x02A0;
}

/* ----------------------------------------------------------
 * Function: Timer A0 Interrupt Handler
 *    Input: N/A
 *   Output: N/A
 * ---------------------------------------------------------- */
void TA0_N_IRQHandler(void)
{
    // When program detects a falling edge, it will reset TACLR
    if(!(TA0_PORT->IN & HE_PIN))
    {
        if(Period == 0 && STARTUP_FLAG) // If it detects a falling edge at start-up
        {
            TIMER_A0->CTL |= 0x4; // Clear timer
            STARTUP_FLAG = 0;
        }
        else
        {
            Period = TIMER_A0->CCR[1]; // Save current time at falling edge
            TIMER_A0->CTL |= 0x4; // Clear timer
        }

        // Calculates RPM
        calculate_RPM();
        // Calculate MPH
        calculate_MPH();
    }

    if(TA1_PORT->IN & USS_ECHO_PIN)
        TIMER_A1->CTL |= 0x4; // Clear timer

    if(!(TA1_PORT->IN & USS_ECHO_PIN))
        currentedge = TIMER_A1->R;

    TIMER_A0->CCTL[1] &= ~BIT0; // Clear TA0.1 flags
    TIMER_A1->CCTL[2] &= ~BIT0; // Clear TA0.2 flags
}

/* ----------------------------------------------------------
 * Function: Timer A0 Interrupt Handler
 *    Input: N/A
 *   Output: N/A
 * ---------------------------------------------------------- */
void TA1_N_IRQHandler(void)
{
    if(TA1_PORT->IN & USS_ECHO_PIN)
    {
        TIMER_A1->CTL |= TACLR; // Clear timer
        TIMER_A1->CTL = 0x02A0;
    }

    if(!(TA1_PORT->IN & USS_ECHO_PIN))
        currentedge = TIMER_A1->CCR[2];

    TIMER_A1->CCTL[2] &= ~BIT0; // Clear TA0.2 flags
}

/* ----------------------------------------------------------
 * Function: Calculate RPM of motor
 *    Input: N/A
 *   Output: N/A
 * ---------------------------------------------------------- */
void calculate_RPM(void)
{
    HC_time = Period * timePerCycle;
    FC_time = 2 * HC_time;
    Freq = 1 / FC_time;

    RPM = 60 * Freq;
}

/* ----------------------------------------------------------
 * Function: Calculate MPH of motor
 *    Input: N/A
 *   Output: N/A
 * ---------------------------------------------------------- */
void calculate_MPH(void)
{
    diameter_feet = DIAMETER / 12;
    circum = diameter_feet * pi;
    feetPerMin = circum * RPM;
    feetPerHour = feetPerMin * 60;

    MPH = feetPerHour / FPH_MPH;
}


/* ----------------------------------------------------------
 * Function: Sets Timer A to be used for measuring RPM
 *    Input: N/A
 *   Output: N/A
 * ---------------------------------------------------------- */
void setTimerA_Period(void)
{
    TIMER_A0->CCR[0]  = 65535-1;            // Sets the Period for the count
    TIMER_A0->CCTL[1] = 0x81F0;              // CCR1 reset/set mode 7
    TIMER_A0->CCR[1]  = 0;
    TIMER_A0->CTL     = 0x0120;
}
