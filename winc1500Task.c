#include "atmel_start.h"
#include "stdio_start.h"
#include "winc_init.h"
#include <string.h>
#include "main.h"
#include "common/include/nm_common.h"
#include "driver/include/m2m_wifi.h"
#include "socket/include/socket.h"
#include "httpOperations.h"

#include <hal_gpio.h>
#include <hal_delay.h>

#include "hal_io.h"
#include "hal_rtos.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

/*
 * WINC1500 Task events
 */
#define WIFI_CB_STATE_CHANGED (1<<0)
#define WIFI_CB_DHCP_CONNECTION (1<<1)
#define WIFI_CB_OTHER (1<<2)

#define SOCKET_CB_EVENT		(1<<3) /**< Socket event has happened */
#define SOCKET_CB_MSG_SENT	(1<<4) /**< echo message has been sent, sockets have been closed */
#define SOCKET_CB_BIND_ERROR	(1<<5) /**< echo message has been sent, sockets have been closed */

static TaskHandle_t xCreatedWiFiTask;

#define TASK_WIFI_STACK_SIZE (1024 / sizeof(portSTACK_TYPE))
#define TASK_WIFI_STACK_PRIORITY (tskIDLE_PRIORITY + 1)

/** Message format definitions. */
typedef struct s_msg_wifi_product {
	uint8_t name[9];
} t_msg_wifi_product;

/** Message format declarations. */
static t_msg_wifi_product msg_wifi_product = {
    .name = MAIN_WIFI_M2M_PRODUCT_NAME,
};

/** Receive buffer definition. */
static uint8_t gau8SocketTestBuffer[MAIN_WIFI_M2M_BUFFER_SIZE];

/** Socket for TCP communication */
static SOCKET tcp_server_socket = -1;
static SOCKET tcp_client_socket = -1;

/** Wi-Fi connection state */
static uint8_t wifi_connected;

static char echoBuffer[64];


/**
 * \brief Callback to get the Data from socket.
 *
 * \param[in] sock socket handler.
 * \param[in] u8Msg socket event type. Possible values are:
 *  - SOCKET_MSG_BIND
 *  - SOCKET_MSG_LISTEN
 *  - SOCKET_MSG_ACCEPT
 *  - SOCKET_MSG_CONNECT
 *  - SOCKET_MSG_RECV
 *  - SOCKET_MSG_SEND
 *  - SOCKET_MSG_SENDTO
 *  - SOCKET_MSG_RECVFROM
 * \param[in] pvMsg is a pointer to message structure. Existing types are:
 *  - tstrSocketBindMsg
 *  - tstrSocketListenMsg
 *  - tstrSocketAcceptMsg
 *  - tstrSocketConnectMsg
 *  - tstrSocketRecvMsg
 */
static void socket_cb(SOCKET sock, uint8_t u8Msg, void *pvMsg)
{
	int idx;
	xTaskNotify(xCreatedWiFiTask,SOCKET_CB_EVENT,eSetBits);

	switch (u8Msg) {
	/* Socket bind */
	case SOCKET_MSG_BIND: {
		tstrSocketBindMsg *pstrBind = (tstrSocketBindMsg *)pvMsg;
		if (pstrBind && pstrBind->status == 0) {
			printf("socket_cb: bind success!\r\n");
			listen(tcp_server_socket, 0);
		} else {
			xTaskNotify(xCreatedWiFiTask,SOCKET_CB_BIND_ERROR,eSetBits);
			printf("socket_cb: bind error!\r\n");
			close(tcp_server_socket);
			tcp_server_socket = -1;
		}
	} break;

	/* Socket listen */
	case SOCKET_MSG_LISTEN: {
		tstrSocketListenMsg *pstrListen = (tstrSocketListenMsg *)pvMsg;
		if (pstrListen && pstrListen->status == 0) {
			printf("socket_cb: listen success!\r\n");
			accept(tcp_server_socket, NULL, NULL);
		} else {

			printf("socket_cb: listen error!\r\n");
			close(tcp_server_socket);
			tcp_server_socket = -1;
		}
	} break;

	/* Connect accept */
	case SOCKET_MSG_ACCEPT: {
		tstrSocketAcceptMsg *pstrAccept = (tstrSocketAcceptMsg *)pvMsg;
		if (pstrAccept) {
			printf("socket_cb: accept success!\r\n");
			accept(tcp_server_socket, NULL, NULL);
			tcp_client_socket = pstrAccept->sock;
			recv(tcp_client_socket, gau8SocketTestBuffer, sizeof(gau8SocketTestBuffer), 0);
		} else {
			printf("socket_cb: accept error!\r\n");
			close(tcp_server_socket);
			tcp_server_socket = -1;
		}
	} break;

	/* Message send */
	case SOCKET_MSG_SEND: {
		xTaskNotify(xCreatedWiFiTask,SOCKET_CB_MSG_SENT,eSetBits);
		printf("socket_cb: send success!\r\n");
		printf("TCP Server Test Complete!\r\n");
		printf("close socket\n");
		close(tcp_client_socket);
		close(tcp_server_socket);
	} break;

	/* Message receive */
	case SOCKET_MSG_RECV: {
		tstrSocketRecvMsg *pstrRecv = (tstrSocketRecvMsg *)pvMsg;
		if (pstrRecv && pstrRecv->s16BufferSize > 0) 
		{
			httpOperationsHttpParse(pstrRecv->pu8Buffer,pstrRecv->s16BufferSize);
		}
		close(tcp_client_socket);
		close(tcp_server_socket);
#if 0
		if (pstrRecv && pstrRecv->s16BufferSize > 0) {
			printf("socket_cb: recv success!\r\n");
			/* echo data back to client */
			for(idx=0;idx<pstrRecv->s16BufferSize;idx++)
			{
				echoBuffer[idx] = pstrRecv->pu8Buffer[idx];
			}
			printf("echo back: %s\n",echoBuffer);
			send(tcp_client_socket,echoBuffer,idx,0);
			//send(tcp_client_socket, &msg_wifi_product, sizeof(t_msg_wifi_product), 0);
		} else {
			printf("socket_cb: recv error!\r\n");
			close(tcp_server_socket);
			tcp_server_socket = -1;
		}

#endif
	}
	break;

	default:
		break;
	}
}

/**
 * \brief Callback to get the Wi-Fi status update.
 *
 * \param[in] u8MsgType type of Wi-Fi notification. Possible types are:
 *  - [M2M_WIFI_RESP_CURRENT_RSSI](@ref M2M_WIFI_RESP_CURRENT_RSSI)
 *  - [M2M_WIFI_RESP_CON_STATE_CHANGED](@ref M2M_WIFI_RESP_CON_STATE_CHANGED)
 *  - [M2M_WIFI_RESP_CONNTION_STATE](@ref M2M_WIFI_RESP_CONNTION_STATE)
 *  - [M2M_WIFI_RESP_SCAN_DONE](@ref M2M_WIFI_RESP_SCAN_DONE)
 *  - [M2M_WIFI_RESP_SCAN_RESULT](@ref M2M_WIFI_RESP_SCAN_RESULT)
 *  - [M2M_WIFI_REQ_WPS](@ref M2M_WIFI_REQ_WPS)
 *  - [M2M_WIFI_RESP_IP_CONFIGURED](@ref M2M_WIFI_RESP_IP_CONFIGURED)
 *  - [M2M_WIFI_RESP_IP_CONFLICT](@ref M2M_WIFI_RESP_IP_CONFLICT)
 *  - [M2M_WIFI_RESP_P2P](@ref M2M_WIFI_RESP_P2P)
 *  - [M2M_WIFI_RESP_AP](@ref M2M_WIFI_RESP_AP)
 *  - [M2M_WIFI_RESP_CLIENT_INFO](@ref M2M_WIFI_RESP_CLIENT_INFO)
 * \param[in] pvMsg A pointer to a buffer containing the notification parameters
 * (if any). It should be casted to the correct data type corresponding to the
 * notification type. Existing types are:
 *  - tstrM2mWifiStateChanged
 *  - tstrM2MWPSInfo
 *  - tstrM2MP2pResp
 *  - tstrM2MAPResp
 *  - tstrM2mScanDone
 *  - tstrM2mWifiscanResult
 */
static void wifi_cb(uint8_t u8MsgType, void *pvMsg)
{

	switch (u8MsgType) {
	case M2M_WIFI_RESP_CON_STATE_CHANGED: 
	{
		xTaskNotify(xCreatedWiFiTask,WIFI_CB_STATE_CHANGED,eSetBits);
		tstrM2mWifiStateChanged *pstrWifiState = (tstrM2mWifiStateChanged *)pvMsg;
		if (pstrWifiState->u8CurrState == M2M_WIFI_CONNECTED) {
			printf("wifi_cb: M2M_WIFI_RESP_CON_STATE_CHANGED: CONNECTED\r\n");
			m2m_wifi_request_dhcp_client();
		} else if (pstrWifiState->u8CurrState == M2M_WIFI_DISCONNECTED) {
			printf("wifi_cb: M2M_WIFI_RESP_CON_STATE_CHANGED: DISCONNECTED\r\n");
			wifi_connected = 0;
			m2m_wifi_connect(
			    (char *)MAIN_WLAN_SSID, sizeof(MAIN_WLAN_SSID), MAIN_WLAN_AUTH, (char *)MAIN_WLAN_PSK, M2M_WIFI_CH_ALL);
		}
	} 
	break;

	case M2M_WIFI_REQ_DHCP_CONF: 
	{
		xTaskNotify(xCreatedWiFiTask,WIFI_CB_DHCP_CONNECTION,eSetBits);
		uint8_t *pu8IPAddress = (uint8_t *)pvMsg;
		wifi_connected        = 1;
		printf("wifi_cb: M2M_WIFI_REQ_DHCP_CONF: IP is %u.%u.%u.%u\r\n",
		       pu8IPAddress[0],
		       pu8IPAddress[1],
		       pu8IPAddress[2],
		       pu8IPAddress[3]);
	}
	break;

	default:
		xTaskNotify(xCreatedWiFiTask,WIFI_CB_OTHER,eSetBits);
		break;
	}
}


static void task_winc1500(void *p)
{
	(void)p;
	tstrWifiInitParam param;
	int8_t            ret = 0;
	uint32_t notifyBits;
	struct sockaddr_in addr;
	BaseType_t xResult;
	int8_t wifiConnectionFlag = 0; // flag to indicate if a wifi connection is made
	int8_t openSocketFlag = 0;
	/* Initialize the BSP. */
	nm_bsp_init();

	/* Initialize socket address structure. */
	addr.sin_family      = AF_INET;
	addr.sin_port        = _htons(HTTP_PORT);
	addr.sin_addr.s_addr = 0;

	setupHttpParserOperations();


	/* Initialize Wi-Fi parameters structure. */
	memset((uint8_t *)&param, 0, sizeof(tstrWifiInitParam));

	/* Initialize Wi-Fi driver with data and status callbacks. */
	param.pfAppWifiCb = wifi_cb;
	wifi_init(&param);

	/* Initialize socket module */
	socketInit();
	registerSocketCallback(socket_cb, NULL);

	/* Connect to router. */
	m2m_wifi_connect(
	(char *)MAIN_WLAN_SSID, sizeof(MAIN_WLAN_SSID), MAIN_WLAN_AUTH, (char *)MAIN_WLAN_PSK, M2M_WIFI_CH_ALL);




	for (;;)
	{
		xResult = xTaskNotifyWait( 0x00,    /* Don't clear bits on entry. */
				0xFFFFFFFF,        /* Clear all bits on exit. */
				&notifyBits, /* Stores the notified value. */
				20 );
		if( xResult == pdPASS )
		{
			if( notifyBits & WIFI_CB_STATE_CHANGED)
			{
				printf("WIFI_CB_STATE_CHANGED\n");
			}
			if( notifyBits & WIFI_CB_DHCP_CONNECTION)
			{
				wifiConnectionFlag = 1;
			}
			if( notifyBits & WIFI_CB_OTHER)
			{
				printf("WIFI_CB_OTHER\n");
			}

			if( wifiConnectionFlag )
			{
				if( !openSocketFlag )
				{
					/* Open TCP server socket */
					if ((tcp_server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
					{
						printf("main: failed to create TCP server socket error!\r\n");
					}
					else
					{
						/* Bind service*/
						openSocketFlag = 1;
						bind(tcp_server_socket, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));
					}
				}

				if( notifyBits & SOCKET_CB_MSG_SENT)
				{
					/* Open TCP server socket */
					if ((tcp_server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
					{
						printf("main: failed to create TCP server socket error!\r\n");
					}
					else
					{
						/* Bind service*/
						openSocketFlag = 1;
						bind(tcp_server_socket, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));
					}
				}
				if( notifyBits & SOCKET_CB_BIND_ERROR)
				{
					/* Open TCP server socket */
					if ((tcp_server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
					{
						printf("main: failed to create TCP server socket error!\r\n");
					}
					else
					{
						/* Bind service*/
						openSocketFlag = 1;
						bind(tcp_server_socket, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));
					}
				}
			}
		}
		/* Handle pending events from network controller. */
		m2m_wifi_handle_events(NULL);
#if 0
		if (wifi_connected == M2M_WIFI_CONNECTED) {
			if (tcp_server_socket < 0) {
				/* Open TCP server socket */
				if ((tcp_server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
					printf("main: failed to create TCP server socket error!\r\n");
					continue;
				}

				/* Bind service*/
				bind(tcp_server_socket, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));
			}
		}
#endif
		//os_sleep(10);
	}
}


/**
 * \brief Create OS task for WiFi Operations
 */
void task_winc1500_create(void)
{
	/* Create task to make led blink */
	if (xTaskCreate(task_winc1500, "WiFi", TASK_WIFI_STACK_SIZE, NULL, TASK_WIFI_STACK_PRIORITY, &xCreatedWiFiTask) != pdPASS) {
		while (1) {
			;
		}
	}
}

