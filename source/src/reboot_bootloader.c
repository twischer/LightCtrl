#include "reboot_bootloader.h"
#include <Arduino.h>
#include <ets_sys.h>
#include <user_interface.h>
#include <spi_flash.h>
#include "../../../framework-esp8266-nonos-sdk/driver_lib/include/driver/uart.h"

#define PERIPHS_DPORT_18	(PERIPHS_DPORT_BASEADDR + 0x018)	// 0x3feffe00 + 0x218
#define PERIPHS_DPORT_24	(PERIPHS_DPORT_BASEADDR + 0x024)	// 0x3feffe00 + 0x224
#define PERIPHS_I2C_48		(0x60000a00 + 0x348)

//void _ResetVector();
uint32_t Wait_SPI_Idle(SpiFlashChip *fc);
void Cache_Read_Disable();
int32_t system_func1(uint32_t);
void clockgate_watchdog(uint32_t);
void pm_open_rf();
void user_uart_wait_tx_fifo_empty(uint32_t ch, uint32_t arg2);
void uartAttach();
void Uart_Init(uint32_t uart_no);
void ets_install_uart_printf(uint32_t uart_no);
void UartDwnLdProc(uint8_t* ram_addr, uint32_t size, void (**user_start_ptr)());

typedef void (*VoidFunc)();
VoidFunc user_start_fptr = NULL; /* originally allocated at 0x3fffdcd0 */


void ICACHE_RAM_ATTR boot_from_something_bootloader(void (**user_start_ptr)())
{
	/* simplified for following condition
	 * (GPI >> 0x10 & 0x07) == 1 and
	 * (GPI >> 0x1D & 0x07) == 6
	 */

	/* loc_4000127a */
	uint32_t uart_no = 0; // TODO GPI BIT_18
	uint32_t divlatch = uart_baudrate_detect(uart_no, 0);
	uart_div_modify(uart_no, divlatch & 0xFFFF);
	uint32_t a0 = 2;

	/* loc_4000123b */
	if (a0 == 2) {
		UartDwnLdProc((void*)0x3fffa000, 0x2000, user_start_ptr);
	} else {
		//sip_40001160();
	}

	if (uart_no) {
		uart_buff_switch(0);
	}
}

void ICACHE_RAM_ATTR main_bootloader()
{
	// TODO may be it is part of _start()
	// at least ets_intr_lock() is called in system_restart_local()
	ets_intr_unlock();

	uartAttach();
	Uart_Init(0);
	ets_install_uart_printf(0);

	boot_from_something_bootloader((VoidFunc*)0x3fffdcd0); //&user_start_fptr);

	//if (*user_start_fptr) {
	//}

	//boot_from_flash();

	//_xtos_set_exception_handler(0x9, window_spill_exc_handler);
	// TODO ...

//	if (user_start_fptr) {
//		user_start_fptr();
//	}

	while (true);
}

void ICACHE_RAM_ATTR system_restart_core_bootloader()
{
	Wait_SPI_Idle(flashchip);

	// TODO exception when calling uart_div_modify()
	//Cache_Read_Disable();
	//CLEAR_PERI_REG_MASK(PERIPHS_DPORT_24, 0x18); /* 0x18 == ~(-0x19) */
//	_ResetVector();
//	TODO reset stack pointer
	main_bootloader();
}

void system_restart_local_bootloader()
{
	if (system_func1(0x4) == -1) {
		clockgate_watchdog(0);
		SET_PERI_REG_MASK(PERIPHS_DPORT_18, 0xffff00ff);
		pm_open_rf();
	}

	struct rst_info rst_info;
	system_rtc_mem_read(0, &rst_info, sizeof(rst_info));
	if (rst_info.reason != REASON_SOFT_WDT_RST &&
		rst_info.reason != REASON_EXCEPTION_RST) {
		ets_memset(&rst_info, 0, sizeof(rst_info));
		WRITE_PERI_REG(RTC_STORE0, REASON_SOFT_RESTART);
		rst_info.reason = REASON_SOFT_RESTART;
        	system_rtc_mem_write(0, &rst_info, sizeof(rst_info));
	}
	/* system_restart_hook() is not required because it only contains return */
	user_uart_wait_tx_fifo_empty(0, 0x7a120);
	user_uart_wait_tx_fifo_empty(1, 0x7a120);
	ets_intr_lock();
	SET_PERI_REG_MASK(PERIPHS_DPORT_18, 0x7500);
	CLEAR_PERI_REG_MASK(PERIPHS_DPORT_18, 0x7500); //~0xffff8aff;
	SET_PERI_REG_MASK(PERIPHS_I2C_48, 0x2);
	SET_PERI_REG_MASK(PERIPHS_I2C_48, 0x2);	// ~(-3)

	system_restart_core_bootloader();
}

