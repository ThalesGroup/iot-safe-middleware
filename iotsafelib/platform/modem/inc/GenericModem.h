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

#ifndef __GENERIC_MODEM_H__
#define __GENERIC_MODEM_H__

#include "ATInterface.h"
#include "SEInterface.h"

class GenericModem: public SEInterface {
	public:
		// Create an instance of Cinterion Modem.
		GenericModem(void);
		~GenericModem(void);

		bool open(const char *modem_port) {
			return _at.open(modem_port);
		}

		void close(void) {
			_at.close();
		}

	//protected:

		bool transmitApdu(uint8_t* apdu, uint16_t apduLen, uint8_t* response, uint16_t* responseLen);

	private:
		ATInterface _at;
};

#endif /* __GENERIC_MODEM_H__ */
