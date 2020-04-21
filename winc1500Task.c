#include "atmel_start.h"
#include "stdio_start.h"
#include "winc_init.h"
#include <string.h>
#include "main.h"
#include "common/include/nm_common.h"
#include "driver/include/m2m_wifi.h"
#include "socket/include/socket.h"
#include "httpOperations.h"
#include "winc1500Task.h"

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

static TaskHandle_t xCreatedWiFiTask;

#define TASK_WIFI_STACK_SIZE (2*1024 / sizeof(portSTACK_TYPE))
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
static SOCKET tcp_send_socket = -1;

/** Wi-Fi connection state */
static uint8_t wifi_connected;

static char echoBuffer[64];
#define NUMBER_SOCKETS (6)
#define CLIENT_SOCKET_LIMIT (6)
static SOCKET tl_ClientSocket[CLIENT_SOCKET_LIMIT];
static uint8_t tl_ClientSocketIdx = 0;

static void PushClientSocket(SOCKET clientSocket )
{
	printf("push socket %d\n",clientSocket);
	tl_ClientSocket[tl_ClientSocketIdx] = clientSocket;
	
	if( (tcp_server_socket >=0) &&(tcp_send_socket<0))
	{
		tcp_send_socket = clientSocket;
		printf("send socket %d\r\n",tcp_send_socket);
	}

	tl_ClientSocketIdx++;
}

static SOCKET SendClientSocket()
{
	if(tl_ClientSocketIdx>0)
	{
		printf("send data to socket %d\n",tl_ClientSocket[tl_ClientSocketIdx-1]);
		return tl_ClientSocket[tl_ClientSocketIdx-1];
	}
	else
	{
		return -1;
	}
}
static void PopClientSockets()
{
	int idx;
	for( idx=0; idx <tl_ClientSocketIdx; idx++ )
	{
		printf("close socket %d\n",tl_ClientSocket[idx]);
		close(tl_ClientSocket[idx]);
		tl_ClientSocket[idx] = -1;
	}
	tcp_send_socket = -1;
	tl_ClientSocketIdx = 0;
}

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
	BaseType_t xHigherPriorityTaskWoken;
	xHigherPriorityTaskWoken = pdFALSE;
	
	xTaskNotifyFromISR(xCreatedWiFiTask,SOCKET_CB_EVENT,eSetBits,&xHigherPriorityTaskWoken);

	switch (u8Msg) {
	/* Socket bind */
	case SOCKET_MSG_BIND: {
		tstrSocketBindMsg *pstrBind = (tstrSocketBindMsg *)pvMsg;
		if (pstrBind && pstrBind->status == 0) {
			listen(tcp_server_socket, 0);
			xTaskNotifyFromISR(xCreatedWiFiTask,TASK_SOCKET_MSG_BIND,eSetBits,&xHigherPriorityTaskWoken);

		} else {
			xTaskNotifyFromISR(xCreatedWiFiTask,SOCKET_CB_BIND_ERROR,eSetBits,&xHigherPriorityTaskWoken);
			close(tcp_server_socket);
			printf("socket_cb: bind error! s:%d\r\n",tcp_server_socket);
			tcp_server_socket = -1;
		}
	} break;

	/* Socket listen */
	case SOCKET_MSG_LISTEN: {
		tstrSocketListenMsg *pstrListen = (tstrSocketListenMsg *)pvMsg;
		if (pstrListen && pstrListen->status == 0) {
			accept(tcp_server_socket, NULL, NULL);
			xTaskNotifyFromISR(xCreatedWiFiTask,TASK_SOCKET_MSG_LISTEN,eSetBits,&xHigherPriorityTaskWoken);
		} else {

			close(tcp_server_socket);
			printf("socket_cb: listen error! s:%d\r\n",tcp_server_socket);
			tcp_server_socket = -1;
		}
	} break;

	/* Connect accept */
	case SOCKET_MSG_ACCEPT: {
		tstrSocketAcceptMsg *pstrAccept = (tstrSocketAcceptMsg *)pvMsg;
		if (pstrAccept) {
			accept(tcp_server_socket, NULL, NULL);
			tcp_client_socket = pstrAccept->sock;
			PushClientSocket(tcp_client_socket);
			
			recv(tcp_client_socket, gau8SocketTestBuffer, sizeof(gau8SocketTestBuffer), 0);
			xTaskNotifyFromISR(xCreatedWiFiTask,TASK_SOCKET_MSG_ACCEPT,eSetBits,&xHigherPriorityTaskWoken);

		}
		else {
			close(tcp_server_socket);
			printf("socket_cb: accept error! s:%d\r\n",tcp_server_socket);
			tcp_server_socket = -1;
		}
	} break;

	/* Message send */
	case SOCKET_MSG_SEND: {
		//close(tcp_client_socket);
		close(tcp_server_socket);
		tcp_server_socket = -1;
		//printf("close socket\n");
		xTaskNotifyFromISR(xCreatedWiFiTask,SOCKET_CB_MSG_SENT,eSetBits,&xHigherPriorityTaskWoken);
	} break;

	/* Message receive */
	case SOCKET_MSG_RECV: {
		tstrSocketRecvMsg *pstrRecv = (tstrSocketRecvMsg *)pvMsg;
		if (pstrRecv && pstrRecv->s16BufferSize > 0) 
		{
			httpOperationsHttpParse(pstrRecv->pu8Buffer,pstrRecv->s16BufferSize);
		}
		xTaskNotifyFromISR(xCreatedWiFiTask,TASK_SOCKET_MSG_RECV,eSetBits,&xHigherPriorityTaskWoken);

/*		
		if(tcp_send_socket>= 0)
		{
			SendPage(tcp_send_socket);
		}
	*/	

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

	BaseType_t xHigherPriorityTaskWoken;
	xHigherPriorityTaskWoken = pdFALSE;

	switch (u8MsgType) {
	case M2M_WIFI_RESP_CON_STATE_CHANGED: 
	{
		xTaskNotifyFromISR(xCreatedWiFiTask,WIFI_CB_STATE_CHANGED,eSetBits,&xHigherPriorityTaskWoken);
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
		xTaskNotifyFromISR(xCreatedWiFiTask,WIFI_CB_DHCP_CONNECTION,eSetBits,&xHigherPriorityTaskWoken);
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
		xTaskNotifyFromISR(xCreatedWiFiTask,WIFI_CB_OTHER,eSetBits,&xHigherPriorityTaskWoken);
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
	
	tstrM2MAPConfig   strM2MAPConfig;

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

#if USE_AP_CONNECTION // if set to 1 if act as an access point
	/* Initialize AP mode parameters structure with SSID, channel and OPEN security type. */
	memset(&strM2MAPConfig, 0x00, sizeof(tstrM2MAPConfig));
	strcpy((char *)&strM2MAPConfig.au8SSID, MAIN_WLAN_SSID);
	strM2MAPConfig.u8ListenChannel = MAIN_WLAN_CHANNEL;
	strM2MAPConfig.u8SecType       = MAIN_WLAN_AUTH;

	strM2MAPConfig.au8DHCPServerIP[0] = 192;
	strM2MAPConfig.au8DHCPServerIP[1] = 168;
	strM2MAPConfig.au8DHCPServerIP[2] = 100;
	strM2MAPConfig.au8DHCPServerIP[3] = 1;

#if USE_WEP
	strcpy((char *)&strM2MAPConfig.au8WepKey, MAIN_WLAN_WEP_KEY);
	strM2MAPConfig.u8KeySz   = strlen(MAIN_WLAN_WEP_KEY);
	strM2MAPConfig.u8KeyIndx = MAIN_WLAN_WEP_KEY_INDEX;
#endif

	/* Bring up AP mode with parameters structure. */
	ret = m2m_wifi_enable_ap(&strM2MAPConfig);
	if (M2M_SUCCESS != ret) {
		printf("main: m2m_wifi_enable_ap call error!\r\n");
		while (1) {
		}
	}

	printf("AP mode started. You can connect to %s.\r\n", (char *)MAIN_WLAN_SSID);

#else // if USE_AP_CONNECTION is set to 0 if we are to connect to a router
	/* Connect to router. */
	m2m_wifi_connect(
	(char *)MAIN_WLAN_SSID, sizeof(MAIN_WLAN_SSID), MAIN_WLAN_AUTH, (char *)MAIN_WLAN_PSK, M2M_WIFI_CH_ALL);
#endif



	for (;;)
	{
		xResult = xTaskNotifyWait( 0x00,    /* Don't clear bits on entry. */
				0xFFFFFFFF,        /* Clear all bits on exit. */
				&notifyBits, /* Stores the notified value. */
				2 );
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

			if( notifyBits & TASK_SOCKET_MSG_BIND)
			{
				printf("socket_cb: bind success! s:%d\r\n",tcp_server_socket);
			}
			if( notifyBits & TASK_SOCKET_MSG_LISTEN)
			{
				printf("socket_cb: listen success! s:%d\r\n",tcp_server_socket);
			}
			if(notifyBits & TASK_SOCKET_MSG_ACCEPT)
			{
				printf("socket_cb: accept success! c:%d\r\n",tcp_client_socket);
			}

			if(notifyBits & TASK_SOCKET_MSG_ACCEPT)
			{
				printf("socket_cb: accept success! c:%d\r\n",tcp_client_socket);
			}
			
			if( notifyBits & HTTP_httpUrlCallback)
			{
				printf("HTTP_httpUrlCallback\r\n");
			}
			if( notifyBits & HTTP_httpHeaderFieldCallback)
			{
				printf("HTTP_httpHeaderFieldCallback\r\n");
			}
			if( notifyBits & HTTP_httpOnStatusCallback)
			{
				printf("HTTP_httpOnStatusCallback\r\n");
			}
			if( notifyBits & HTTP_httpOnBodyCallback)
			{
				printf("HTTP_httpOnBodyCallback\r\n");
			}
			if( notifyBits & HTTP_httpOnHeaderValueCallback)
			{
				printf("HTTP_httpOnHeaderValueCallback\r\n");
			}
			if( notifyBits & HTTP_httpOnMessageBeginCallback)
			{
				printf("HTTP_httpOnMessageBeginCallback\r\n");
			}
			if( notifyBits & HTTP_httpOnHeadersCompleteCallback)
			{
				printf("HTTP_httpOnHeadersCompleteCallback\r\n");
			}
			if( notifyBits & HTTP_httpOnMessaGeCompleteCallback)
			{
				printf("HTTP_httpOnMessaGeCompleteCallback\r\n");
				extern void PrintHTTPCBMsg();
				PrintHTTPCBMsg();
			}
			if( notifyBits & TASK_SOCKET_MSG_RECV)
			{
				if(tcp_send_socket>= 0)
				{
					SendPage(tcp_send_socket);
				}
			}
					

			if( wifiConnectionFlag )
			{
				if( !openSocketFlag )
				{
					/* Open TCP server socket */
					PopClientSockets();
					if ((tcp_server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
					{
						printf("main: failed to create TCP server socket error!\r\n");
					}
					else
					{
						/* Bind service*/
						openSocketFlag = 1;
						printf("bind to socket %d\n",tcp_server_socket);
						bind(tcp_server_socket, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));
					}
				}

				if( notifyBits & SOCKET_CB_MSG_SENT)
				{
					printf("socket_cb: send success! c:%d s:%d\r\n",tcp_client_socket,tcp_server_socket);
					printf("TCP Server Test Complete!\r\n");

					/* Open TCP server socket */
					PopClientSockets();
					if ((tcp_server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
					{
						printf("main: failed to create TCP server socket error!\r\n");
					}
					else
					{
						/* Bind service*/
						openSocketFlag = 1;
						printf("bind to socket s:%d\n",tcp_server_socket);
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
						printf("bind to socket s:%d\n",tcp_server_socket);
						bind(tcp_server_socket, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));
					}
				}
			}
		}
		/* Handle pending events from network controller. */
		m2m_wifi_handle_events(NULL);
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

TaskHandle_t GetWiFiTaskId()
{
	return xCreatedWiFiTask;
}
