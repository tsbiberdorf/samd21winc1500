/*
 * httpOperations.c
 *
 * Created: 4/13/2020 8:45:02 AM
 *  Author: ThreeBoysTech
 */ 

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
static void httpHeaderFieldCallback(http_parser *parser, const char *at, size_t length)
{
	return 0;
}

/**
 * \brief callback for on body 
 */
static void httpOnBodyCallback(http_parser *parser, const char *at, size_t length)
{
	return 0;
}


void setupHttpParserOperations()
{

	/* setup HTTP parser callbacks */
	tl_HTTPSettings.on_url = httpUrlCallback;
	tl_HTTPSettings.on_header_field = httpHeaderFieldCallback;
	tl_HTTPSettings.on_body = httpOnBodyCallback;
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
	send(tcpSocket,indexPage,sizeof(indexPage),0);
}

