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

// this is the start of file Applet.h

#ifndef __APPLET_H__
#define __APPLET_H__

#include "SEInterface.h"

#define USE_ROT_APPLET 1

#ifdef __cplusplus

/**
 * The class is for Applet basic operations, select, deselect and command transmission. 
 * A wrapper layer on top of SEInterface class. 
 */
class Applet {
	public:
		/**
	 * Create an instance of Applet and settings its corresponding AID.
	 *
	 * @param[in]  aid the aid buffer
	 * @param[in]  aidLen the length of aid
	 */
	Applet(const uint8_t *aid, uint16_t aidLen);
	
	/**
	 * Destrcutor
	 */
	~Applet(void);
	
	/**
	 * Configure Applet instance with Secure Element access interface to use
	 * to access the targetted applet.
	 *
	 * @param[in]  seiface a pointer to SEInterface instance
	 */
	void init(const SEInterface *se);

	/**
	 * Close all the sessions
	 */
	void closeSessions();
	
	/**
	 * Check if the applet is selected
	 * 
	 * @return true in case applet is selected, false otherwise.
	 */
	bool isSelected(void);
	/**
	 * Select the applet using basic or logical channel.
	 * 
	 * @param[in]  isBasic set true to use basic logic channel only
	 * @return true in case select was successful, false otherwise.
	 */
	bool select(bool isBasic = true);

	/**
	 * Deselect the applet by closing the channel opened during the
	 * select phase.
	 * 
	 * @return true in case deselect was successful, false otherwise.
	 */
	bool deselect(void);
	/**
	 * Transmit an APDU case 1 to the applet through the corresponding 
	 * channel.
	 * 
	 * @param[in]  cla CLA value for APDU command 
	 * @param[in]  ins INS value for APDU command 
	 * @param[in]  p1 P1 value for APDU command 
	 * @param[in]  p2 P2 value for APDU command 
	 * @return true in case transmit was successful, false otherwise.
	 */
	bool transmit(uint8_t cla, uint8_t ins, uint8_t p1, uint8_t p2);

	/**
	 * Transmit an APDU case 2 to the applet through the corresponding 
	 * channel.
	 * 
	 * @param[in]  cla CLA value for APDU command 
	 * @param[in]  ins INS value for APDU command 
	 * @param[in]  p1 P1 value for APDU command 
	 * @param[in]  p2 P2 value for APDU command 
	 * @param[in]  le Le value for APDU command 
	 * @return true in case transmit was successful, false otherwise.
	 */
	bool transmit(uint8_t cla, uint8_t ins, uint8_t p1, uint8_t p2, uint8_t le);

	/**
	 * Transmit an APDU case 3 to the applet through the corresponding 
	 * channel.
	 * 
	 * @param[in]  cla CLA value for APDU command 
	 * @param[in]  ins INS value for APDU command 
	 * @param[in]  p1 P1 value for APDU command 
	 * @param[in]  p2 P2 value for APDU command 
	 * @param[in]  data pointer to the data buffer for APDU command 
	 * @param[in]  dataLen length of the data buffer
	 * @return true in case transmit was successful, false otherwise.
	 */
	bool transmit(uint8_t cla, uint8_t ins, uint8_t p1, uint8_t p2, const uint8_t *data, uint16_t dataLen);

	/**
	 * Transmit an APDU case 4 to the applet through the corresponding 
	 * channel.
	 * 
	 * @param[in]  cla CLA value for APDU command 
	 * @param[in]  ins INS value for APDU command 
	 * @param[in]  p1 P1 value for APDU command 
	 * @param[in]  p2 P2 value for APDU command 
	 * @param[in]  data pointer to the data buffer for APDU command 
	 * @param[in]  dataLen length of the data buffer
	 * @param[in]  le Le value for APDU command 
	 * @return true in case transmit was successful, false otherwise.
	 */
	bool transmit(uint8_t cla, uint8_t ins, uint8_t p1, uint8_t p2, const uint8_t *data, uint16_t dataLen, uint8_t le);

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
	SEInterface *_seiface; // Secure Element on which is installed the targetted applet.
	uint8_t _channel;	   // channel value
	bool _isSelected;	   // flag to indicate if the applet is currently selected.
	bool _isBasic;		   // flag to indicate if the applet has been selected through basic channel.
	uint8_t *_aid;		   // Applet's AID
	uint16_t _aidLen;	   // Applet's AID length
};

#else 
	
typedef struct Applet Applet;

Applet* Applet_create(uint8_t* aid, uint16_t aid_len);
void Applet_destroy(Applet* applet);

void Applet_init(Applet* applet, SEInterface* seiface);
bool Applet_is_selected(Applet* applet);
bool Applet_select(Applet* applet, bool is_basic);
bool Applet_deselect(Applet* applet);
		
bool Applet_transmit_case1(Applet* applet, uint8_t cla, uint8_t ins, uint8_t p1, uint8_t p2);
bool Applet_transmit_case2(Applet* applet, uint8_t cla, uint8_t ins, uint8_t p1, uint8_t p2, uint8_t le);
bool Applet_transmit_case3(Applet* applet, uint8_t cla, uint8_t ins, uint8_t p1, uint8_t p2, uint8_t* data, uint16_t data_len);
bool Applet_transmit_case4(Applet* applet, uint8_t cla, uint8_t ins, uint8_t p1, uint8_t p2, uint8_t* data, uint16_t data_len, uint8_t le);

uint16_t Applet_get_status_word(Applet* applet);
uint16_t Applet_get_response(Applet* applet, uint8_t* data);
uint16_t Applet_get_response_length(Applet* applet);
		
#endif

#endif /* __APPLET_H__ */

// end of file Applet.h
