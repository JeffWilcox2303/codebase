#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

// I2C defines
// This example will use I2C0 on GPIO8 (SDA) and GPIO9 (SCL) running at 400KHz.
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define GATE_PORT i2c0
#define GATE_SDA 8 // PROBABLY NEEDS TO CHANGE
#define GATE_SCL 9 // PROBABLY NEEDS TO CHANGE

#define DRAIN_PORT i2c1
#define DRAIN_SDA 10 // PROBABLY NEEDS TO CHANGE
#define DRAIN_SCL 11 // PROBABLY NEEDS TO CHANGE

#define resDAC 12
#define gainBits 14 // CHANGE THIS, should be the integer form of the bits that translate gain setting
const uint8_t *firstByteDAC;
const uint8_t *secondByteDAC;
int addressDAC = 120; // PROBABLY NEEDS TO CHANGE 

void writeDAC(float vout, int numDAC, int gain){
    /*
    * This function writes a voltage to a selected DAC using the desired gain setting. IDK if this works, should try something.
    * Should probably be integrated into an alarm or timer.
    * 
    * Inputs:
    * vout (float) - Output voltage as a floating point number
    * numDAC (int) - Which DAC to write the output voltage to. 0 is gate, 1 is drain
    * gain (int) - Desired gain setting for the DAC
    * 
    * Outputs:
    * None (void)
    */

    // Translate analog voltage to digital value - Vout/Vcc*2^bits
    int digVal = vout/3.3*(1 << resDAC);
    
    // Combine digital value with DAC gain and any other settings to create DAC code
    int codeDAC = (gain&gainBits) + digVal&((1 << resDAC)-1);
    uint8_t upperCode = (codeDAC>>4)&31;
    firstByteDAC = &upperCode;
    uint8_t lowerCode = codeDAC&31;
    secondByteDAC = &lowerCode;

    // Write this code to the desired DAC over I2C
    if(numDAC == 0){
        i2c_write_blocking(GATE_PORT, addressDAC, firstByteDAC, 2, false);
        i2c_write_blocking(GATE_PORT, addressDAC, secondByteDAC, 2, false);
    } 
    else if(numDAC == 1){
        i2c_write_blocking(DRAIN_PORT, addressDAC, firstByteDAC, 2, false);
        i2c_write_blocking(DRAIN_PORT, addressDAC, secondByteDAC, 2, false);
    }
    else{
        gpio_put(GATE_SDA,0);
        gpio_put(GATE_SCL,0);
        gpio_put(DRAIN_SDA,0);
        gpio_put(DRAIN_SCL,0);
    }
}

int main()
{
    stdio_init_all();

    // I2C Initialisation. Using it at 400kHz.
    i2c_init(GATE_PORT, 400*1000);
    
    gpio_set_function(GATE_SDA, GPIO_FUNC_I2C);
    gpio_set_function(GATE_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(GATE_SDA);
    gpio_pull_up(GATE_SCL);

    i2c_init(DRAIN_PORT, 400*1000);
    
    gpio_set_function(DRAIN_SDA, GPIO_FUNC_I2C);
    gpio_set_function(DRAIN_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(DRAIN_SDA);
    gpio_pull_up(DRAIN_SCL);
    // For more examples of I2C use see https://github.com/raspberrypi/pico-examples/tree/master/i2c

    while (true) {
        printf("Hello, world!\n");
        sleep_ms(1000);
    }
}
