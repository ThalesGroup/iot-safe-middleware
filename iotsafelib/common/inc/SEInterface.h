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

#ifndef __SE_INTERFACE_H__
#define __SE_INTERFACE_H__

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#define APDU_CLA_OFFSET 0
#define APDU_INS_OFFSET 1
#define APDU_P1_OFFSET 2
#define APDU_P2_OFFSET 3
#define APDU_LE_OFFSET 4
#define APDU_LC_OFFSET 4
#define APDU_DATA_OFFSET 5

#define APDU_CMD_HEADER_LEN 5
#define MAX_APDU_DATA_LEN 256
#define APDU_CMD_HEADER_LE_LEN 1
#define APDU_RESPONSE_LEN 2
#define APDU_RESPONSE_MAX_PAYLOAD 256
#define APDU_MAX_RESPONSE_LEN (APDU_RESPONSE_LEN + APDU_RESPONSE_MAX_PAYLOAD)


// Error Code
#define ERR_NOERR 0
#define ERR_GENERIC 1
#define ERR_INVALID_LENGTH 2
#define ERR_INCORRECT_DATA 3
#define ERR_INVALID_OPERATION 4
#define ERR_INVALID_RESPONSE 5
#define ERR_INVALID_PARAMETERS 6
#define ERR_OUT_OF_MEMORY 7

#define SW_DATA_AVAILABLE					0x6100
#define SW_NO_INFOMATION_GIVEN					0x6300
#define SW_EXECUTION_OK						0x9000
#define SW_OK							0x9100
#define CONDITION_NOT_SATISFIED 		0x6985


#define SW1_DATA_AVAILABLE					0x61
#define SW1_WRONG_LENGTH_LE					0x6C
#define SW1_DATE_AVAILABLE					0x9F

//#define APDU_DEBUG

#ifdef __cplusplus

/**
 * The class is for APDU commands transmission and response.
 */
class SEInterface
{
public:
	/**
	 * Create an instance of SEInterface
	 *
	 */
	SEInterface(void);
	
	/**
	 * Destrcutor
	 */
	~SEInterface(void){};

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
	int transmit(uint8_t cla, uint8_t ins, uint8_t p1, uint8_t p2);

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
	int transmit(uint8_t cla, uint8_t ins, uint8_t p1, uint8_t p2, uint8_t le);

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
	int transmit(uint8_t cla, uint8_t ins, uint8_t p1, uint8_t p2, const uint8_t *data, uint16_t dataLen);

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
	int transmit(uint8_t cla, uint8_t ins, uint8_t p1, uint8_t p2, const uint8_t *data, uint16_t dataLen, uint8_t le);

	/**
	 * Get status word from the data response received after the last 
	 * successful transmit
	 * 
	 * @return the status word received after the last successful transmit, 
	 *         0 otherwise.
	 */
	uint16_t getStatusWord(void);

	/**
	 * Copy the data response received after the last successful transmit 
	 * 
	 * @param[out]  data pointer to the data buffer for response command 
	 * @return the length of the response, 0 otherwise.
	 */
	uint16_t getResponse(uint8_t *data);

	/**
	 * Returns the length of the data response received after the last 
	 * successful transmit
	 * 
	 * @return the length of the response, 0 otherwise.
	 */
	uint16_t getResponseLength(void);

protected:
	// Low layer implementation to transmit an APDU and retrieve the corresponding APDU Response
	// Returns true in case transmit was successful, false otherwise
	virtual bool transmitApdu(uint8_t *apdu, uint16_t apduLen, uint8_t *response, uint16_t *responseLen) = 0;

private:
	// Internal buffers
	uint8_t _apdu[APDU_CMD_HEADER_LEN + MAX_APDU_DATA_LEN + APDU_CMD_HEADER_LE_LEN];//total 262
	uint16_t _apduLen;
	uint8_t _apduResponse[APDU_MAX_RESPONSE_LEN];//total 258
	uint16_t _apduResponseLen;

	// 'In between' layer implementation which auto handle 6Cxx and 61xx response
	// Stack:
	//  - transmitApdu
	//  - transmit
	//  - transmit (case 1 ... 4)
	// Returns true in case transmit was successful, false otherwise
	bool transmit(void);
};

#else 

typedef struct SEInterface SEInterface; 

bool SEInterface_lock(SEInterface* seiface);
bool SEInterface_unlock(SEInterface* seiface);
		
bool SEInterface_transmit_case1(SEInterface* seiface, uint8_t cla, uint8_t ins, uint8_t p1, uint8_t p2);
bool SEInterface_transmit_case2(SEInterface* seiface, uint8_t cla, uint8_t ins, uint8_t p1, uint8_t p2, uint8_t le);
bool SEInterface_transmit_case3(SEInterface* seiface, uint8_t cla, uint8_t ins, uint8_t p1, uint8_t p2, uint8_t* data, uint16_t data_len);
bool SEInterface_transmit_case4(SEInterface* seiface, uint8_t cla, uint8_t ins, uint8_t p1, uint8_t p2, uint8_t* data, uint16_t data_len, uint8_t le);

uint16_t SEInterface_get_status_word(SEInterface* seiface);
uint16_t SEInterface_get_response(SEInterface* seiface, uint8_t* data);
uint16_t SEInterface_get_response_length(SEInterface* seiface);

#endif

#endif /* __SE_INTERFACE_H__ */
