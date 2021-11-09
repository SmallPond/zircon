#include <stdint.h>
#include "debug.h"


void delay(uint64_t time);
void put32(uint64_t addr, uint32_t c);
uint32_t get32(uint64_t addr);
void uart_init(void);
#define AUX_MU_BAUD(baud) ((AUX_UART_CLOCK/(baud*8))-1)

static volatile uint32_t have_inited = 0;
enum {
    PERIPHERAL_BASE = 0xFE000000,
    GPFSEL0         = PERIPHERAL_BASE + 0x200000,
    GPSET0          = PERIPHERAL_BASE + 0x20001C,
    GPCLR0          = PERIPHERAL_BASE + 0x200028,
    GPPUPPDN0       = PERIPHERAL_BASE + 0x2000E4
};
enum {
    Pull_None = 0,
    Pull_Down = 1, // Are down and up the right way around?
    Pull_Up = 2
};
enum {
    GPIO_MAX_PIN       = 53,
    GPIO_FUNCTION_OUT  = 1,
    GPIO_FUNCTION_ALT5 = 2,
    GPIO_FUNCTION_ALT3 = 7
};
enum {
    AUX_BASE        = PERIPHERAL_BASE + 0x215000,
    AUX_IRQ         = AUX_BASE,
    AUX_ENABLES     = AUX_BASE + 4,
    AUX_MU_IO_REG   = AUX_BASE + 64,
    AUX_MU_IER_REG  = AUX_BASE + 68,
    AUX_MU_IIR_REG  = AUX_BASE + 72,
    AUX_MU_LCR_REG  = AUX_BASE + 76,
    AUX_MU_MCR_REG  = AUX_BASE + 80,
    AUX_MU_LSR_REG  = AUX_BASE + 84,
    AUX_MU_MSR_REG  = AUX_BASE + 88,
    AUX_MU_SCRATCH  = AUX_BASE + 92,
    AUX_MU_CNTL_REG = AUX_BASE + 96,
    AUX_MU_STAT_REG = AUX_BASE + 100,
    AUX_MU_BAUD_REG = AUX_BASE + 104,
    AUX_UART_CLOCK  = 500000000,
    UART_MAX_QUEUE  = 16 * 1024
};



// void uart_init (void)
// {
// 	uint32_t selector;

// 	selector = get32(GPFSEL1);
// 	selector &= ~(7<<12);                   // clean gpio14
// 	selector |= 2<<12;                      // set alt5 for gpio14
// 	selector &= ~(7<<15);                   // clean gpio15
// 	selector |= 2<<15;                      // set alt5 for gpio15
// 	put32(GPFSEL1,selector);

// 	put32(GPPUD,0);
// 	delay(150);
// 	put32(GPPUDCLK0,(1<<14)|(1<<15));
// 	delay(150);
// 	put32(GPPUDCLK0,0);

// 	put32(AUX_ENABLES,1);                   //Enable mini uart (this also enables access to it registers)
// 	put32(AUX_MU_CNTL_REG,0);               //Disable auto flow control and disable receiver and transmitter (for now)
// 	put32(AUX_MU_IER_REG,0);                //Disable receive and transmit interrupts
// 	put32(AUX_MU_LCR_REG,3);                //Enable 8 bit mode
// 	put32(AUX_MU_MCR_REG,0);                //Set RTS line to be always high
// 	put32(AUX_MU_BAUD_REG,270);             //Set baud rate to 115200

// 	put32(AUX_MU_CNTL_REG,3);               //Finally, enable transmitter and receiver
// }
void mmio_write(long reg, unsigned int val);
unsigned int mmio_read(long reg);
unsigned int gpio_call(unsigned int pin_number, unsigned int value, unsigned int base, unsigned int field_size, unsigned int field_max);
unsigned int gpio_set     (unsigned int pin_number, unsigned int value);
unsigned int gpio_clear   (unsigned int pin_number, unsigned int value);
unsigned int gpio_pull    (unsigned int pin_number, unsigned int value);
unsigned int gpio_function(unsigned int pin_number, unsigned int value);
void gpio_useAsAlt5(unsigned int pin_number);

void mmio_write(long reg, unsigned int val) 
{ 
	*(volatile unsigned int *)reg = val; 
}
unsigned int mmio_read(long reg) 
{ 
	return *(volatile unsigned int *)reg;
}

unsigned int gpio_call(unsigned int pin_number, unsigned int value, unsigned int base, unsigned int field_size, unsigned int field_max) {
    unsigned int field_mask = (1 << field_size) - 1;
  
    if (pin_number > field_max) return 0;
    if (value > field_mask) return 0; 

    unsigned int num_fields = 32 / field_size;
    unsigned int reg = base + ((pin_number / num_fields) * 4);
    unsigned int shift = (pin_number % num_fields) * field_size;

    unsigned int curval = mmio_read(reg);
    curval &= ~(field_mask << shift);
    curval |= value << shift;
    mmio_write(reg, curval);

    return 1;
}

unsigned int gpio_set     (unsigned int pin_number, unsigned int value) { return gpio_call(pin_number, value, GPSET0, 1, GPIO_MAX_PIN); }
unsigned int gpio_clear   (unsigned int pin_number, unsigned int value) { return gpio_call(pin_number, value, GPCLR0, 1, GPIO_MAX_PIN); }
unsigned int gpio_pull    (unsigned int pin_number, unsigned int value) { return gpio_call(pin_number, value, GPPUPPDN0, 2, GPIO_MAX_PIN); }
unsigned int gpio_function(unsigned int pin_number, unsigned int value) { return gpio_call(pin_number, value, GPFSEL0, 3, GPIO_MAX_PIN); }

void gpio_useAsAlt5(unsigned int pin_number) {
    gpio_pull(pin_number, Pull_None);
    gpio_function(pin_number, GPIO_FUNCTION_ALT5);
}

void uart_init() {
    mmio_write(AUX_ENABLES, 1); //enable UART1
    mmio_write(AUX_MU_IER_REG, 0);
    mmio_write(AUX_MU_CNTL_REG, 0);
    mmio_write(AUX_MU_LCR_REG, 3); //8 bits
    mmio_write(AUX_MU_MCR_REG, 0);
    mmio_write(AUX_MU_IER_REG, 0);
    mmio_write(AUX_MU_IIR_REG, 0xC6); //disable interrupts
    mmio_write(AUX_MU_BAUD_REG, AUX_MU_BAUD(115200));
    gpio_useAsAlt5(14);
    gpio_useAsAlt5(15);
    mmio_write(AUX_MU_CNTL_REG, 3); //enable RX/TX
}


// boot-shim 时调用，后续的console 输出会走 boot-shim-config.h 文件的定义的 uart
void uart_pputc(char c)
{
    if(!have_inited){
        uart_init();
        have_inited = 1;
    }
    while(1) {
		if(get32(AUX_MU_LSR_REG)&0x20) 
			break;
	}
	put32(AUX_MU_IO_REG,c);
}
