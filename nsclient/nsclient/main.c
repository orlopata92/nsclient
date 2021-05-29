#define _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include "DNSmessage.h"
#include "RFC.h"

#define MAX_LABEL_LENGTH 63

int validationCharInLabel(char nextChar) {
	if (nextChar >= '0' && nextChar <= '9')
		return TRUE;
	if (nextChar >= 'a' && nextChar <= 'z')
		return TRUE;
	if (nextChar >= 'A' && nextChar <= 'Z')
		return TRUE;
	if (nextChar == '-')
		return TRUE;
	return FALSE;
}

int validationNewLabel(char* input, int offset, int newLabelLengthCounter) {
	char nextChar = *(input + offset + 1);
	if (newLabelLengthCounter == 0 || newLabelLengthCounter > MAX_LABEL_LENGTH)
		return FALSE;
	if ((nextChar >= 'a' && nextChar <= 'z') || (nextChar >= 'A' && nextChar <= 'Z'))
		return TRUE;
	return FALSE;
}

int checkInputValidation(char* input) {
	int newLabelLengthCounter = 0;
	int offset = 0;
	char nextChar = *input;
	if (!((nextChar >= 'a' && nextChar <= 'z') || (nextChar >= 'A' && nextChar <= 'Z')))
		return FALSE;
	while (nextChar != NULL && nextChar != '\n') {
		if (nextChar == '.') { // new label
			if (validationNewLabel(input, offset, newLabelLengthCounter) == FALSE) {
				return FALSE;
			}
			offset++;
			newLabelLengthCounter = 1;
		}
		else // not new label
		{
			if (validationCharInLabel(nextChar) == FALSE)
				return FALSE;
			newLabelLengthCounter++;
		}
		offset++;
		nextChar = *(input + offset);
	}
	if (newLabelLengthCounter > MAX_LABEL_LENGTH)
		return FALSE;
	if (((*(input + offset - 1)) >= '0' && (*(input + offset - 1)) <= '9') ||
		((*(input + offset - 1)) >= 'a' && (*(input + offset - 1)) <= 'z') ||
		((*(input + offset - 1)) >= 'A' && (*(input + offset - 1)) <= 'Z'))
			return TRUE;
	return FALSE;
}

void freeAddrList(char*** addrList) {
	int addrListLength = 0;
	while ((*addrList)[addrListLength+1] != NULL) {
		addrListLength++;
	}
	while (addrListLength >= 0) {
		free((*addrList)[addrListLength]);
		addrListLength--;
	}
}

void nsclientUI(char* dnsAddress) {
	int id = 0;
	char* nextInput = NULL;
	while (1) {
		
		nextInput = malloc(sizeof(char) * (MAX_DOMAIN_NAME_LENGTH + 2)); // for NULL + \n
		if (nextInput == NULL)
		{
			printf("malloc() for nextInput failed, error \n");
			return;
		}
		printf("nsclient> ");
		fgets(nextInput, MAX_DOMAIN_NAME_LENGTH + 2, stdin);
		*(nextInput + strlen(nextInput) - 1) = 0;
		if ((strlen(nextInput) == MAX_DOMAIN_NAME_LENGTH) && (nextInput[MAX_DOMAIN_NAME_LENGTH] != '\n')) { // The input is longer than valid domain name
			printf("ERROR: BAD NAME\n");
			gets();
		}
		else if (strcmp("quit", nextInput) == 0) {
			free(nextInput);
			break;
		}
		else if (checkInputValidation(nextInput) == FALSE) {
			printf("ERROR: BAD NAME\n");
		}
		else {
			id++;
			struct hostent* remoteHost = dnsQuery(dnsAddress, nextInput, id);
			if (remoteHost != NULL)
			{
				printf("%s\n", remoteHost->h_addr_list[0]);
				int h_addr_list_index = 0;
				freeAddrList(&(remoteHost->h_addr_list));
				free(remoteHost);
			}
		}
		free(nextInput);
	}
}

int verifyIpFormat(char* ipAddress) {
	int offset = 0, dotsCounter = 0, lastDotAppearance = -1;
	char nextChar = *ipAddress;
	if (nextChar == '.') return FALSE;
	while (nextChar != NULL) {
		if ((!(nextChar >= '0' && nextChar <= '9')) && (nextChar != '.')) return FALSE;
		if (nextChar == '.') {
			dotsCounter++;
			if (offset - lastDotAppearance == 1) return FALSE;
			lastDotAppearance = offset;
		}
		offset++;
		nextChar = *(ipAddress + offset);
	}
	if (lastDotAppearance + 1 == offset)
		return FALSE;
	return (dotsCounter == 3);
}

int main(int argc, char* argv[])
{
	if (argc != 2) {
		printf("Wrong number of input parameters\n");
		return 0;
	}
	else if (verifyIpFormat(argv[1]) == FALSE) {
		printf("ERROR: BAD IP FORMAT\n");
		return 0;
	}
	nsclientUI(argv[1]);
	return 0;
}
