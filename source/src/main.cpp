#include <Arduino.h>
#include <WiFiManager.h>
#include <reboot_uart_dwnld.h>


void setup()
{
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

	system_restart_local_uart_dwnld();

}

void loop()
{
}

