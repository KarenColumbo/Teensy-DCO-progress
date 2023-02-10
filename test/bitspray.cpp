// Language C/C++
// Teensy 4.1
//  write a function that  writes a 14 bit precision voltage "noteVolt" to a single GPIO pin using bit spray technique.
// use Teensy 4.1 Arduino standard libraries.
// comment how to set up the GPIO pin before calling the function.
// comment optimal timer settings.


void writeVoltage(int notevolt, int pin) 
{
    // bit spray technique involves setting the pin high and low for exact amount of microseconds 
    // in order to output the respective bit
    
    // notes on timing: 
    // for 0: Tlow >= 0.4;   Thigh <= 0.8
    // for 1: Tlow <= 0.2;   Thigh >= 1.6
    
    // for a 14-bit word, 
    // we will have 14 pairs of tlow and thigh that we need to set according to the bit
    
    // we will create the pairs in a loop, setting the output pin accordingly
    for (int i = 0; i < 14; i++) { 
        int mask = 1 <<(14 - i -1); // used to access respective bit of notevolt
        
        if(notevolt & mask) 
        {
            digitalWrite(pin, HIGH); //Tlow <= 0.2
            delayMicroseconds(200);
            digitalWrite(pin, LOW); //Thigh >= 1.6
            delayMicroseconds(1600);
        }
        else 
        {
            digitalWrite(pin, HIGH); //Tlow >= 0.4
            delayMicroseconds(400);
            digitalWrite(pin, LOW); //Thigh <= 0.8
            delayMicroseconds(800);
        }
    }
}

// To set up the GPIO pin, we should assign it as an output pin with the pinMode function: 
pinMode(pin, OUTPUT);

// Optimal timer settings: 
// The optimal timer settings for 14-bit precision depend on whether the respective bit is 0 or 1: 
// For bits that are 0, Tlow should be equal to or greater than 0.4 microseconds, and Thigh equal to or less than 0.8 microseconds. 
// For bits that are 1, Tlow should be equal to or less than 0.2 microseconds, and Thigh equal to or greater than 1.6 microseconds. 
// The total time required to transmit the bits is then 28 microseconds.