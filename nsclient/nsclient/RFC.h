#ifndef RFC_h
#define RFC_h

// Library Includes.
#include <stdio.h>	
#include <string.h>
#include <stdlib.h>
#include <windows.h>

// Macros & definitions.

#define SERVER_PORT 53
#define MAX_DOMAIN_NAME_LENGTH 253

// Structs

typedef struct _header {
	int id, qr, opcode, aa, tc, rd, ra, z, rcode, qdcount, ancount, nscount, arcount;
}Header;

typedef struct _questionEntry {
	char* qname;
	int qtype, qclass;
}QuestionEntry;

typedef struct _question {
	QuestionEntry* questionEntry;
}Question;

typedef struct _resourceRecord {
	char *name, *rdata;
	int type, class, ttl, rdlength;
}ResourceRecord;

typedef struct _rfc {
	Header header;
	QuestionEntry *question;
	ResourceRecord *answer;
	ResourceRecord *authority;
	ResourceRecord *additional;
}RFC;

// Declarations.
void fillRfcStruct(RFC* p_messageToSend, int id);
void messageToHexa(RFC* messageToSend, char* HexaMessage, char* url);
void buildMessage(char* URL, char* hexaMessage, int id);
char* charToAsciiHexStr(char input);
void urlToHexStr(char* URL);
char intToHexChar(int number);
Header parseHeader(char* response, int* responseOffset);
BOOLEAN isNextMessageSectionAPointer(int byteAsInt);
char* getQnameFromResponse(char* response, int* responseOffset);
int getNextTwoCharsAsInt(char* response, int* responseOffset);
char* getRdDataFromResponse_A(char* response, int rdLength, int* responseOffset);
QuestionEntry* parseQuestion(char* response, int* responseOffset);
ResourceRecord* parseResource(char* response, int numberOfElements, int* responseOffset);
RFC convertResponseToRFC(char* response);

#endif 