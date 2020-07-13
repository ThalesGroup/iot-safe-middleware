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
#ifndef __LSERIAL_H__
#define __LSERIAL_H__

#include "Serial.h"

class LSerial: public Serial {
	public:
		// Create an instance of WSerial.
		LSerial(void);
		~LSerial(void);

		bool start(const char *modem_port);
		bool send(char* data, unsigned long  int toWrite, unsigned long  int* written);
		bool recv(char* data, unsigned long int toRead, unsigned long  int* read);
		bool stop(void);

	private:
		int32_t m_uart;

};

#endif /* __LSERIAL_H__ */
