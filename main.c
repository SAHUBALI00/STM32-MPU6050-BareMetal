#include<serialPort.h>

#define RCC_base_addr         0x40021000
#define APB1_offset           0x1C
#define APB2_offset           0x18
#define PORTB_base_addr       0x40010C00
#define GPIOB_CRL_offset      0x00
#define I2C_base_addr         0x40005400
#define I2C_CR1               0x00

#define I2C_CR2               0x04

#define I2C_SR1               0x14

#define I2C_SR2               0x18

#define I2C_CCR               0x1C

#define I2C_TRISE             0x20

#define I2C_DR                0x10



typedef struct{
  int16_t Accel_X, Accel_Y, Accel_Z;
  int16_t Temperature;
  int16_t Gyro_X, Gyro_Y,Gyro_Z;
  
}mpu6050;

void config_I2C();
void I2C_START();
void I2C_STOP();
void I2C_Write(uint8_t data);
void I2C_receive(int16_t bytes[]);
mpu6050 process_mpu_data();

volatile uint32_t *const abp1 =(volatile uint32_t*) (RCC_base_addr + APB1_offset);
volatile uint32_t *const portB =  (volatile uint32_t*) (PORTB_base_addr + GPIOB_CRL_offset);
volatile uint32_t * const i2c_cr1 =    (volatile uint32_t*)(I2C_base_addr + I2C_CR1);

volatile uint32_t * const i2c_cr2 =    (volatile uint32_t*)(I2C_base_addr + I2C_CR2);

volatile uint32_t * const i2c_ccr =    (volatile uint32_t*)(I2C_base_addr + I2C_CCR);

volatile uint32_t * const i2c_trise =  (volatile uint32_t*)(I2C_base_addr + I2C_TRISE);

volatile uint32_t * const i2c_dr =     (volatile uint32_t*)(I2C_base_addr + I2C_DR);

volatile uint32_t * const i2c_sr1 =    (volatile uint32_t*)(I2C_base_addr + I2C_SR1);

volatile uint32_t * const i2c_sr2 =    (volatile uint32_t*)(I2C_base_addr + I2C_SR2);



mpu6050 data;
void setup() {
  config_UART();
  config_I2C();
//  // Wake up MPU6050
  printUART("HII lets read data from mpu6050");
  UART_SendNewLine();
  I2C_START(0x68, 0); 
  I2C_Write(0x6B); // PWR_MGMT_1 register
  I2C_Write(0x00); // Set to 0 to wake it up
  I2C_Stop();
// for(volatile int i=0; i<50000; i++);
// After I2C_Stop() or before I2C_Start()
while (*i2c_sr2 & (1 << 1)); // Wait until the BUSY bit is cleared by hardware
  
}

void config_I2C() {
  *abp1 |= (1 << 21);   //enabling clk for i2c1
  *abp2 |= (1 << 3);   //port B enable

  *portB &= ~(15 << 24);
  *portB &= ~(15 << 28);

  *portB |= (15 << 24);
  *portB |= (15 << 28);

  *i2c_cr2 = (36 << 0);  //apb1 operating freq: 36MHZ
  *i2c_ccr = 180;       //ccr value: how to toggle scl
  *i2c_trise = 37;
  *i2c_cr1 |= (1 << 0); //peripheral enable

}

void I2C_START(volatile uint8_t address, int r_w) {
  *i2c_cr1 |= (1 << 8); //set start bit
  int timeout=0;

  while (!(*i2c_sr1 & (1 << 0)));  // 2. Wait for SB (EV5)

  *i2c_dr = (address << 1) | r_w;  // //Shifting 0x68(mpu6050 addr) left and adding 1 gives you 0xD1.
  
  while (!(*i2c_sr1 & (1 << 1)));  //addr bit wait

  uint32_t temp = *i2c_sr1;        // Read SR1

  temp = *i2c_sr2;                  //Read SR2


}

mpu6050 process_mpu_data(){
  int16_t raw[14] ;
   I2C_receive(raw);
  mpu6050 sensor;

 // Accelerometer (0x3B to 0x40)
    sensor.Accel_X = (int16_t)(raw[0] << 8 | raw[1]);
    sensor.Accel_Y = (int16_t)(raw[2] << 8 | raw[3]);
    sensor.Accel_Z = (int16_t)(raw[4] << 8 | raw[5]);

    // Temperature (0x41 to 0x42)
    sensor.Temperature = (int16_t)(raw[6] << 8 | raw[7]);

    // Gyroscope (0x43 to 0x48)
    sensor.Gyro_X = (int16_t)(raw[8] << 8 | raw[9]);
    sensor.Gyro_Y = (int16_t)(raw[10] << 8 | raw[11]);
    sensor.Gyro_Z = (int16_t)(raw[12] << 8 | raw[13]);

    return sensor;

}

 void I2C_receive(int16_t bytes[]) {
    // 1. Point to the first register (Accel X High)
   I2C_START(0x68, 0); 
    I2C_Write(0x3B); 
    while (!(*i2c_sr1 & (1 << 2))); // Wait for BTF

    I2C_Stop();
// After I2C_Stop() or before I2C_Start()
while (*i2c_sr2 & (1 << 1)); // Wait until the BUSY bit is cleared by hardware

    // --- CRITICAL FIX: ENABLE ACK BEFORE START ---
    *i2c_cr1 |= (1 << 10); 
    
    I2C_START(0x68, 1); // Start in Read Mode
    
    // printUART(" Restart Done, Waiting for Data... ");

    for(int i = 0; i < 14; i++) {
        // Prepare NACK for the last byte
        if (i == 13) {
            *i2c_cr1 &= ~(1 << 10); // Clear ACK (NACK)
            *i2c_cr1 |= (1 << 9);   // Set STOP
        }

        // Wait for RxNE
        int timeout = 0;
        while (!(*i2c_sr1 & (1 << 6))) {
            if(++timeout > 100000) {
                printUART("!Data Timeout at byte ");
                UART_SendInt(i);
                return;
            }
        }
        
        bytes[i] = *i2c_dr; // Read clears RxNE
        
        // Re-enable ACK for all bytes except the last one
        if (i < 12) {
            *i2c_cr1 |= (1 << 10);
        }
    }
    
}


 void I2C_Stop(void) {

  *i2c_cr1 |= (1 << 9);            // Generate STOP

}
//f

void I2C_Write(uint8_t data) {

  while (!(*i2c_sr1 & (1 << 7)));  // Wait for TxE (Transmit Empty) (set by hardware of receiving acknowledgement)

  *i2c_dr = data;

  while (!(*i2c_sr1 & (1 << 2))){
    if (*i2c_sr1 & (1 << 10)) { // Check AF (Acknowledge Failure)
            *i2c_sr1 &= ~(1 << 10); // Clear AF bit
            printUART("write I2C NACK Error!\n");
  }  // Wait for BTF (Byte Transfer Finished)

}
  
}



void loop(){
    data = process_mpu_data();
    printUART(" temperature : ");
    UART_SendInt((data.Temperature)/340 + 36.53);
    printUART("  accelX : ");
    UART_SendInt(data.Accel_X);
    printUART("  gyroX : ");
    UART_SendInt(data.Gyro_X);
    UART_SendNewLine();
    delay(2000);
  
}


