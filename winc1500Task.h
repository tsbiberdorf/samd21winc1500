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


#endif /* WINC1500TASK_H_ */