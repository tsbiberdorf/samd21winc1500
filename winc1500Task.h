/*
 * winc1500Task.h
 *
 * Created: 4/19/2020 9:27:45 AM
 *  Author: ThreeBoysTech
 */ 


#ifndef WINC1500TASK_H_
#define WINC1500TASK_H_


/*
 * WINC1500 Task events
 */
#define WIFI_CB_STATE_CHANGED (1<<0)
#define WIFI_CB_DHCP_CONNECTION (1<<1)
#define WIFI_CB_OTHER (1<<2)

#define SOCKET_CB_EVENT		(1<<3) /**< Socket event has happened */
#define SOCKET_CB_MSG_SENT	(1<<4) /**< echo message has been sent, sockets have been closed */
#define SOCKET_CB_BIND_ERROR	(1<<5) /**< echo message has been sent, sockets have been closed */
#define SEND_HTTP_PAGE (1<<6)
#define STORE_SOCKET_ID (1<<7)

#define TASK_SOCKET_MSG_BIND (1<<8)
#define TASK_SOCKET_MSG_LISTEN (1<<9)
#define TASK_SOCKET_MSG_ACCEPT (1<<10)
#define TASK_SOCKET_MSG_RECV (1<<11)

#define USE_WEP (0)
#define USE_AP_CONNECTION (1)

/** Wi-Fi Settings */
#if USE_AP_CONNECTION
#define MAIN_WLAN_SSID "RelayAP"            /**< name of AP to connect to*/
#else
#define MAIN_WLAN_SSID "TBT"            /**< Destination SSID */
#endif
#define MAIN_WLAN_AUTH M2M_WIFI_SEC_WPA_PSK /**< Security manner */
#define MAIN_WLAN_PSK "TBTP4ssW0rd"            /**< Password for Destination SSID */
#define MAIN_WIFI_M2M_PRODUCT_NAME "NMCTemp"
#define MAIN_WIFI_M2M_SERVER_IP 0xFFFFFFFF /* 255.255.255.255 */
#define MAIN_WIFI_M2M_SERVER_PORT (6666)
#define MAIN_WIFI_M2M_REPORT_INTERVAL (1000)
#define HTTP_PORT (80)

#define MAIN_WIFI_M2M_BUFFER_SIZE 1460


/*
 * settings for AP mode
 */
#define MAIN_WLAN_CHANNEL (6) /* < Channel number */

#if USE_WEP 
#define MAIN_WLAN_WEP_KEY "1234567890"  /* < Security Key in WEP Mode */
#define MAIN_WLAN_WEP_KEY_INDEX (0)
#else
#define MAIN_WLAN_AUTH M2M_WIFI_SEC_OPEN /* < Security manner */

#endif

#endif /* WINC1500TASK_H_ */