#define _CRT_SECURE_NO_WARNINGS

#include "RFC.h"
// encoding and decoding of the message.

void fillRfcStruct(RFC* p_messageToSend, int id)
{
	//Header section.
	p_messageToSend->header.id = id; // This is the first message from the client to the server.
	p_messageToSend->header.qr = 0; // It is query.
	p_messageToSend->header.opcode = 0; // It is a standard query, not inverse.
	p_messageToSend->header.aa = 0; // It isnt a response.
	p_messageToSend->header.tc = 0; // The length isnt greater than that permitted on the transmission channel.
	p_messageToSend->header.rd = 1; // it directs the name server to pursue the query recursively, it is optional.
	p_messageToSend->header.ra = 0; // It isnt a response.															//// maybe unnecessary.
	p_messageToSend->header.z = 0; // Must be zero in all queries and responses.
	p_messageToSend->header.rcode = 0; // It isnt a response.														//// maybe unnecessary.
	p_messageToSend->header.qdcount = 0; // Specifying the number of entries in the question section.
	p_messageToSend->header.ancount = 0; // The number of resource records in the answer section.
	p_messageToSend->header.nscount = 0; // The number of name server resource records in the authority records section.
	p_messageToSend->header.arcount = 0; // The number of resource records in the additional records section.

	//Question section.
	strcpy(p_messageToSend->question->qname, "00"); //The domain name terminates with the zero length octet for the null label of the root.
	p_messageToSend->question->qtype = 1;
	p_messageToSend->question->qclass = 1; // The class is internet=IN=1.

	//Answer section.
	// 0000 no need.

	//Authority section.
	// 0000 no need.

	//Additional section.
	// 0000 no need.
}

void messageToHexa(RFC* p_messageToSend, char* hexaMessage, char* url)
{
	char middleString[5];
	sprintf(middleString, "%04x", p_messageToSend->header.id);
	strcpy(hexaMessage, middleString);
	// Every start of sending package after the id is 0x01000001000000000000.
	strcat(hexaMessage, "01000001000000000000"); // in bin "00000001000000000000000000000001000000000000000000000000000000000000000000000000");
	urlToHexStr(url);
	strcat(hexaMessage, url);
	strcat(hexaMessage, "00");
	sprintf(middleString, "%04x", p_messageToSend->question->qtype);
	strcat(hexaMessage, middleString);
	sprintf(middleString, "%04x", p_messageToSend->question->qclass);
	strcat(hexaMessage, middleString);
	strcat(hexaMessage, "\0");
}

void buildMessage(char* url, char* hexaMessage, int id)
{
	char qname[3];
	Question question;
	QuestionEntry questionEntry;
	RFC message_to_send;
	RFC* p_message_to_send;
	p_message_to_send = &message_to_send;
	p_message_to_send->question = &questionEntry;
	p_message_to_send->question->qname = qname;
	fillRfcStruct(p_message_to_send, id);
	messageToHexa(p_message_to_send, hexaMessage, url);
}

char* charToAsciiHexStr(char input) //need to be used after there is message string of chars.
{
	char hex_number[3];
	int i, remainder, ascii;
	ascii = input; //char to int ascii.
	for (i = 1; i >= 0; i--) //convert int to hex string.
	{
		remainder = ascii % 16;
		if (remainder < 10)
			hex_number[i] = remainder + '0';
		else
			hex_number[i] = remainder + 'a' - 10;
		ascii -= remainder;
		ascii /= 16;
	}
	hex_number[2] = '\0';
	return hex_number;
}

void urlToHexStr(char* URL)
{
	int flag = 0, i, j = 2, k = 0, word_len = 0;
	char* hexURL = NULL;
	// Memory allocation for the temporary hex URL.
	hexURL = (char*)malloc((strlen(URL) * 4 + 1) * sizeof(char)); //*4 because for every word in the url we need at least 4 chars, *4 from hex to bin, +1 for '\0'.
	if (hexURL == NULL)
	{
		printf("malloc() for hexURL failed, error \n");
		return -1;
	}
	strcpy(hexURL, "@@");
	for (i = 0; i < strlen(URL); i++)
	{
		if (flag == 1)
		{
			hexURL[k] = '@';
			hexURL[k + 1] = '@';
			hexURL[k + 2] = '\0';
			flag = 0;
		}
		if (URL[i] == '.')
		{
			if (word_len < 16)
			{
				hexURL[k] = '0';
				hexURL[k + 1] = intToHexChar(word_len);
			}
			else
			{
				hexURL[k + 1] = intToHexChar((word_len % 16)); //find the hex ones.
				hexURL[k] = intToHexChar(((word_len - (word_len % 16)) / 16)); //find the hex tens.
			}
			word_len = 0;
			k = j;
			j += 2;
			flag = 1;
		}
		else
		{
			word_len++;
			strcat(hexURL, charToAsciiHexStr(URL[i]));
			j += 2;
		}
	}
	// For the last word.
	if (word_len < 16)
	{
		hexURL[k] = '0';
		hexURL[k + 1] = intToHexChar(word_len);
	}
	else
	{
		hexURL[k + 1] = intToHexChar((word_len % 16)); //find the hex ones.
		hexURL[k] = intToHexChar(((word_len - (word_len % 16)) / 16)); //find the hex tens.
	}
	strcpy(URL, hexURL);
	free(hexURL);
}

char intToHexChar(int number) //need to be used after there is message string of chars.
{
	char hex_number;
	int i, remainder;
	remainder = number % 16;
	if (remainder < 10)
		hex_number = remainder + '0';
	else
		hex_number = remainder + 'a' - 10;
	return hex_number;
}

Header parseHeader(char* response, int* responseOffset) {
	int tempNumber;
	Header parsedHeader;
	char* tempString = malloc(sizeof(char) * 5);
	if (tempString == NULL)
		printf("malloc() for tempString failed, error \n");
	strncpy_s(tempString, 5, response + *responseOffset, 4);
	parsedHeader.id = (int)strtol(tempString, NULL, 16);
	strncpy_s(tempString, 5, response + *responseOffset + 4, 4);
	tempNumber = (int)strtol(tempString, NULL, 16);
	parsedHeader.rcode = tempNumber & 0xF;
	parsedHeader.z = 0;
	tempNumber = tempNumber >> 7;
	parsedHeader.ra = tempNumber & 0x1;
	tempNumber = tempNumber >> 1;
	parsedHeader.rd = tempNumber & 0x1;
	tempNumber = tempNumber >> 1;
	parsedHeader.tc = tempNumber & 0x1;
	tempNumber = tempNumber >> 1;
	parsedHeader.aa = tempNumber & 0x1;
	tempNumber = tempNumber >> 1;
	parsedHeader.opcode = tempNumber & 0xF;
	tempNumber = tempNumber >> 4;
	parsedHeader.qr = tempNumber;
	strncpy_s(tempString, 5, response + *responseOffset + 8, 4);
	parsedHeader.qdcount = (int)strtol(tempString, NULL, 16);
	strncpy_s(tempString, 5, response + *responseOffset + 12, 4);
	parsedHeader.ancount = (int)strtol(tempString, NULL, 16);
	strncpy_s(tempString, 5, response + *responseOffset + 16, 4);
	parsedHeader.nscount = (int)strtol(tempString, NULL, 16);
	strncpy_s(tempString, 5, response + *responseOffset + 20, 4);
	parsedHeader.arcount = (int)strtol(tempString, NULL, 16);
	*responseOffset = *responseOffset + 24;
	return parsedHeader;
}

BOOLEAN isNextMessageSectionAPointer(int byteAsInt) {
	return ((byteAsInt & 0b11000000) == 0b11000000);
}

char* getQnameFromResponse(char* response, int* responseOffset) {
	char tempStr[10];
	int indexToFill = 0;
	int nextLength;
	char* qnameToReturn = malloc(sizeof(char) * MAX_DOMAIN_NAME_LENGTH);
	if (qnameToReturn == NULL)
	{
		printf("malloc() for qnameToReturn failed, error \n");
		return NULL;
	}
	nextLength = getNextTwoCharsAsInt(response, responseOffset);
	while (nextLength != 0) {
		if (isNextMessageSectionAPointer(nextLength)) {
			*(response + *responseOffset - 2) = ((nextLength >> 4) - 12) + '0';
			*responseOffset = *responseOffset - 2;
			strncpy(tempStr, response + *responseOffset, 4);
			*(tempStr + 5) = NULL;
			int addressOffset = (int)strtol(tempStr, NULL, 16);
			int previousResponseOffset = *responseOffset;
			*responseOffset = addressOffset * 2;
			char* tempQname = getQnameFromResponse(response, responseOffset);
			qnameToReturn = realloc(qnameToReturn, sizeof(qnameToReturn) + strlen(tempQname));
			*(qnameToReturn + indexToFill) = 0;
			strcat(qnameToReturn, tempQname);
			*responseOffset = previousResponseOffset + 4;
			nextLength = 0;
		}
		else {
			while (nextLength > 0) {
				*(qnameToReturn + indexToFill) = getNextTwoCharsAsInt(response, responseOffset);
				indexToFill++;
				nextLength--;
			}
			nextLength = getNextTwoCharsAsInt(response, responseOffset);
			if (nextLength != 0) {
				*(qnameToReturn + indexToFill) = '.';
				indexToFill++;
			}
			*(qnameToReturn + indexToFill) = 0;
		}
	}
	return qnameToReturn;
}

int getNextTwoCharsAsInt(char* response, int* responseOffset) {
	char* nextOctatsAsHex = malloc(sizeof(char) * 3);
	if (nextOctatsAsHex == NULL)
	{
		printf("malloc() for nextOctatsAsHex failed, error \n");
		return NULL;
	}
	nextOctatsAsHex[0] = *(response + *responseOffset);
	nextOctatsAsHex[1] = *(response + *responseOffset + 1);
	nextOctatsAsHex[2] = 0;
	int valueToReturn = (int)strtol(nextOctatsAsHex, NULL, 16);
	free(nextOctatsAsHex);
	*responseOffset = *responseOffset + 2;
	return valueToReturn;
}

char* getRdDataFromResponse_A(char* response, int rdLength, int* responseOffset) {
	char *rdDataToReturn = malloc(sizeof(char) * 16);
	if (rdDataToReturn == NULL)
	{
		printf("malloc() for rdDataToReturn failed, error \n");
		return NULL;
	}
	int indexToFill = 0;
	int nextnumberAsInt;
	char* nextNumberAsString = malloc(sizeof(char) * 4);
	if (nextNumberAsString == NULL)
	{
		printf("malloc() for nextNumberAsString failed, error \n");
		return NULL;
	}
	while (rdLength > 0) {
		nextnumberAsInt = getNextTwoCharsAsInt(response, responseOffset);
		sprintf(nextNumberAsString, "%d", nextnumberAsInt);
		strncpy(rdDataToReturn + indexToFill, nextNumberAsString, strlen(nextNumberAsString));
		indexToFill += strlen(nextNumberAsString);
		rdLength--;
		if (rdLength > 0) {
			*(rdDataToReturn + indexToFill) = '.';
			indexToFill++;
		}
		rdDataToReturn[indexToFill] = NULL;
	}
	return rdDataToReturn;
}

QuestionEntry* parseQuestion(char* response, int* responseOffset) {
	char* tempString = malloc(sizeof(char) * 10);
	if (tempString == NULL)
	{
		printf("malloc() for tempString failed, error \n");
		return NULL;
	}
	int elementsFilled = 0;
	QuestionEntry* parsedQuestion = (QuestionEntry*)malloc(sizeof(QuestionEntry));
	if (parsedQuestion == NULL)
	{
		printf("malloc() for parsedQuestion failed, error \n");
		return NULL;
	}
	(parsedQuestion + elementsFilled)->qname = getQnameFromResponse(response, responseOffset);
	strncpy_s(tempString, 5, response + *responseOffset, 4);
	(parsedQuestion + elementsFilled)->qtype = (int)strtol(tempString, NULL, 16);
	strncpy_s(tempString, 5, response + *responseOffset + 4, 4);
	(parsedQuestion + elementsFilled)->qclass = (int)strtol(tempString, NULL, 16);
	*responseOffset = *responseOffset + 8;
	elementsFilled++;
	free(tempString);
	return parsedQuestion;
}

ResourceRecord* parseResource(char* response, int numberOfElements, int* responseOffset) {
	if (numberOfElements == 0)
		return NULL;
	char* tempString = malloc(sizeof(char) * 24);
	if (tempString == NULL)
	{
		printf("malloc() for tempString failed, error \n");
		return NULL;
	}
	int elementsFilled = 0;
	ResourceRecord* parsedResource = (ResourceRecord*)malloc(sizeof(ResourceRecord) * numberOfElements);
	if (parsedResource == NULL)
	{
		printf("malloc() for parsedResource failed, error \n");
		return NULL;
	}
	while (numberOfElements > 0) {
		(parsedResource + elementsFilled)->name = getQnameFromResponse(response, responseOffset);
		strncpy_s(tempString, 5, response + *responseOffset, 4);
		(parsedResource + elementsFilled)->type = (int)strtol(tempString, NULL, 16);
		strncpy_s(tempString, 5, (response + *responseOffset + 4), 4);
		(parsedResource + elementsFilled)->class = (int)strtol(tempString, NULL, 16);
		strncpy_s(tempString, 9, (response + *responseOffset + 8), 8);
		(parsedResource + elementsFilled)->ttl = (int)strtol(tempString, NULL, 16);
		strncpy_s(tempString, 5, (response + *responseOffset + 16), 4);
		(parsedResource + elementsFilled)->rdlength = (int)strtol(tempString, NULL, 16);
		*responseOffset = *responseOffset + 20;
		(parsedResource + elementsFilled)->rdata = NULL;
		if ((parsedResource + elementsFilled)->type == 1) //TYPE: A
			(parsedResource + elementsFilled)->rdata = getRdDataFromResponse_A(response, (parsedResource + elementsFilled)->rdlength, responseOffset);
		else
			*responseOffset = *responseOffset + 2 * ((parsedResource + elementsFilled)->rdlength);
		numberOfElements--;
		elementsFilled++;
	}
	free(tempString);
	return parsedResource;
}

RFC convertResponseToRFC(char* response) {
	RFC parsedRfc;
	int resourceOffset = 0;
	parsedRfc.header = parseHeader(response, &resourceOffset);
	parsedRfc.question = parseQuestion(response, &resourceOffset);
	parsedRfc.answer = parseResource(response, parsedRfc.header.ancount, &resourceOffset);
	parsedRfc.authority = NULL;
	parsedRfc.additional = NULL;
	return parsedRfc;
}
