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

#ifndef __AT_INTERFACE_H__
#define __AT_INTERFACE_H__

#include "Serial.h"

class ATInterface {
	public:

		ATInterface(Serial* serial);
		virtual ~ATInterface(void);

		bool open(const char *modem_port);
		void close(void);

		bool sendATCSIM(uint8_t* apdu, uint16_t apduLen, uint8_t* response, uint16_t* responseLen);

	protected:
		bool bytesArray2HexString(uint8_t* bytes, uint16_t bytesLen, uint8_t* hexstr, uint16_t* hexstrLen);
		bool hexString2BytesArray(uint8_t* hexstr, uint16_t hexstrLen, uint8_t* bytes, uint16_t* bytesLen);
		bool readLine(char* data, unsigned long int* len);

	private:
		Serial* _serial;

};

#endif /* __AT_INTERFACE_H__ */
