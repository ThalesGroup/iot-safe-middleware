/*
 *    Copyright (c) 2019 - 2020, Thales DIS Singapore, Inc
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 *
 */

#include "ATInterface.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>

//#define AT_DEBUG

ATInterface::ATInterface(Serial* serial) {
	_serial = serial;
	_readBufferOffset = 0;
	_readBufferLen = 0;
}

ATInterface::~ATInterface(void) {

}

bool ATInterface::open(const char *modem_port) {
	return _serial->start(modem_port);
}

void ATInterface::close(void) {
	_serial->stop();
}

bool ATInterface::bytesArray2HexString(uint8_t* bytes, uint16_t bytesLen, uint8_t* hexstr, uint16_t* hexstrLen) {
	const uint8_t* hex = (const uint8_t*) "0123456789ABCDEF";
	uint16_t i;

	*hexstrLen = 0;
	for(i = 0; i < bytesLen; i++, *hexstrLen += 2) {
		*hexstr = hex[(bytes[i] >> 4) & 0xF];
		hexstr++;
		*hexstr = hex[bytes[i] & 0xF];
		hexstr++;
	}

	return true;
}

bool ATInterface::hexString2BytesArray(uint8_t* hexstr, uint16_t hexstrLen, uint8_t* bytes, uint16_t* bytesLen) {
	uint8_t d;
	uint16_t i, j;

	*bytesLen = 0;
	for(i = 0; i < hexstrLen; *bytesLen += 1) {
		d = 0;
		for(j = i + 2; i < j; i++) {
			d <<= 4;
			if((hexstr[i] >= '0') && (hexstr[i] <= '9')) {
				d |= hexstr[i] - '0';
			}
			else if((hexstr[i] >= 'a') && (hexstr[i] <= 'f')) {
				d |= hexstr[i] - 'a' + 10;
			}
			else if((hexstr[i] >= 'A') && (hexstr[i] <= 'F')) {
				d |= hexstr[i] - 'A' + 10;
			}
		}
		*bytes = d;
		bytes++;
	}

	return true;
}

bool ATInterface::readLine(char* data, unsigned long int* len) {
	static const size_t MAX_LINE_LENGTH = 537;
	const int timeout_ms = 5000;
	unsigned long int off;
	unsigned long int read;
	bool newLineFound = false;
	int bufferPosition = 0;
	int i;
	off = 0;
	read = 0;
	int index;
	while (_readBufferOffset < _readBufferLen)
	{
		data[off] = _readLineBuffer[_readBufferOffset];
		if(data[off] == '\n')
		{
			*len = off +1;
			_readBufferOffset++; // now offset goes to the character after the \n
			return true;
		}
		off++;
		_readBufferOffset++;
		if (off >= MAX_LINE_LENGTH) {
			printf("readLine: buffer overflow\n");
			return false;
		}
	}
	// All data in the buffer has been copied
	_readBufferLen = 0;
	_readBufferOffset = 0;

	int leftover = 0;

	while (off <= MAX_LINE_LENGTH)
	{
		bufferPosition = 0;
		if (!_serial->recv(&_readLineBuffer[0], NUM_BYTES_TO_READ, &read, timeout_ms)) {
				printf("readLine: receive timeout\n");
				return false;
			}
		while (bufferPosition < read && off < MAX_LINE_LENGTH)
			{
				data[off] = _readLineBuffer[bufferPosition];
				if (data[off] == '\n')
				{
					*len = off + 1;
					leftover = read - (bufferPosition + 1);
					if (leftover > 0)
					{
						memcpy(_readLineBuffer, &_readLineBuffer[bufferPosition + 1], leftover);
					}
					_readBufferLen = leftover;
					_readBufferOffset = 0;
					return true;
				}
				off++;
				bufferPosition++;
			}
			_readBufferLen = 0;
			_readBufferOffset = 0;

	}
	return false;
#ifdef AT_DEBUG
	if (*len > 0)
		printf("readLine: %s\n", data);
	else
		printf("readLine: (empty)\n");
#endif
}

bool ATInterface::sendATCSIM(uint8_t* apdu, uint16_t apduLen, uint8_t* response, uint16_t* responseLen) {
	static const char *ERROR_RESPONSE = "ERROR\r\n";
	static const char *CME_ERROR_RESPONSE = "+CME ERROR";
	static const char *CSIM_RESPONSE = "+CSIM: ";
	static const char *OK_RESPONSE = "OK\r\n";

	char* buf = nullptr;
	uint16_t i = 0;
	unsigned long int off, len = 0;
	bool send_ok = false;
	bool read_ok = false;
	bool read_something = false;
	bool read_csim_response = false;
	bool read_ok_response = false;

	#ifdef AT_DEBUG
	printf("SND: ");
	for(i=0; i<apduLen; i++) {
		printf("%02X", apdu[i]);
	}
	printf("\n");
	#endif

	off = 0;
	buf = (char*) malloc(537 * sizeof(char));

	while (!read_csim_response)
	{
		memset(buf, 0, sizeof(buf));
		off = 0;
		off += sprintf(&buf[off], "AT+CSIM=%d,\"", apduLen * 2);
		for(i=0; i<apduLen; i++) {
			off += sprintf(&buf[off], "%02X", apdu[i]);
		}
		off += sprintf(&buf[off], "\"\r\n");

		send_ok = _serial->send(buf, off, &len);
		if (send_ok)
		{
			memset(buf, 0, 537 * sizeof(char));
			do {
				read_ok = readLine(buf, &len);
				if (!read_ok) {
					printf("Failure reading an AT line\n");
					fflush(stdout);
					free(buf);
					return false;
				}
				if (memcmp(buf, ERROR_RESPONSE, strlen(ERROR_RESPONSE)) == 0) {
					printf("Read back AT line as %s\n", ERROR_RESPONSE);
					fflush(stdout);
					free(buf);
					return false;
				}
				if (memcmp(buf, CME_ERROR_RESPONSE, strlen(CME_ERROR_RESPONSE)) == 0) {
					printf("Read back AT line as %s\n", CME_ERROR_RESPONSE);
					fflush(stdout);
					free(buf);
					return false;
				}
				read_something = (len > 0);
				read_csim_response = (memcmp(buf, CSIM_RESPONSE, strlen(CSIM_RESPONSE)) == 0);
			} while (read_something && !read_csim_response);
		}
		else
		{
			printf("AT command send failed\n");
			fflush(stdout);
			free(buf);
			return false;
		}

		if (!read_csim_response)
		{
			printf("About to re-send AT cmd because did not get CSIM response (possibly due to unsolicited result code)\n");
			sleep(1);
		}
	}

#ifdef AT_DEBUG
	printf("Orig RCV: ");
	printf("%s\n", buf);
	#endif

	off = 7;
	*responseLen = 0;
	while(buf[off] != ',') {
		*responseLen *= 10;
		*responseLen += buf[off] - '0';
		off++;
	}

	while( !(((buf[off] >= '0') && (buf[off] <= '9')) ||
		   ((buf[off] >= 'A') && (buf[off] <= 'F')) ||
		   ((buf[off] >= 'a') && (buf[off] <= 'f'))
		   )) {
		off++;
	}

	hexString2BytesArray((uint8_t*) &buf[off], *responseLen, response, responseLen);
	memset(buf, 0, 537 * sizeof(char));
	do {
		read_ok = readLine(buf, &len);
		if (!read_ok) {
			printf("Failure reading an AT line\n");
			fflush(stdout);
			free(buf);
			return false;
		}
		if (memcmp(buf, ERROR_RESPONSE, strlen(ERROR_RESPONSE)) == 0) {
			printf("Read back AT line as %s\n", ERROR_RESPONSE);
			fflush(stdout);
			free(buf);
			return false;
		}
		if (memcmp(buf, CME_ERROR_RESPONSE, strlen(CME_ERROR_RESPONSE)) == 0) {
			printf("Read back AT line as %s\n", CME_ERROR_RESPONSE);
			fflush(stdout);
			free(buf);
			return false;
		}
		read_something = (len > 0);
		read_ok_response = (memcmp(buf, OK_RESPONSE, strlen(OK_RESPONSE)) == 0);

	} while (read_something && !read_ok_response);

	if (!read_ok_response)
	{
		printf("Failure to read an AT OK response\n");
		fflush(stdout);
		free(buf);
		return false;
	}

	#ifdef AT_DEBUG
	printf("RCV: ");
	for(i=0; i<*responseLen; i++) {
		printf("%02X", response[i]);
	}
	printf("\n");
	#endif

	free(buf);
	return true;
}
