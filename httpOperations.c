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
#if 0
const char indexPage[]="\
<html>\
        <head>\
        <title> very simple HTML page </title>\
        </head>\
        <body>\
                <h1>heading 1: very simple HTML page</h1>\
                <p>some data </p>\
				<button >Click Me!</button>\
        </body>\
</html>\n";
#else
const char indexPage[]="\
<!DOCTYPE html>\
<html>\
<body>\
<h2>Using the XMLHttpRequest Object</h2>\
<div id=\"demo\">\
<button type=\"button\" onclick=\"loadXMLDoc()\">Change Content</button>\
</div>\
<script>\
function loadXMLDoc() {\
  var xhttp = new XMLHttpRequest();\
  xhttp.onreadystatechange = function() {\
    if (this.readyState == 4 && this.status == 200) {\
      document.getElementById(\"demo\").innerHTML =\
      this.responseText;\
    }\
  };\
  xhttp.open(\"GET\", \"xmlhttp_info.txt\", true);\
  xhttp.send();\
}\
</script>\
</body>\
</html>\n";
#endif

char *MyHeader = "HTTP/1.1 200 OK\r\nConnection: close\r\nContent-Type: text/html\r\nContent-Length: ";

#define MAX_BUFFER_SIZE (1024)
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

	sprintf(SendBuffer,"%s%d\r\n\r\n%s",MyHeader,sizeof(indexPage),indexPage);
	length = strlen(SendBuffer) + 1;
	
	//sprintf(SendBuffer,"%s%d\r\n\r\n%s",MyHeader,sizeof(buttonPage),buttonPage);
	//length = strlen(buttonPage) + 1;
	
	send(tcpSocket,SendBuffer,length,0);
}

