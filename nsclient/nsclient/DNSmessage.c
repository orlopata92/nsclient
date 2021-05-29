#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include "DNSmessage.h"

SOCKET m_socket;
char acceptedStr[RECV_MESSAGE_LENGTH];
int acceptedStrLength = 0;

HOSTENT* dnsQuery(char* dnsServer, char* hostName, int id)
{
	struct hostent* remoteHost = NULL;
	int iResult, free_flag = 0;
	char* hexa_message = NULL;
	char* encodedMessage = NULL;
	char send_message_id[5], receive_message_id[5];
	fd_set readfds;
	struct timeval tv;
	DWORD wait_code;
	TransferResult_t SendRes, RecvRes;
	// initialize windows networking
	WSADATA wsaData;

	// Memory allocation for the hexa_message to send.
	hexa_message = (char*)malloc((strlen(hostName) * 4 + 32 + 1) * sizeof(char) * 4); //*4 because for every word in the hostName we need at least 4 chars, *4 from hex to bin, +32 for the headers of the message, +1 for '\0'.
	if (hexa_message == NULL)
	{
		printf("malloc() for hexa_message failed, error \n");
		return NULL;
	}

	// Create from the hostName a message.
	buildMessage(hostName, hexa_message, id);
	strncpy(send_message_id, hexa_message, 4);
	send_message_id[4] = '\0';
	// Transfrom the hexa message to bin message.
	encodedMessage = (char*)malloc((strlen(hostName) * 4 + 32 + 1) * sizeof(char) * 4); //*4 because for every word in the hostName we need at least 4 chars, *4 from hex to bin, +32 for the headers of the message, +1 for '\0'.
	if (encodedMessage == NULL)
	{
		printf("malloc() for encodedMessage failed, error \n");
		return NULL;
	}
	hexToEncoded(hexa_message, encodedMessage);
	int encodedMessageLength = strlen(hexa_message) / 2;
	free(hexa_message);

	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != NO_ERROR)
	{
		printf("Error at WSAStartup()\n");
		return NULL;
	}

	// create the socket that will listen for incoming connections
	m_socket = socket(AF_INET, SOCK_DGRAM, 0);
	if (m_socket == INVALID_SOCKET) {
		printf("Error at socket(): %ld\n", WSAGetLastError());
		goto End_running1;
	}

	//send the message.
	SendRes = sendString(dnsServer, encodedMessage, m_socket, encodedMessageLength);
	if (SendRes != TRNS_SUCCEEDED)
		goto End_running2;
	free(encodedMessage);
	free_flag = 1;

	// Select function.
	tv.tv_sec = 2;
	tv.tv_usec = 0;
	FD_ZERO(&readfds);
	FD_SET(m_socket, &readfds);
	select(m_socket + 1, &readfds, NULL, NULL, &tv);
	if (!(FD_ISSET(m_socket, &readfds)))
	{
		printf("TIMEOUT\n");
		goto End_running2;
	}
	//recive message.
	RecvRes = receiveString(dnsServer, m_socket);
	if (RecvRes != TRNS_SUCCEEDED) 
		goto End_running2;

	//Check the message id.
	strncpy(receive_message_id, acceptedStr, 4);
	receive_message_id[4] = '\0';
	if (strcmp(send_message_id, receive_message_id) != 0)
	{
		printf("TIMEOUT\n");
		goto End_running2;
	}
	RFC rfc = convertResponseToRFC(acceptedStr);
	if (rfc.header.rcode == 0) {
		remoteHost = fillHostent(rfc);
	}
	else {
		printErrorByRcode(rfc.header.rcode);
	}
	// AFTER THE PROGRAM SHOWS THE OUTPUT TO THE USER
	free(rfc.question);
	free(rfc.answer);
	free(rfc.authority);
	free(rfc.additional);

End_running2:
	if (closesocketSimple(m_socket) != 0)
		return NULL;

End_running1:
	if (wsaCleanupSimple() != 0)
		return NULL;
	if (free_flag == 0)
		free(encodedMessage);
	return remoteHost;

}

void printErrorByRcode(int rcode) {
	if (rcode == 2) {
		printf("ERROR: SERVER FAILURE\n");
	}
	else if (rcode == 3) {
		printf("ERROR: NONEXISTENT\n");
	}
	else if (rcode == 4) {
		printf("ERROR: NOT IMPLEMNTED\n");
	}
	else if (rcode == 5) {
		printf("ERROR: REFUSED\n");
	}
}

int closesocketSimple(SOCKET m_socket)
{
	int return_value = 0;

	return_value = closesocket(m_socket);
	// Check for errors to ensure that the socket is a valid socket.
	if (return_value != 0)
	{
		printf("Error at socket(): %ld\n", WSAGetLastError());
		return_value = -1;
	}
	else
		return_value = 0;
	return return_value;
}

int wsaCleanupSimple()
{
	int return_value = 0;

	return_value = WSACleanup();
	// Check for errors to ensure that the socket is a valid socket.
	if (return_value != 0)
	{
		printf("Error at socket(): %ld\n", WSAGetLastError());
		return_value = -1;
	}
	else
		return_value = 0;
	return return_value;
}

/**
 * sendBuffer() uses a socket to send a buffer.
 *
 * Accepts:
 * -------
 * serverAddress - the ip address of the server
 * Buffer - the buffer containing the data to be sent.
 * BytesToSend - the number of bytes from the Buffer to send.
 * sd - the socket used for communication.
 *
 * Returns:
 * -------
 * TRNS_SUCCEEDED - if sending succeeded
 * TRNS_FAILED - otherwise
 */
TransferResult_t sendBuffer(char* serverAddress, const char* Buffer, int BytesToSend, SOCKET sd)
{
	const char* CurPlacePtr = Buffer;
	int BytesTransferred;
	if (Buffer == NULL)
		return TRNS_FAILED;

	//Create a sockaddr_in object clientService and set values.
	struct sockaddr_in remote_addr;
	remote_addr.sin_family = AF_INET;
	remote_addr.sin_addr.s_addr = inet_addr(serverAddress);
	remote_addr.sin_port = htons(SERVER_PORT);
	BytesTransferred = sendto(sd, CurPlacePtr, BytesToSend, 0, (SOCKADDR *)(&remote_addr), sizeof(remote_addr));
	if (BytesTransferred == SOCKET_ERROR)
	{
		printf("send() failed, error %d\n", WSAGetLastError());
		return TRNS_FAILED;
	}
	return TRNS_SUCCEEDED;
}

/**
 * sendString() uses a socket to send a string.
 * serverAddress - the ip address of the server
 * Str - the string to send.
 * sd - the socket used for communication.
 * encodedMessageLength - the encoded message's length
 */
TransferResult_t sendString(char* serverAddress, const char *Str, SOCKET sd, int encodedMessageLength)
{
	/* Send the the request to the server on socket sd */
	TransferResult_t SendRes;
	if (Str == NULL)
		return TRNS_FAILED;

	SendRes = sendBuffer(serverAddress, (const char *)(Str), encodedMessageLength, sd);

	return SendRes;
}

/**
 * Accepts:
 * -------
 * receiveBuffer() uses a socket to receive a buffer.
 * serverAddress - the ip address of the server
 * OutputBuffer - pointer to a buffer into which data will be written
 * OutputBufferSize - size in bytes of Output Buffer
 * BytesReceivedPtr - output parameter. if function returns TRNS_SUCCEEDED, then this
 *					  will point at an int containing the number of bytes received.
 * sd - the socket used for communication.
 *
 * Returns:
 * -------
 * TRNS_SUCCEEDED - if receiving succeeded
 * TRNS_FAILED - otherwise
 */
TransferResult_t receiveBuffer(char* serverAddress, char* OutputBuffer, int BytesToReceive, SOCKET sd)
{
	char* CurPlacePtr = OutputBuffer;
	int BytesJustTransferred;
	int RemainingBytesToReceive = BytesToReceive;

	if (OutputBuffer == NULL)
		return TRNS_FAILED;

	//Create a sockaddr_in object clientService and set values.
	struct sockaddr_in remote_addr;
	remote_addr.sin_family = AF_INET;
	remote_addr.sin_addr.s_addr = inet_addr(serverAddress);
	remote_addr.sin_port = htons(SERVER_PORT);

	struct sockaddr_in SenderAddr;
	int SenderAddrSize = sizeof(SenderAddr);
	BytesJustTransferred = recvfrom(sd, CurPlacePtr, RemainingBytesToReceive, 0, (SOCKADDR *)(&SenderAddr), &SenderAddrSize);
	acceptedStrLength = BytesJustTransferred;
 	if (BytesJustTransferred == SOCKET_ERROR)
	{
		printf("recv() failed, error %d\n", WSAGetLastError());
		return TRNS_FAILED;
	}
	return TRNS_SUCCEEDED;
}

/**
 * receiveString() uses a socket to receive a string, and stores it in dynamic memory.
 *
 * Accepts:
 * -------
 * serverAddress - the ip address of the server
 * sd - the socket used for communication.
 *
 * Returns:
 * -------
 * TRNS_SUCCEEDED - if receiving and memory allocation succeeded
 * TRNS_FAILED - otherwise
 */
TransferResult_t receiveString(char* serverAddress, SOCKET sd)
{
	/* Recv the the request to the server on socket sd */
	TransferResult_t RecvRes;
	char* strBuffer = NULL;

	strBuffer = (char*)malloc(RECV_MESSAGE_LENGTH * sizeof(char));

	if (strBuffer == NULL)
	{
		printf("malloc() for strBuffer failed, error \n");
		return TRNS_FAILED;
	}


	RecvRes = receiveBuffer(serverAddress, strBuffer, RECV_MESSAGE_LENGTH, sd);
	strBuffer[acceptedStrLength] = 0;

	if (RecvRes == TRNS_SUCCEEDED)
	{
		encodedToHex(strBuffer, acceptedStrLength, acceptedStr);
	}
	free(strBuffer);
	return RecvRes;
}

void hexToEncoded(char* hexMessage, char* encodedMessage) {
	int hexOffset = 0, encodedOffset = 0;
	char* nextByte = malloc(sizeof(char) * 3);
	if (nextByte == NULL)
	{
		printf("malloc() for nextByte failed, error \n");
		return NULL;
	}
	*(nextByte + 2) = 0;
	while (*(hexMessage + hexOffset)) {
		*(nextByte) = *(hexMessage + hexOffset);
		*(nextByte + 1) = *(hexMessage + hexOffset + 1);
		*(encodedMessage + encodedOffset) = (int)strtol(nextByte, NULL, 16);
		encodedOffset++;
		hexOffset = hexOffset + 2;
	}
	*(encodedMessage + encodedOffset) = NULL;
	free(nextByte);
}

void encodedToHex(char* encodedMessage, int encodedMessageLength, char* hexMessage) {
	int hexOffset = 0, encodedOffset = 0;
	while (encodedOffset < encodedMessageLength) {
		sprintf(hexMessage + hexOffset, "%02x", (unsigned char)(*(encodedMessage + encodedOffset)));
		encodedOffset++;
		hexOffset = hexOffset + 2;
		*(hexMessage + hexOffset) = NULL;
	}
	*(hexMessage + hexOffset) = NULL;
}

int findIndexOfTypeAAnswer(RFC rfc) {
	int answerClass, answerType;
 	int iNumberOfAnswers = 0;
	while (iNumberOfAnswers < rfc.header.ancount) {
		answerClass = (rfc.answer)[iNumberOfAnswers].class;
		answerType = (rfc.answer)[iNumberOfAnswers].type;
		if (answerClass == 1 && answerType == 1)
			return iNumberOfAnswers;
		iNumberOfAnswers++;
	}
	return -1;
}

void fillAddrList(char *** p_h_addr_list, RFC rfc, int indexOfFirstTypeAAnswer) {
	int answerClass, answerType;
	int index = indexOfFirstTypeAAnswer;
	int h_addr_list_counter = 0;
	while (index < rfc.header.ancount) {
		answerClass = (rfc.answer)[index].class;
		answerType = (rfc.answer)[index].type;
		if (answerClass == 1 && answerType == 1)
		{
			(*p_h_addr_list)[h_addr_list_counter] = (rfc.answer)[index].rdata;
			h_addr_list_counter++;
		}
		index++;
	}
	(*p_h_addr_list)[h_addr_list_counter] = NULL;
}

HOSTENT* fillHostent(RFC rfc)
{
	struct hostent* remoteHost = NULL;
	int indexOfTypeAAnswer = findIndexOfTypeAAnswer(rfc);
	if (indexOfTypeAAnswer == -1)
	{
		printf("J");
		return NULL;
	}
	remoteHost = (HOSTENT*)malloc(sizeof(HOSTENT));
	if (remoteHost == NULL)
	{
		printf("malloc() for remoteHost failed, error \n");
		return NULL;
	}
	remoteHost->h_name = NULL;
	remoteHost->h_aliases = NULL;
	remoteHost->h_addrtype = AF_INET;
	remoteHost->h_length = 4;
	remoteHost->h_addr_list = malloc(sizeof(char*) * rfc.header.ancount);
	if (remoteHost->h_addr_list == NULL)
	{
		printf("malloc() for remoteHost failed, error \n");
		return NULL;
	}
	fillAddrList(&(remoteHost->h_addr_list), rfc, indexOfTypeAAnswer);
	return remoteHost;
}

