#include "peripherals/mini_uart.h"

#define HAS_DEVICE_TREE 1

// FIXME
static const zbi_platform_id_t platform_id = {
    .vid = PDEV_VID_BROADCOM,
    .pid = PDEV_PID_BCM2711,
    .board_name = "raspberry Pi 4B",
};

static const zbi_cpu_config_t cpu_config = {
    .cluster_count = 1,
    .clusters = {
        {
            .cpu_count = 1,
        },
    },
};
// static const zbi_cpu_config_t cpu_config = {
//     .cluster_count = 1,
//     .clusters =
//         {
//             {
//                 .cpu_count = 4,
//             },
//         },
// };

static const zbi_mem_range_t mem_config[] = {
    {
        .type = ZBI_MEM_RANGE_RAM,
        .paddr = 0x00000000,
        .length = 0x3b400000, // 9xx MB
    },
    {
        // memory to reserve to avoid stomping on bootloader data
        .type = ZBI_MEM_RANGE_RESERVED,
        .paddr = 0x00000000,
        .length = 0x00080000,           // BL31 trust-firmware date
    },
    {
        .type = ZBI_MEM_RANGE_PERIPHERAL,
        .paddr = 0xFC000000,
        .length = 0x40000000,
    },
};

static const dcfg_simple_t mini_uart_driver = {
    .mmio_phys = AUX_BASE,
    .irq = 29,
};

// static const dcfg_simple_t mini_uart_driver = {
//     .mmio_phys = 0xfe215040,
//     .irq = 29,
// };
/*
 * uart 0 0xfe201000
 * uart 2 0xfe204000
*/ 
static const dcfg_simple_t uart_driver = {
    .mmio_phys = 0xfe201000,
    .irq = 57,
};

static const dcfg_arm_gicv2_driver_t gicv2_driver = {
    .mmio_phys = 0xff840000,
    .gicd_offset = 0x1000,
    .gicc_offset = 0x2000,
    .gich_offset = 0x4000,
    .gicv_offset = 0x6000,
    .ipi_base = 12,
};

static void append_board_boot_item(zbi_header_t* bootdata) {
    // add CPU configuration
    append_boot_item(bootdata, ZBI_TYPE_CPU_CONFIG, 0, &cpu_config,
                    sizeof(zbi_cpu_config_t) +
                    sizeof(zbi_cpu_cluster_t) * cpu_config.cluster_count);

    // add kernel drivers
    // append_boot_item(bootdata, ZBI_TYPE_KERNEL_DRIVER, KDRV_MINI_UART, &mini_uart_driver,
    //                 sizeof(mini_uart_driver));
    append_boot_item(bootdata, ZBI_TYPE_KERNEL_DRIVER, KDRV_PL011_UART, &uart_driver,
                    sizeof(uart_driver));
    append_boot_item(bootdata, ZBI_TYPE_KERNEL_DRIVER, KDRV_ARM_GIC_V2, &gicv2_driver,
                   sizeof(gicv2_driver));
    // add memory configuration
    append_boot_item(bootdata, ZBI_TYPE_MEM_CONFIG, 0, &mem_config,
                    sizeof(zbi_mem_range_t) * countof(mem_config));

    // add platform ID
    append_boot_item(bootdata, ZBI_TYPE_PLATFORM_ID, 0, &platform_id, sizeof(platform_id));
}
