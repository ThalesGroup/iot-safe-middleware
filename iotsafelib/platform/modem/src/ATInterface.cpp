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

//#define AT_DEBUG

ATInterface::ATInterface(Serial* serial) {
	_serial = serial;
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
	unsigned long int off;
	unsigned long int read;

	off = 0;
	read = 0;
	do {
		if(!_serial->recv(&data[off], 1, &read)) {
			return false;
		}
		if(read) {
			off += read;
			if(data[off - 1] == '\n') {
				break;
			}
		}
	} while(1);

	*len = off;
	return true;
}

bool ATInterface::sendATCSIM(uint8_t* apdu, uint16_t apduLen, uint8_t* response, uint16_t* responseLen) {
	char* buf;
	uint16_t i;
	unsigned long int off, len;

	#ifdef AT_DEBUG
	printf("SND: ");
	for(i=0; i<apduLen; i++) {
		printf("%02X", apdu[i]);
	}
	printf("\n");
	#endif

	off = 0;
	buf = (char*) malloc(537 * sizeof(char));

	off += sprintf(&buf[off], "AT+CSIM=%d,\"", apduLen * 2);
	for(i=0; i<apduLen; i++) {
		off += sprintf(&buf[off], "%02X", apdu[i]);
	}
	off += sprintf(&buf[off], "\"\r\n");

	_serial->send(buf, off, &len);
	memset(buf, 0, sizeof(buf));

	do {
		readLine(buf, &len);
		if(memcmp(buf, "ERROR\r\n", 7) == 0) {
			return false;
		}
	} while((memcmp(buf, "+CSIM: ", 7) != 0));
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

	do {
		readLine(buf, &len);
	} while(memcmp(buf, "OK\r\n", 4) != 0);

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
