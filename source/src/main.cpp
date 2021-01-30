#include <Arduino.h>
#include <WiFiManager.h>
#include <reboot_bootloader.h>


void setup()
{
/*	VoidFunc _ResetVector = (VoidFunc)0x40000080;
	VoidFunc bootMain = (VoidFunc)0x40000fec;
	VoidFunc uartAttach = (VoidFunc)0x4000383c;
	U32Func Uart_Init = (U32Func)0x40003a14;
	VoidFunc ets_install_uart_printf = (VoidFunc)0x40002438;
	U32VoidFunc rtc_get_reset_reason = (U32VoidFunc)0x400025e0;
	VoidFunc bootmode1 = (VoidFunc)0x400010be;*/

	delay(1000);

	Serial.begin(115200);
	Serial.print("Reset reason: ");
	Serial.println(ESP.getResetInfo());

	int test = 0;
	Serial.printf("Hallo world! %p\r\n", &test);

	delay(1000);


	Serial.println("Reset");


	ESP.wdtDisable();
	*((volatile uint32_t*) 0x60000900) &= ~(1); // Hardware WDT OFF

	system_restart_local_bootloader();

}

void loop()
{
}

