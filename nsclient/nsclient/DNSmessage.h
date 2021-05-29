#ifndef DNSmessage_h
#define DNSmessage_h
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma once

// Library Includes.
#include <winsock2.h>
#include <stdio.h>	
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <windows.h>
#include "RFC.h"

#pragma comment(lib, "ws2_32.lib")

// Macros & definitions.

#define TIMEOUT_IN_MILLISECONDS 2000
#define RECV_MESSAGE_LENGTH 131070

typedef enum { TRNS_FAILED, TRNS_SUCCEEDED } TransferResult_t;
// Structs
struct sockaddr_in;
struct hostent;

typedef struct
{
	SOCKET AcceptSocket_p;
	int denied;
	int number;
} THREAD_params_t;

// Declarations.
HOSTENT* dnsQuery(char* dnsServer, char* hostName, int id);
int closesocketSimple(SOCKET m_socket);
int wsaCleanupSimple();
TransferResult_t sendBuffer(char* serverAddress, const char* buffer, int bytesToSend, SOCKET sd);
TransferResult_t sendString(char* serverAddress, const char *str, SOCKET sd, int encodedMessageLength);
TransferResult_t receiveBuffer(char* serverAddress, char* outputBuffer, int bytesToReceive, SOCKET sd);
TransferResult_t receiveString(char* serverAddress, SOCKET sd);
void hexToEncoded(char* hexMessage, char* encodedMessage);
void encodedToHex(char* encodedMessage, int encodedMessageLength, char* hexMessage);
HOSTENT* fillHostent(RFC rfc);
int findIndexOfTypeAAnswer(RFC rfc);
void printErrorByRcode(int rcode);
void fillAddrList(char *** p_h_addr_list, RFC rfc, int indexOfFirstTypeAAnswer);

#endif 