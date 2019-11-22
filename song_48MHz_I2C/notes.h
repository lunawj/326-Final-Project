//frequency values for notes
/*
 * Author: Wesley Luna
 * Date: November 10th, 2018
 * Description: This holds the frequency values for all the notes, length, and traits
 */
#ifndef NOTES_H_
#define NOTES_H_


#define C0      16.35
#define Cs0     17.32
#define Db0     17.32
#define D0      18.35
#define Ds0     19.45
#define Eb0     19.45
#define E0      20.60
#define F0      21.83
#define Fs0     23.12
#define Gb0     23.12
#define G0      24.50
#define Gs0     25.96
#define Ab0     25.96
#define A0      27.50
#define As0     29.14
#define Bb0     29.14
#define B0      30.87

#define C1      32.70
#define Cs1     17.32
#define Db1     34.65
#define D1      36.71
#define Ds1     38.89
#define Eb1     38.89
#define E1      41.20
#define F1      43.65
#define Fs1     46.25
#define Gb1     46.25
#define G1      49.00
#define Gs1     51.91
#define Ab1     51.91
#define A1      55.00
#define As1     58.27
#define Bb1     58.27
#define B1      61.74

#define C2      65.41
#define Cs2     69.30
#define Db2     69.30
#define D2      73.42
#define Ds2     77.78
#define Eb2     77.78
#define E2      82.41
#define F2      87.31
#define Fs2     92.50
#define Gb2     92.50
#define G2      98.00
#define Gs2     103.83
#define Ab2     103.83
#define A2      110.00
#define As2     116.54
#define Bb2     116.54
#define B2      123.47

#define C3      130.81
#define Cs3     138.59
#define Db3     138.59
#define D3      146.83
#define Ds3     155.56
#define Eb3     155.56
#define E3      164.81
#define F3      174.61
#define Fs3     185.00
#define Gb3     185.00
#define G3      196.00
#define Gs3     207.65
#define Ab3     207.65
#define A3      220.00
#define As3     233.08
#define Bb3     233.08
#define B3      246.94

#define C4      261.63
#define Cs4     277.18
#define Db4     277.18
#define D4      293.66
#define Ds4     311.13
#define Eb4     311.13
#define E4      329.63
#define F4      349.23
#define Fs4     369.99
#define Gb4     369.99
#define G4      392.00
#define Gs4     415.30
#define Ab4     415.30
#define A4      440.00
#define As4     466.16
#define Bb4     466.16
#define B4      493.88

#define C5      523.25
#define Cs5     554.37
#define Db5     554.37
#define D5      587.33
#define Ds5     622.25
#define Eb5     622.25
#define E5      659.25
#define F5      698.46
#define Fs5     739.99
#define Gb5     739.99
#define G5      783.99
#define Gs5     830.61
#define Ab5     830.61
#define A5      880.00
#define As5     932.33
#define Bb5     932.33
#define B5      987.77

#define C6      1046.50
#define Cs6     1108.73
#define Db6     1108.73
#define D6      1174.66
#define Ds6     1244.51
#define Eb6     1244.51
#define E6      1318.51
#define F6      1396.91
#define Fs6     1479.98
#define Gb6     1479.98
#define G6      1567.98
#define Gs6     1161.22
#define Ab6     1161.22
#define A6      1760.00
#define As6     1864.66
#define Bb6     1864.66
#define B6      1975.53

#define C7      2093.00
#define Cs7     2217.46
#define Db7     2217.46
#define D67     2349.32
#define Ds7     2489.02
#define Eb7     2489.02
#define E7      2637.02
#define F7      2793.83
#define Fs7     2959.96
#define Gb7     2959.96
#define G7      3135.96
#define Gs7     3322.44
#define Ab7     3322.44
#define A7      3520.00
#define As7     3729.31
#define Bb7     3729.31
#define B7      3951.07

#define C8      4186.01
#define Cs8     4434.92
#define Db8     4434.92
#define D68     4698.63
#define Ds8     4978.03
#define Eb8     4978.03
#define E8      5274.04
#define F8      5587.65
#define Fs8     5919.91
#define Gb8     5919.91
#define G8      6271.93
#define Gs8     6644.88
#define Ab8     6644.88
#define A8      7040.00
#define As8     7458.62
#define Bb8     7458.62
#define B8      7902.13

#define REST        0
#define SIXTEENTH   (1/4.0)
#define EIGHTH      (1/2.0)
#define ETRIPLET    (1/3.0)
#define QTRIPLET    (2/3.0)
//#define STRIPLET    (1/6.0)
#define QUARTER     1
#define HALF        2
#define WHOLE       4

#define DOTTED      1.5
#define SLUR        100
#define ACCENT      1
#define NONE        0
#define BREATH_TIME (1/1000.0)
#define END         -1

typedef struct{
float note;
float length;
int accent;
int slur;
int end;

}NOTE;


#endif /* NOTES_H_ */
