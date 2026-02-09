//UART
#define RCC_base_addr         0x40021000
#define APB2_offset           0x18
#define UART1_BASE        0x40013800
#define UART1_SR      (volatile uint32_t*)(UART1_BASE + 0x00)
#define UART1_DR      (volatile uint32_t*)(UART1_BASE + 0x04)
#define UART1_BRR     (volatile uint32_t*)(UART1_BASE + 0x08)
#define UART1_CR1     (volatile uint32_t*)(UART1_BASE + 0x0C)
#define portA_BASE       0x40010800
#define portA        (volatile uint32_t*)(portA_BASE + 0x04)
volatile uint32_t *const abp2 =    (volatile uint32_t*) (RCC_base_addr + APB2_offset);
//functions
void config_UART(void);
void UART_SEND_CHAR(char character);
void printUART(char* str);
void UART_SendInt(int16_t num);
void UART_SendNewLine(void);
void config_UART(){

  *abp2 |= (1 << 14) |(1 << 2);  //enable clock for uart and portA
  *portA &= ~(15 << 4);
  *portA &= ~(15 << 8);

  *portA |= (15 << 4); // TX
  *portA |= (14 << 8); //RX

  //bit rate setup : for 9600 : div=468.75 i.e. USART_BRR = 1D4C
  *UART1_BRR = 0x271;
  *UART1_CR1 = (1 << 13) | (1 << 3) | (1 << 2);
}

void UART_SEND_CHAR(char character){
  while(! (*UART1_SR & (1<<7)));
  *UART1_DR = (character << 0);
}

void printUART(char* str){
  while(*str != '\0'){
    UART_SEND_CHAR(*str);
    str++;
  }
}

void UART_SendInt(int16_t num) {
    char buf[10];
    int i = 0;

    // Handle 0
    if (num == 0) {
        UART_SEND_CHAR('0');
        return;
    }

    // Handle Negative Numbers
    if (num < 0) {
        UART_SEND_CHAR('-');
        num = -num;
    }

    // Extract digits (backwards)
    while (num > 0) {
        buf[i++] = (num % 10) + '0';
        num /= 10;
    }

    // Print digits in correct order (reverse)
    while (--i >= 0) {
        UART_SEND_CHAR(buf[i]);
    }
}
void UART_SendNewLine(void) {
    UART_SEND_CHAR('\r');
    UART_SEND_CHAR('\n');
}
