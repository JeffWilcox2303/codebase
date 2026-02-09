#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

#define GATE 0
#define DRAIN 1

#define GATE_VOLTAGE 0
#define DRAIN_VOLTAGE 2

// I2C defines
// This example will use I2C0 on GPIO8 (SDA) and GPIO9 (SCL) running at 400KHz.
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define GATE_PORT i2c0
#define GATE_SDA 16
#define GATE_SCL 17

#define DRAIN_PORT i2c1
#define DRAIN_SDA 18
#define DRAIN_SCL 19

#define resDAC 12
#define resADC 12
uint8_t DACcodes[3];
const uint8_t *DACbytes;
int addressDAC = 0b1100001; // Last bit depends on solder jumper

void writeDAC(float vout, int numDAC){
    /*
    * This function writes a voltage to a selected DAC using the desired gain setting.
    * Currently works, however it is limited by hardware (opamp) and can only output up to 2V.
    * Should probably be integrated into an alarm or timer.
    * 
    * Inputs:
    * vout (float) - Output voltage as a floating point number
    * numDAC (int) - Which DAC to write the output voltage to. 0 is gate, 1 is drain
    * 
    * Outputs:
    * None (void)
    */

    // Translate analog voltage to digital value - Vout/Vcc*2^bits
    int digVal = (int)((vout/3.3f)*(float)((1 << resDAC)-1));
    
    // Combine digital value with DAC gain and any other settings to create DAC code
    int codeDAC = digVal&((1 << resDAC)-1);
    DACcodes[0] = 64;
    DACcodes[1] = (codeDAC>>4)&255;
    DACcodes[2] = (codeDAC<<4)&255;
    DACbytes = &DACcodes[0];

    // Write this code to the desired DAC over I2C
    if(numDAC == GATE){
        i2c_write_blocking(GATE_PORT, addressDAC, DACbytes, 3, true);
    } 
    else if(numDAC == DRAIN){
        i2c_write_blocking(DRAIN_PORT, addressDAC, DACbytes, 3, true);
    }
    else{
        gpio_put(GATE_SDA,0);
        gpio_put(GATE_SCL,0);
        gpio_put(DRAIN_SDA,0);
        gpio_put(DRAIN_SCL,0);
    }
}

// There is no gate current ADC so no need for modularity
int readDrainCurrent_mA(){
    adc_select_input(1);
    // Read ADC
    int result = adc_read();

    // Apply transfer function to get voltage from digital reading
    return result*3300/(1<<resADC); // THIS IS INCORRECT FOR CURRENT ONLY
}

int readVoltage_mV(int node){
    adc_select_input(node);

    // Read ADC
    int result = adc_read();

    // Apply transfer function to get voltage from digital reading
    return result*3300/(1<<resADC);
}

void sendData(){
    /*
    Ideally this will send sets of gate voltage, drain voltage, and drain current corresponding to certain time instances.
    Not sure how to do this.
    Comma separate different values and slahs separate different sets?
    */
}

void inputTrigger(){
    /*
    To start the measurements, a trigger should be received through serial data or an interrupt on a register value.
    Could maybe do a timer, but that doesn't seem ideal.
    */
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

    adc_init();
    adc_gpio_init(26);
    adc_gpio_init(27);
    adc_gpio_init(28);

    writeDAC(1.5f,GATE);
    sleep_ms(1000);
    float val = 0;

    while (true) {
        // printf("Hello, world!\n");
        writeDAC(val,DRAIN);
        sleep_ms(100);
        val += 0.1f;
    }
}
