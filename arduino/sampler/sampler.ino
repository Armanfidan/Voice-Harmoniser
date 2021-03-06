#include <SoftwareSerial.h>
#include <math.h>
#include <stdio.h>

#include "midi.h"
#include "audio_out.h"

/*
Output pins of the A register form an 8-bit binary number using A7-A0 (71-78).
From this, a resistor ladder is attached to make an analog signal.

The output pins have the corresponding values:
Register id  A7   A6   A5   A4   A3   A2   A1   A0
Pin number   71   72   73   74   75   76   77   78
Value        128  64   32   16   8    4    2    1
*/

#define SAMPLE_LENGTH pow(10, -3) // 1 milisecond samples
#define SAMPLE_RATE 44100 // standard 44.1kHz sample rate

#define SAMPLE_ARR_SIZE (size_t) round(SAMPLE_RATE * SAMPLE_LENGTH)

#define AUDIO_IN A0

#define MIDI_IN 19
#define MIDI_OUT 18

#define PITCH_DETECTOR_IN 17
#define PITCH_DETECTOR_OUT 16

SoftwareSerial midiDevice(MIDI_IN, MIDI_OUT);
SoftwareSerial pitchDetector(PITCH_DETECTOR_IN, PITCH_DETECTOR_OUT);

note notes[MAX_VOICES];

uint8_t sample[SAMPLE_ARR_SIZE];
uint8_t amplitude;

int midi_msg[3];
char arduino_status_msg[100];

double voice_f;

void get_midi_msg(int msg[3], Stream &midiDevice){
    size_t msg_length = midiDevice.available();
    size_t i = 0;

    while(i < msg_length){
        msg[i] = midiDevice.read(); //need to check for status byte
        i++;
    }
}

void setup(){
    Serial.begin(9600); // USB baud rate
    while(!Serial); // While USB connection has not been established
    Serial.print("USB connection established\n");
    
    midiDevice.begin(31250); // MIDI baud rate
    while(!midiDevice);
    Serial.print("MIDI Device connection established\n");

    pitchDetector.begin(9600); // Arduino baud rate
    while(!pitchDetector);
    Serial.print("Pitch Detector Arduino connection established\n");

    sprintf(
        arduino_status_msg, 
        "Initial Setup:\nSampling Rate: %d Hz.\nSample length %d ms.\n%d frames per sample.\n", 
        SAMPLE_RATE, 
        (int) SAMPLE_LENGTH * 1000, 
        SAMPLE_ARR_SIZE
    );

    Serial.print(arduino_status_msg);

    // Do not start processing signal until voice frequency determined
    while(!pitchDetector.available()){ 
        pitchDetector.listen();
    }

    voice_f = pitchDetector.parseFloat();
    
    sprintf(arduino_status_msg, "New voice frequency: %d\n", voice_f);
    Serial.print(arduino_status_msg);

    // Set up audio output pins
    DDRA = B11111111;
}

void loop(){
    midiDevice.listen();
    if(midiDevice.available()){
        get_midi_msg(midi_msg, midiDevice);
        handle_midi_msg(midi_msg, notes);
        report_midi_change(midi_msg, arduino_status_msg);
        Serial.print(arduino_status_msg);
    }

    pitchDetector.listen();

    if(pitchDetector.available()){
        voice_f = pitchDetector.parseFloat();

        sprintf(arduino_status_msg, "New voice frequency: %f\n", voice_f);
        Serial.print(arduino_status_msg);
    }

    for(size_t frame = 0; frame < SAMPLE_ARR_SIZE; frame++){ // Record the sample frame by frame
        sample[frame] = analogRead(AUDIO_IN) / 4; // Work out how to use NOT_AUDIO_IN to reduce noise
        delay(pow(SAMPLE_RATE, -1) * 1000);
    }

    for(size_t frame = 0; frame < SAMPLE_ARR_SIZE; frame++){ // Play back sample frame by frame
        // If voice_f is zero, then return the original frame
        PORTA = voice_f ? combined_notes_amplitude_8bit(sample, notes, voice_f, frame, SAMPLE_ARR_SIZE) : sample[frame];
        delay(pow(SAMPLE_RATE, -1) * 1000);
    }
}