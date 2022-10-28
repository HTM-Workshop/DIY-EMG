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
BYTE channel_select = 0;

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
    String mode;
    char buff[10];
    
    // indicator
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);

    // run until we get a valid input mode string from the computer
    while(1) {
//    
//        // Wait for input from the capture program to determine which 
//        while(Serial.available() == 0) {}
//        if(Serial.available()) {
//            mode = Serial.readString();     // using C++ strings for input
//            mode.trim();
//        } else {
//            continue;
//        }
        BYTE idx = 0;
        BYTE do_read = 1;
        while(do_read != 0) {
            Serial.flush();
            if(Serial.available() == 0)
                continue;
            char c = Serial.read();
            if(c != '$')
                continue;
            delay(1000);
            while(Serial.available() != 0) {
                c = Serial.read();
                if(c == '\n' or c == '\r') {
                    do_read = 0;
                    buff[idx] = 0;
                    break;
                }
                buff[idx] = c;
                idx++;
            }
        }

        // use input from serial to select ADC channel
        if(strcmp(buff, "NORMAL") == 0) {
            channel_select = 0;
            break;
        } 
        else if(strcmp(buff, "WAVE") == 0) {
            channel_select = 1;
            break;
        } else {
            
            // input mode string was invalid, send error message to program,
            // clear buffers, and try again
            Serial.print("INVALID \"");
            Serial.print(buff);
            Serial.println("\"");
            Serial.flush();
            continue;
        }
    } 
    while(Serial.available() > 0) {
        Serial.read();
    }
    Serial.println("READY");
    Serial.flush();

    // setup analog input A0 or A1 for operation
    // optimized for high-performance
    // channel_select determines which ADC channel is active
    // 0 = A0, 1 = A1
    ADMUX = (1 << REFS0) | channel_select;
    ADCSRA = 0x82;
}


// optimized for high-performance
void __attribute__((optimize("O3"), flatten)) loop(void) {
    int analog_level;
    char output_buffer[6];
    while(channel_select == 0) {
        update_reading(&analog_level);
        update_output(analog_level);
    }
    while(channel_select == 1) {
        while(Serial.available() == 0) {}
        while(Serial.available() > 0) {
            Serial.read();
        }
        update_reading(&analog_level);
        sprintf(output_buffer, "$%03d\n", analog_level);
        Serial.print(output_buffer);
    }
}
