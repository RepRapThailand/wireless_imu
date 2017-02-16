#include "osapi.h"
#include "ip_addr.h"
#include "espconn.h"
#include "user_interface.h"
#include "mem.h"

LOCAL os_timer_t timer;
LOCAL struct espconn myconn;
char data;

uint32 ICACHE_FLASH_ATTR
user_rf_cal_sector_set(void)
{
    enum flash_size_map size_map = system_get_flash_size_map();
    uint32 rf_cal_sec = 0;

    switch (size_map) {
        case FLASH_SIZE_4M_MAP_256_256:
            rf_cal_sec = 128 - 8;
            break;

        case FLASH_SIZE_8M_MAP_512_512:
            rf_cal_sec = 256 - 5;
            break;

        case FLASH_SIZE_16M_MAP_512_512:
        case FLASH_SIZE_16M_MAP_1024_1024:
            rf_cal_sec = 512 - 5;
            break;

        case FLASH_SIZE_32M_MAP_512_512:
        case FLASH_SIZE_32M_MAP_1024_1024:
            rf_cal_sec = 1024 - 5;
            break;

        default:
            rf_cal_sec = 0;
            break;
    }

    return rf_cal_sec;
}



void ICACHE_FLASH_ATTR loop(void){
	os_timer_disarm(&timer);

	//The remote IP address and port must be redeclared every time
	myconn.proto.udp->remote_port = 28752;
	const char my_remote_ip[4] = {192, 168, 4, 2};
	os_memcpy(myconn.proto.udp->remote_ip, my_remote_ip, 4);

	//uint8 result = system_get_vdd33();
	//char res[2] = (char *) result;
	/*i2c_master_start();
	i2c_master_writeByte(0x18); //00011000

	i2c_master_writeByte(0x8); //00001000

	i2c_master_start();
	i2c_master_writeByte(0x19); //00011001
	while(!i2c_master_checkAck());
	data = i2c_master_readByte();
	i2c_master_send_nack();
	i2c_master_stop();*/
	
	//Read compass data
	i2c_master_start();
	i2c_master_writeByte(0xd0); //11010000
	i2c_master_checkAck();
	i2c_master_writeByte(0x42); //01000010
	i2c_master_checkAck();
	i2c_master_start();
	i2c_master_writeByte(0xd1); //11010001
	i2c_master_checkAck();
	data = i2c_master_readByte();
	i2c_master_send_nack();
	i2c_master_stop();

	espconn_send(&myconn, &data, 1);

	//Run this function every 30ms
	os_timer_setfn(&timer, (os_timer_func_t *)loop, NULL);
	os_timer_arm(&timer, 30, 0);
}




void ICACHE_FLASH_ATTR user_init()
{
	//Wait for MPU to reset
	unsigned long int delay = 86666;
	while(delay > 1){
		delay--;
	}

	i2c_master_gpio_init();

	//Network is unsecured, so password doesn't matter
	char ssid[32] = "gyro";
    char password[64] = "password";

   	struct softap_config softapConf;
	myconn.type = ESPCONN_UDP;

	//SoftAP mode
    wifi_set_opmode( 0x2 );

	wifi_softap_get_config(&softapConf);

	os_memset(softapConf.ssid, 0, 32);
   	os_memset(softapConf.password, 0, 64);
    os_memcpy(softapConf.ssid, ssid, 32);
    os_memcpy(softapConf.password, password, 64);

	softapConf.channel = 11;
	softapConf.max_connection = 1;
	softapConf.ssid_hidden = 0;

	myconn.proto.udp = (esp_udp *)os_zalloc(sizeof(esp_udp));

	const char my_remote_ip[4] = {192, 168, 4, 2};
	os_memcpy(myconn.proto.udp->remote_ip, my_remote_ip, 4);

	myconn.proto.udp->remote_port = 28752;

	espconn_create(&myconn);

    wifi_softap_set_config(&softapConf);

	//Enable i2c bypass mode
	/*i2c_master_start();
	i2c_master_writeByte(0xd0); //11010000
	i2c_master_checkAck();
	i2c_master_writeByte(0x37); //00110111
	i2c_master_checkAck();
	i2c_master_writeByte(0x2); //00000010
	i2c_master_checkAck();
	i2c_master_stop();

	delay = 86666;
	while(delay > 1){
		delay--;
	}


	//Enable free-running mode 2
	i2c_master_start();
	i2c_master_writeByte(0x18); //00011000
	i2c_master_checkAck();
	i2c_master_writeByte(0xa); //00001010
	i2c_master_checkAck();
	i2c_master_writeByte(0x16); //00010110
	i2c_master_checkAck();
	i2c_master_stop();

	delay = 86666;
	while(delay > 1){
		delay--;
	}*/

	loop();
}
