#include <stdint.h>
#include "debug.h"
#include "rpi4-gpio.h"

void delay(uint64_t time);
void put32(uint64_t addr, uint32_t c);
uint32_t get32(uint64_t addr);
void uart_init(void);
#define AUX_MU_BAUD(baud) ((AUX_UART_CLOCK/(baud*8))-1)

static volatile uint32_t have_inited = 0;

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
