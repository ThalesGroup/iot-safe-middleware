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

#include <stdio.h>
#include "Applet.h"

/**
 * Create an instance of Applet and settings its corresponding AID.
 *
 * @param[in]  aid the aid buffer
 * @param[in]  aidLen the length of aid
 */
Applet::Applet(const uint8_t *aid, uint16_t aidLen)
{
    _seiface = nullptr;
    _channel = 0;
    _isBasic = false;
    _isSelected = false;
    _aid = const_cast<uint8_t *>(aid);
    _aidLen = aidLen;
}

Applet::~Applet(void)
{
    if (isSelected())
    {
        deselect();
    }
}

/**
 * Configure Applet instance with Secure Element access interface to use
 * to access the targetted applet.
 *
 * @param[in]  seiface a pointer to SEInterface instance
 */
void Applet::init(const SEInterface *seiface)
{
    _seiface = const_cast<SEInterface *>(seiface);
}

/**
 * Close all the sessions
 */
void Applet::closeSessions()
{
    for (uint8_t i = 0; i < 6; i++)
        (this->_seiface)->transmit(this->_channel, 0x2A, 0x01, i, 0x00);
}

/**
 * Check if the applet is selected
 * 
 * @return true in case applet is selected, false otherwise.
 */
bool Applet::isSelected(void)
{
    return _isSelected;
}

/**
 * Select the applet using basic or logical channel.
 * 
 * @param[in]  isBasic set true to use basic logic channel only
 * @return true in case select was successful, false otherwise.
 */
bool Applet::select(bool isBasic /* = true */)
{
    if (_seiface != nullptr)
    {
        if (isBasic)
        {
            _channel = 0;
            if (_seiface->transmit(_channel, 0xA4, 0x04, 0x00, _aid, _aidLen) == ERR_NOERR)
            {
                if ((_seiface->getStatusWord() == SW_EXECUTION_OK) || ((_seiface->getStatusWord() & 0xFF00) == SW_DATA_AVAILABLE) || ((_seiface->getStatusWord() & 0xFF00) == SW_OK))
                {
                    _isSelected = true;
                    _isBasic = true;
                    return true;
                }
            }
        }
        else
        {
            _channel = 1;

            if(_seiface->transmit(0x00, 0x70, 0x00, 0x00, 0x01) == ERR_NOERR) {
                if(_seiface->getStatusWord() == SW_EXECUTION_OK || ((_seiface->getStatusWord() & 0xFF00) == SW_OK)) {
                    _seiface->getResponse(&_channel);
                    if(_seiface->transmit(0x00 | _channel, 0xA4, 0x04, 0x00, _aid, _aidLen) == ERR_NOERR) {
                        if((_seiface->getStatusWord() == SW_EXECUTION_OK) || ((_seiface->getStatusWord() & 0xFF00) == SW_DATA_AVAILABLE)) {
                            _isSelected = true;
                            _isBasic = false;
                            return true;
                        }
                    }
                        
                    _seiface->transmit(0x00, 0x70, 0x80, _channel);
                }
	        }
        }
    }
	
    return false;
}

/**
 * Deselect the applet by closing the channel opened during the
 * select phase.
 * 
 * @return true in case deselect was successful, false otherwise.
 */
bool Applet::deselect(void)
{
    if (_seiface != nullptr)
    {
        if (_isSelected && !_isBasic)
        {
            if (_channel != 0)
            {
                if (_seiface->transmit(_channel, 0x70, 0x80, _channel, 0x00) == ERR_NOERR)
                {
                    if (_seiface->getStatusWord() == SW_EXECUTION_OK)
                    {
                        _isSelected = false;
                    }
                }
            }
        }
        else if (_isSelected && _isBasic)
        {
            _isSelected = false;
        }
    }
    return _isSelected;
}

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
bool Applet::transmit(uint8_t cla, uint8_t ins, uint8_t p1, uint8_t p2) {
	if(_isSelected) {
		return (_seiface->transmit(cla | _channel, ins, p1, p2) == ERR_NOERR);
	}
	return false;
}

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
bool Applet::transmit(uint8_t cla, uint8_t ins, uint8_t p1, uint8_t p2, uint8_t le) {
	if(_isSelected) {
		return (_seiface->transmit(cla | _channel, ins, p1, p2, le) == ERR_NOERR);
	}
	return false;
}

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
bool Applet::transmit(uint8_t cla, uint8_t ins, uint8_t p1, uint8_t p2, const uint8_t *data, uint16_t dataLen)
{
    if (_isSelected)
    {
        return (_seiface->transmit(cla | _channel, ins, p1, p2, data, dataLen) == ERR_NOERR);
    }
    return false;
}

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
bool Applet::transmit(uint8_t cla, uint8_t ins, uint8_t p1, uint8_t p2, const uint8_t *data, uint16_t dataLen, uint8_t le)
{
    if (_isSelected)
    {
        return (_seiface->transmit(cla | _channel, ins, p1, p2, data, dataLen, le) == ERR_NOERR);
    }
    return false;
}

/**
 * Get status word from the data response received after the last 
 * successful transmit
 * 
 * @return the status word received after the last successful transmit, 
 *         0 otherwise.
 */
uint16_t Applet::getStatusWord(void)
{
    uint16_t sw = 0;

    if (_isSelected)
    {
        sw = _seiface->getStatusWord();
    }

    return sw;
}

/**
 * Copy the data response received after the last successful transmit 
 * 
 * @param[out]  data pointer to the data buffer for response command 
 * @return the length of the response, 0 otherwise.
 */
uint16_t Applet::getResponse(uint8_t *data)
{
    uint16_t len = 0;

    if (_isSelected)
    {
        len = _seiface->getResponse(data);
    }

    return len;
}

/**
 * Returns the length of the data response received after the last 
 * successful transmit
 * 
 * @return the length of the response, 0 otherwise.
 */
uint16_t Applet::getResponseLength(void)
{
    uint16_t len = 0;

    if (_isSelected)
    {
        len = _seiface->getResponseLength();
    }

    return len;
}


/** C Accessors	***************************************************************/

extern "C" Applet* Applet_create(uint8_t* aid, uint16_t aid_len) {
	return new Applet(aid, aid_len);
}

extern "C" void Applet_destroy(Applet* applet) {
	delete applet;
}

extern "C" void Applet_init(Applet* applet, SEInterface* seiface) {
	applet->init(seiface);
}

extern "C" bool Applet_is_selected(Applet* applet) {
	return applet->isSelected();
}

extern "C" bool Applet_select(Applet* applet, bool is_basic) {
	return applet->select(is_basic);
}

extern "C" bool Applet_deselect(Applet* applet) {
	return applet->deselect();
}
		
extern "C" bool Applet_transmit_case1(Applet* applet, uint8_t cla, uint8_t ins, uint8_t p1, uint8_t p2) {
	return applet->transmit(cla, ins, p1, p2);
}

extern "C" bool Applet_transmit_case2(Applet* applet, uint8_t cla, uint8_t ins, uint8_t p1, uint8_t p2, uint8_t le) {
	return applet->transmit(cla, ins, p1, p2, le);
}

extern "C" bool Applet_transmit_case3(Applet* applet, uint8_t cla, uint8_t ins, uint8_t p1, uint8_t p2, uint8_t* data, uint16_t data_len) {
	return applet->transmit(cla, ins, p1, p2, data, data_len);
}

extern "C" bool Applet_transmit_case4(Applet* applet, uint8_t cla, uint8_t ins, uint8_t p1, uint8_t p2, uint8_t* data, uint16_t data_len, uint8_t le) {
	return applet->transmit(cla, ins, p1, p2, data, data_len, le);
}

extern "C" uint16_t Applet_get_status_word(Applet* applet) {
	return applet->getStatusWord();
}

extern "C" uint16_t Applet_get_response(Applet* applet, uint8_t* data) {
	return applet->getResponse(data);
}

extern "C" uint16_t Applet_get_response_length(Applet* applet) {
	return applet->getResponseLength();
}
