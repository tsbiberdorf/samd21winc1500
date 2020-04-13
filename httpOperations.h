/*
 * httpOperations.h
 *
 * Created: 4/13/2020 8:45:16 AM
 *  Author: ThreeBoysTech
 */ 


#ifndef HTTPOPERATIONS_H_
#define HTTPOPERATIONS_H_

void setupHttpParserOperations();
void httpOperationsHttpParse(char *dataBuffer,int bytesRecv);
void SendPage(SOCKET tcpSocket);


#endif /* HTTPOPERATIONS_H_ */