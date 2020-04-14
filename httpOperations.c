/*
 * httpOperations.c
 *
 * Created: 4/13/2020 8:45:02 AM
 *  Author: ThreeBoysTech
 */ 
#include <stdio.h>
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "common/include/nm_common.h"
#include "driver/include/m2m_wifi.h"
#include "socket/include/socket.h"
#include "http_parser.h"


http_parser_settings tl_HTTPSettings;
http_parser *tl_PtrHTTPParser = NULL;

char indexPage[]="\
<html>\
        <head>\
        <title> very simple HTML page </title>\
        </head>\
        <body>\
                <h1>heading 1: very simple HTML page</h1>\
                <p>some data</p>\
        </body>\
</html>";

char *hello = "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 12\n\nHello world!";

char *MyHeader = "HTTP/1.1 200 OK\r\nConnection: close\r\nContent-Type: text/plain\nContent-Length: ";

#define MAX_BUFFER_SIZE (512)
char SendBuffer[MAX_BUFFER_SIZE];

/**
 * \brief callback for URL parser
 */
static int httpUrlCallback(http_parser *parser, const char *at, size_t length)
{
	return 0;
}


/**
 * \brief callback for header field
 */
static int httpHeaderFieldCallback(http_parser *parser, const char *at, size_t length)
{
	return 0;
}

/**
 * \brief callback for on_status 
 */
static int httpOnStatusCallback(http_parser *parser, const char *at, size_t length)
{
	return 0;
}

/**
 * \brief callback for on body 
 */
static int httpOnBodyCallback(http_parser *parser, const char *at, size_t length)
{
	return 0;
}


/**
 * \brief callback for on_header_value 
 */
static int httpOnHeaderValueCallback(http_parser *parser, const char *at, size_t length)
{
	return 0;
}

/**
 * \brief callback for on_message_begin 
 */
static int httpOnMessageBeginCallback(http_parser *parser)
{
	return 0;
}

/**
 * \brief callback for on_headers_complete 
 */
static int httpOnHeadersCompleteCallback(http_parser *parser)
{
	return 0;
}

/**
 * \brief callback for on_message_complete 
 */
static int httpOnMessaGeCompleteCallback(http_parser *parser)
{
	return 0;
}


void setupHttpParserOperations()
{

	/* setup HTTP parser callbacks */
	tl_HTTPSettings.on_url = httpUrlCallback;
	tl_HTTPSettings.on_header_field = httpHeaderFieldCallback;
	tl_HTTPSettings.on_body = httpOnBodyCallback;
	tl_HTTPSettings.on_message_begin = httpOnMessageBeginCallback;
	tl_HTTPSettings.on_status = httpOnStatusCallback;
	tl_HTTPSettings.on_header_value = httpOnHeaderValueCallback;
	tl_HTTPSettings.on_headers_complete = httpOnHeadersCompleteCallback;
	tl_HTTPSettings.on_message_complete = httpOnMessaGeCompleteCallback;

	tl_PtrHTTPParser = pvPortMalloc(sizeof(http_parser));
	http_parser_init(tl_PtrHTTPParser,HTTP_REQUEST);
}

void httpOperationsHttpParse(char *dataBuffer,int bytesRecv)
{
	uint16_t nparsed;

	nparsed = http_parser_execute(tl_PtrHTTPParser,&tl_HTTPSettings,dataBuffer,bytesRecv);

}

void SendPage(SOCKET tcpSocket)
{
	int length;
	/* need to create header to send with msg */

	sprintf(SendBuffer,"%s%d\n\n%s",MyHeader,sizeof(indexPage),indexPage);
	length = strlen(SendBuffer);
	send(tcpSocket,SendBuffer,length,0);
}

