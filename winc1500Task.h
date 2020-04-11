/*
 * winc1500Task.h
 *
 * Created: 4/8/2020 7:20:58 PM
 *  Author: ThreeBoysTech
 */ 


#ifndef WINC1500TASK_H_
#define WINC1500TASK_H_


#include "driver/include/m2m_wifi.h"
#include "socket/include/socket.h"

/** Wi-Fi Settings */
#define MAIN_WLAN_SSID                          "LinkOne" /**< Destination SSID */
#define MAIN_WLAN_AUTH                          M2M_WIFI_SEC_WPA_PSK /**< Security manner */
#define MAIN_WLAN_PSK                           "lightweight" /**< Password for Destination SSID */

#define MAIN_SERVER_PORT_FOR_UDP                (123)
#define MAIN_DEFAULT_ADDRESS                    0xFFFFFFFF /* "255.255.255.255" */
#define MAIN_DEFAULT_PORT                       (6666)
#define MAIN_WIFI_M2M_BUFFER_SIZE               1460

typedef struct  {
	SOCKET sock;
	uint8 u8Msg;
	
	//uint8_t *(socket_action_callback)(void *);
	
	union {
		uint8_t	genericMsg;
		tstrSocketRecvMsg recvMsg;
		tstrSocketAcceptMsg acceptMsg;
		tstrSocketConnectMsg connectMsg;
		tstrSocketListenMsg listenMsg;
		tstrSocketBindMsg bindMsg;
	} u;
	
} socket_event_message_t;

typedef struct {
	
	uint8_t *pu8DomainName;
	uint32_t u32ServerIP;
	
} resolve_event_message_t;

void task_start_wifi(void *pvParameters);
void task_web_socket(void *pvParameters);
void task_web_events(void *pvParameters);



#endif /* WINC1500TASK_H_ */