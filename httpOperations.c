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
#include "winc1500Task.h"
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
  xhttp.open(\"GET\", \"buttonPressed.txt\", true);\
  xhttp.send();\
}\
</script>\
</body>\
</html>\n";
#endif

const char buttonText[]="\
<!DOCTYPE html>\
<html>\
<body>\
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
	xhttp.open(\"GET\", \"buttonPressed.txt\", true);\
	xhttp.send();\
}\
</script>\
</body>\
</html>\n";


char *tl_HTTPPrintPage = indexPage;
const char ButtonPressedText[] = "buttonPressed.txt";
char *tl_HTTPMsg = NULL;
char *MyHeader = "HTTP/1.1 200 OK\r\nConnection: close\r\nContent-Type: text/html\r\nContent-Length: ";

#define MAX_BUFFER_SIZE (1024)
char SendBuffer[MAX_BUFFER_SIZE];

/**
 * \brief callback for URL parser
 */
static int httpUrlCallback(http_parser *parser, const char *at, size_t length)
{
	tl_HTTPMsg=at;
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
	BaseType_t xHigherPriorityTaskWoken;
	xHigherPriorityTaskWoken = pdFALSE;
	xTaskNotifyFromISR(GetWiFiTaskId(),HTTP_httpOnMessaGeCompleteCallback,eSetBits,&xHigherPriorityTaskWoken);

	return 0;
}


static void ParseHTTPMsg(char *ptrHTTPMsg)
{
	int idx,matchIdx;
	int firstMardIdx = 0;
int secondMardIdx = 0;
	
	/* first find the first '/' */	
for(idx=0; idx<MAX_BUFFER_SIZE; idx++)
	{
		if( ptrHTTPMsg[idx] == '/')
		{
		firstMardIdx = idx;
			break;
		}
	}
	
	/*
 * now try to match the possible additional requests
	 */
	for(idx=firstMardIdx+1,matchIdx = 0; idx<(MAX_BUFFER_SIZE-firstMardIdx); idx++)
{
		if( ptrHTTPMsg[idx] == ButtonPressedText[matchIdx])
		{
			matchIdx++;
		}
	if( ptrHTTPMsg[idx] == '/')
		{
			secondMardIdx = idx;
			break;
		}
}
	if(matchIdx == (sizeof(ButtonPressedText)-1))
	{
		printf("button pressed text found\r\n");
		tl_HTTPPrintPage = buttonText;
	}
if( secondMardIdx < MAX_BUFFER_SIZE)
	{
		ptrHTTPMsg[secondMardIdx+1] = 0x0;
	}
}

void PrintHTTPCBMsg()
{
	if(tl_HTTPMsg)
	{
		ParseHTTPMsg(tl_HTTPMsg);
		printf("%s\r\n",tl_HTTPMsg);
		tl_HTTPMsg=NULL;
	}
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

	sprintf(SendBuffer,"%s%d\r\n\r\n%s",MyHeader,strlen(tl_HTTPPrintPage),tl_HTTPPrintPage);
	length = strlen(SendBuffer) + 1;
	
	//sprintf(SendBuffer,"%s%d\r\n\r\n%s",MyHeader,sizeof(buttonPage),buttonPage);
	//length = strlen(buttonPage) + 1;
	
	send(tcpSocket,SendBuffer,length,0);
}

