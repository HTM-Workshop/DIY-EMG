/* DIY Spiker unit controller - v1.0
   Written by Kevin Williams
    10/26/2020:
    - Complete rewrite to match new hardware specifications
      with no code reuse. 
    - Optimized for high-performance analog readings.
    - Compatible with Arduino Uno and Arduino Nano.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/


#define EMG_INPUT_PIN A0            // DO NOT CHANGE, optimizations for A0 port only
#define TRIGGER_HIGH 450            // High limit for trigger, adjust to change false-trigger rejection
#define TRIGGER_LOW 150             // Low limit for trigger, adjust to change false-trigger rejection
#define TRIGGER_WAIT_TIME_MS 50     // Delay between triggers, lower to increase speed of triggering
typedef unsigned char BYTE;


// Read analog input value and store in *level
// optimized for high-performance
inline void __attribute__((optimize("O3"))) update_reading(int *level) {

    // begin ADC conversion
    ADCSRA |= (1 << ADSC);

    // poll ADCSRA until end of conversion
    while((ADCSRA & (1 << ADSC)));

    // store analog reading
    *level = ADC;
    
    return;
}


// Process trigger (if needed)
// optimized for high-performance
inline void __attribute__((optimize("O3"))) update_output(const int level) {
    static BYTE logic_out = 0;
    switch(logic_out) {
        case 0: {
            if(level >= TRIGGER_HIGH) {
                Serial.write("T\n");                // write value and flush
                Serial.flush();
                logic_out = 1;
                PORTB |= 0x20;                      // toggle builtin LED pin
            }
            break;
        }
        case 1: {
            if(level <= TRIGGER_LOW) {
                logic_out = 0;
                PORTB &= 0xDF;                      // toggle buitlin LED pin
                delay(TRIGGER_WAIT_TIME_MS);        // delay before next trigger
            }
            break;
        }
    }
    return;
}


void setup(void) {
    Serial.begin(115200);

    // indicator
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);
   
    // setup analog input A0 for operation
    // optimized for high-performance
    ADMUX = (1 << REFS0);
    ADCSRA = 0x82;
}


// optimized for high-performance
void __attribute__((optimize("O3"), flatten)) loop(void) {
    int analog_level;
    update_reading(&analog_level);
    update_output(analog_level);
}
