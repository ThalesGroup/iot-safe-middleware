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

#include "SEInterface.h"
#include <assert.h>

/**
 * Create an instance of SEInterface
 *
 */
SEInterface::SEInterface(void)
{
	_apduLen = 0;
	_apduResponseLen = 0;
}

/**
 * Transmit an APDU case 1 
 * channel.
 * 
 * @param[in]  cla CLA value for APDU command 
 * @param[in]  ins INS value for APDU command 
 * @param[in]  p1 P1 value for APDU command 
 * @param[in]  p2 P2 value for APDU command 
 * @return zero in case transmit was successful, nonzero otherwise.
 */
int SEInterface::transmit(uint8_t cla, uint8_t ins, uint8_t p1, uint8_t p2)
{
	_apdu[APDU_CLA_OFFSET] = cla;
	_apdu[APDU_INS_OFFSET] = ins;
	_apdu[APDU_P1_OFFSET] = p1;
	_apdu[APDU_P2_OFFSET] = p2;
	_apduLen = 4;
	if(transmit()) {
		return ERR_NOERR;
	}
	return ERR_GENERIC;
}

/**
 * Transmit an APDU case 2 
 * channel.
 * 
 * @param[in]  cla CLA value for APDU command 
 * @param[in]  ins INS value for APDU command 
 * @param[in]  p1 P1 value for APDU command 
 * @param[in]  p2 P2 value for APDU command 
 * @param[in]  le Le value for APDU command 
 * @return zero in case transmit was successful, nonzero otherwise.
 */
int SEInterface::transmit(uint8_t cla, uint8_t ins, uint8_t p1, uint8_t p2, uint8_t le)
{
	_apdu[APDU_CLA_OFFSET] = cla;
	_apdu[APDU_INS_OFFSET] = ins;
	_apdu[APDU_P1_OFFSET] = p1;
	_apdu[APDU_P2_OFFSET] = p2;
	_apdu[APDU_LE_OFFSET] = le;
	_apduLen = 5;
	if(transmit()) {
		return ERR_NOERR;
	}
	return ERR_GENERIC;
}

/**
 * Transmit an APDU case 3 
 * channel.
 * 
 * @param[in]  cla CLA value for APDU command 
 * @param[in]  ins INS value for APDU command 
 * @param[in]  p1 P1 value for APDU command 
 * @param[in]  p2 P2 value for APDU command 
 * @param[in]  data pointer to the data buffer for APDU command 
 * @param[in]  dataLen length of the data buffer
 * @return zero in case transmit was successful, nonzero otherwise.
 */
int SEInterface::transmit(uint8_t cla, uint8_t ins, uint8_t p1, uint8_t p2, const uint8_t *data, uint16_t dataLen)
{
	_apdu[APDU_CLA_OFFSET] = cla;
	_apdu[APDU_INS_OFFSET] = ins;
	_apdu[APDU_P1_OFFSET] = p1;
	_apdu[APDU_P2_OFFSET] = p2;
	_apdu[APDU_LC_OFFSET] = dataLen;
	assert(dataLen <= 255);
	if (dataLen > 255) 
	{
		return ERR_INVALID_PARAMETERS;
	}
	memcpy(&_apdu[APDU_DATA_OFFSET], data, dataLen);
	_apduLen = 5 + dataLen;
	if(transmit()) {
		return ERR_NOERR;
	}
	return ERR_GENERIC;
}

/**
 * Transmit an APDU case 4 
 * channel.
 * 
 * @param[in]  cla CLA value for APDU command 
 * @param[in]  ins INS value for APDU command 
 * @param[in]  p1 P1 value for APDU command 
 * @param[in]  p2 P2 value for APDU command 
 * @param[in]  data pointer to the data buffer for APDU command 
 * @param[in]  dataLen length of the data buffer
 * @param[in]  le Le value for APDU command 
 * @return zero in case transmit was successful, nonzero otherwise.
 */
int SEInterface::transmit(uint8_t cla, uint8_t ins, uint8_t p1, uint8_t p2, const uint8_t *data, uint16_t dataLen, uint8_t le)
{
	_apdu[APDU_CLA_OFFSET] = cla;
	_apdu[APDU_INS_OFFSET] = ins;
	_apdu[APDU_P1_OFFSET] = p1;
	_apdu[APDU_P2_OFFSET] = p2;
	_apdu[APDU_LC_OFFSET] = dataLen;
	assert(dataLen <= 255);
	if (dataLen > 255) 
	{
		return ERR_INVALID_PARAMETERS;
	}
	memcpy(&_apdu[APDU_DATA_OFFSET], data, dataLen);
	_apdu[5 + dataLen] = le;
	_apduLen = 5 + dataLen + 1;
	if(transmit()) {
		return ERR_NOERR;
	}
	return ERR_GENERIC;

}

bool SEInterface::transmit(void)
{
	if (transmitApdu(_apdu, _apduLen, _apduResponse, &_apduResponseLen) == false)
	{
		return false;
	}

	if ((_apduResponseLen == 2) && (_apduResponse[0] == SW1_WRONG_LENGTH_LE))
	{
		_apdu[4] = _apduResponse[1];
		return transmit();
	}

	if ((_apduResponseLen == 2) && ((_apduResponse[0] == SW1_DATA_AVAILABLE) || (_apduResponse[0] == SW1_DATE_AVAILABLE)))
	{
		_apdu[0] = 0x00 | (_apdu[0] & 0x03);
		_apdu[1] = 0xC0;
		_apdu[2] = 0x00;
		_apdu[3] = 0x00;
		_apdu[4] = _apduResponse[1];
		_apduLen = 5;
		return transmit();
	}

	return true;
}

/**
 * Get status word from the data response received after the last 
 * successful transmit
 * 
 * @return the status word received after the last successful transmit, 
 *         0 otherwise.
 */
uint16_t SEInterface::getStatusWord(void)
{
	uint16_t sw = 0;

	if (_apduResponseLen >= 2)
	{
		sw = (_apduResponse[_apduResponseLen - 2] << 8) | _apduResponse[_apduResponseLen - 1];
	}

	return sw;
}

/**
 * Copy the data response received after the last successful transmit 
 * 
 * @param[out]  data pointer to the data buffer for response command 
 * @return the length of the response, 0 otherwise.
 */
uint16_t SEInterface::getResponse(uint8_t *data)
{
	uint16_t len = 0;

	if (_apduResponseLen > 2)
	{
		len = _apduResponseLen - 2;
		if (data)
		{
			assert(len <= 255);
			memcpy(data, _apduResponse, len);
		}
	}

	return len;
}

/**
 * Returns the length of the data response received after the last 
 * successful transmit
 * 
 * @return the length of the response, 0 otherwise.
 */
uint16_t SEInterface::getResponseLength(void)
{
	uint16_t len = 0;

	if (_apduResponseLen > 2)
	{
		len = _apduResponseLen - 2;
	}

	return len;
}

/** C Accessors	***************************************************************/

extern "C" bool SEInterface_transmit_case1(SEInterface* seiface, uint8_t cla, uint8_t ins, uint8_t p1, uint8_t p2) {
	return seiface->transmit(cla, ins, p1, p2);
}

extern "C" bool SEInterface_transmit_case2(SEInterface* seiface, uint8_t cla, uint8_t ins, uint8_t p1, uint8_t p2, uint8_t le) {
	return seiface->transmit(cla, ins, p1, p2, le);
}

extern "C" bool SEInterface_transmit_case3(SEInterface* seiface, uint8_t cla, uint8_t ins, uint8_t p1, uint8_t p2, uint8_t* data, uint16_t data_len) {
	return seiface->transmit(cla, ins, p1, p2, data, data_len);
}

extern "C" bool SEInterface_transmit_case4(SEInterface* seiface, uint8_t cla, uint8_t ins, uint8_t p1, uint8_t p2, uint8_t* data, uint16_t data_len, uint8_t le) {
	return seiface->transmit(cla, ins, p1, p2, data, data_len, le);
}

extern "C" uint16_t SEInterface_get_status_word(SEInterface* seiface) {
	return seiface->getStatusWord();
}

extern "C" uint16_t SEInterface_get_response(SEInterface* seiface, uint8_t* data) {
	return seiface->getResponse(data);
}

extern "C" uint16_t SEInterface_get_response_length(SEInterface* seiface) {
	return seiface->getResponseLength();
}
