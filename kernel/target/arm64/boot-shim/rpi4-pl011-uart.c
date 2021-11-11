#include <stdint.h>
#include "debug.h"
#include "rpi4-gpio.h"

void delay(uint64_t time);

void uart_init(void);
#define AUX_MU_BAUD(baud) ((AUX_UART_CLOCK/(baud*8))-1)

static volatile uint32_t have_inited = 0;


void uart_init() {

    gpio_useAsAlt3(16);
    gpio_useAsAlt3(17);
    gpio_useAsAlt0(14);
    gpio_useAsAlt0(15);
    
    mmio_write(ARM_UART0_IMSC, 0x00);
    mmio_write(ARM_UART0_ICR,  0x7ff);
    mmio_write(ARM_UART0_IBRD, 0x1a);
    mmio_write(ARM_UART0_FBRD, 0x03);
    mmio_write(ARM_UART0_IFLS, 0x08);
    mmio_write(ARM_UART0_LCRH, 0x70);
    mmio_write(ARM_UART0_CR,   0xB01);  // 使能 uart 
    mmio_write(ARM_UART0_IMSC, 0x430);
}


// boot-shim 时调用，后续的console 输出会走 boot-shim-config.h 文件的定义的 uart
void uart_pputc(char c)
{
    if(!have_inited){
        uart_init();
        have_inited = 1;
    }
    while ((mmio_read(ARM_UART0_FR) & 0x20) != 0);
	mmio_write(ARM_UART0_DR,c);
}
