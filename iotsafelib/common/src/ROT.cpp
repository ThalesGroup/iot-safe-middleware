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

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "ROT.h"

/** Constants *******************************************************************/
// AID for IoTSafe Applet
static uint8_t AID[] = { 0xA0, 0x00, 0x00, 0x00, 0x30, 0x53, 0xF1, 0x24, 0x01, 0x77, 0x01, 0x01, 0x49, 0x53, 0x41 };


/**
 * Create an instance of ROT
 */
ROT::ROT(void) : Applet(AID, sizeof(AID)),_keypairs{} 
{

}


/** PRIVATE *******************************************************************/

#define READ_LINE_LEN 255

/**
 * get the size of the container
 * return -1 if error
*/
uint16_t  ROT::getFileLength(
				 const uint8_t *fileId, uint16_t fileIdLen,
				 const uint8_t *fileLbl, uint16_t fileLblLen){
    uint8_t data[50];
    uint16_t result = -1;
    uint16_t cmdLen = fileIdLen + fileLblLen;
    cmdLen += (fileIdLen > 0) ? 2 : 0;  // tag length
    cmdLen += (fileLblLen > 0) ? 2 : 0; // tag length

    if (cmdLen > CMD_MAX_LEN)
    {
        return ERR_INVALID_PARAMETERS;
    }

    uint8_t cmd[CMD_MAX_LEN];     
    uint16_t index = 0;
    uint16_t offset = 0;
    if (fileIdLen > 0)
    {
        if (!fileId)
        {
            return ERR_INVALID_PARAMETERS;
        }
        cmd[index++] = 0x83;
        cmd[index++] = (uint8_t)(fileIdLen & 0xFF);
        memcpy(cmd + index, fileId, fileIdLen);
        index += fileIdLen;
    }        
    if (fileLblLen > 0)
    {
        if (!fileLbl)
        {
            return ERR_INVALID_PARAMETERS;
        }
        cmd[index++] = 0x73;
        cmd[index++] = (uint8_t)(fileLblLen & 0xFF);
        memcpy(cmd + index, fileLbl, fileLblLen);
        index += fileLblLen;
    }

    if (transmit(_channel, 0xCB, 0xC3, 0x00, cmd, cmdLen, 0) &&
        getStatusWord() == SW_EXECUTION_OK)
    {
        uint16_t len = getResponse(data); 
        if(len == 0){
            return -1;
        }
        result = 0;
        //search for tag 20 within the response'
        /**
         * C3 XX
         *      73 XX XX..XX
         *      83 XX XX..XX
         *      60 01 XX
         *      4A 01 XX
         *      21 01 XX
         *      20 02 XX YY
        */
        //starting index is different because some version of applet return C3 while other return the content of C3 only
        if(data[0] == 0xC3){
            index = 2;
        }
        else{
            index = 0;
        }
        
        while(result == 0 && index<len){
            //check if the tag is 0x20
            if(data[index] == 0x20){
                //get the length of the file
                result = (data[index + 2 ]<<8) + data[index + 3];
            }
            else{
                //move the index to the next tag
                index = index + 2 + data[index+1];
            }
        }

        //not found, return 0
        if(result == 0){
            return -1;
        }

    }
    else
    {
        result = -1;
    }

    return result;


}

int ROT::readFile(const uint8_t *path, uint16_t pathLen,
                  const uint8_t *fileId, uint16_t fileIdLen,
                  const uint8_t *fileLbl, uint16_t fileLblLen,
                  uint8_t **data, uint16_t *dataLen)
{
    int result = ERR_GENERIC;
    if (!path || data == nullptr)
    {
        return ERR_INVALID_PARAMETERS;
    }

    if (pathLen > CMD_MAX_LEN) {
        return ERR_INVALID_PARAMETERS;
    }

    if (transmit(_channel, 0xA4, 0x04, 0x00, path, pathLen) &&
        getStatusWord() == SW_EXECUTION_OK)
    {
	uint16_t offset = 0;
        uint8_t toread;

        uint16_t cmdLen = fileIdLen + fileLblLen;
        cmdLen += (fileIdLen > 0) ? 2 : 0;  // tag length
        cmdLen += (fileLblLen > 0) ? 2 : 0; // tag length

        if (cmdLen > CMD_MAX_LEN)
        {
            return ERR_INVALID_PARAMETERS;
        }
        uint8_t cmd[CMD_MAX_LEN];
        uint16_t index = 0;
        if (fileIdLen > 0)
        {
            if (!fileId)
            {
                return ERR_INVALID_PARAMETERS;
            }
            cmd[index++] = 0x83;
            cmd[index++] = (uint8_t)(fileIdLen & 0xFF);
            memcpy(cmd + index, fileId, fileIdLen);
            index += fileIdLen;
        }
        if (fileLblLen > 0)
        {
            if (!fileLbl)
            {
                return ERR_INVALID_PARAMETERS;
            }
            cmd[index++] = 0x73;
            cmd[index++] = (uint8_t)(fileLblLen & 0xFF);
            memcpy(cmd + index, fileLbl, fileLblLen);
            index += fileLblLen;
        }

        //if length is not specified, read the length infortmation of the file
        if(*dataLen == 0){
            printf("Get the length of the container\r\n");
            *dataLen = (uint16_t)getFileLength(fileId, fileIdLen,fileLbl, fileLblLen);
            printf("Container length : %d\r\n", *dataLen);
            if(*dataLen == ((uint16_t) -1)){
                *dataLen = 0;
                return ERR_INVALID_PARAMETERS;
            }

        }

        *data = (uint8_t *)malloc((*dataLen + READ_LINE_LEN) * sizeof(uint8_t));
        if (*data == nullptr) 
        {
	    return ERR_OUT_OF_MEMORY;
        }
        uint8_t p0 = 0;
        uint8_t p1 = 0;

        while (offset < *dataLen)
        {
            // calculate offset again
            p0 = offset >> 8;
            p1 = offset & 0xFF;

            if (cmdLen > CMD_MAX_LEN) {
                return ERR_INVALID_PARAMETERS;
            }

            if (transmit(_channel, 0xB0, p0, p1, cmd, cmdLen, 0) &&
                getStatusWord() == SW_EXECUTION_OK)
            {
                uint16_t len = getResponse(&(*data)[offset]); 
                offset += len;
                result = ERR_NOERR;
                if (len == 0)
                {
                    break;
                }
            }
            else
            {
                result = ERR_INVALID_RESPONSE;
                break;
            }
        }

        if (result == ERR_NOERR)
        {
            // Succeed
            (*data)[offset] = '\0';
            *dataLen += 1;
        }
        else if (*data != nullptr)
        {
            // handle memory
            free(*data);
        }
        return result;
    }
    return result;
}

int ROT::getRandom(uint8_t *data, uint16_t dataLen)
{
    int result = ERR_INVALID_RESPONSE;
    if (dataLen > CMD_MAX_LEN) {
        return ERR_INVALID_PARAMETERS;
    }

    if ((transmit(_channel, 0x84, 0x00, 0x00, dataLen) == true) &&
        (getStatusWord() == SW_EXECUTION_OK || getStatusWord() == SW_NO_INFOMATION_GIVEN) &&
        (getResponse(data) == dataLen))
    {
        return ERR_NOERR;
    }
    return result;
}

int ROT::generateKeypair(const uint8_t *keyId, uint16_t keyIdLen,
                         const uint8_t *keyLbl, uint16_t keyLblLen,
                         uint8_t *privKeyId, uint16_t *privKeyIdLen,
                         uint8_t *pubKeyId, uint16_t *pubKeyIdLen,
                         uint8_t *pubKeyData, uint16_t *pubKeyDataLen)
{
    int result = ERR_GENERIC;
    // Construct command
    uint16_t cmdLen = keyIdLen + keyLblLen;
    cmdLen += (keyIdLen > 0) ? 2 : 0;  // + tag length
    cmdLen += (keyLblLen > 0) ? 2 : 0; // + tag length
    if (cmdLen > CMD_MAX_LEN)
    {
        return ERR_INVALID_PARAMETERS;
    }
    uint8_t cmd[CMD_MAX_LEN];
    uint16_t index = 0;
    if (keyIdLen > 0)
    {
        if (!keyId)
        {
            return ERR_INVALID_PARAMETERS;
        }
        cmd[index++] = 0x84;
        cmd[index++] = keyIdLen;
        memcpy(cmd + index, keyId, keyIdLen);
        index += keyIdLen;
    }
    if (keyLblLen > 0)
    {
        if (!keyLbl)
        {
            return ERR_INVALID_PARAMETERS;
        }
        cmd[index++] = 0x74;
        cmd[index++] = keyLblLen;
        memcpy(cmd + index, keyLbl, keyLblLen);
        index += keyLblLen;
    }

    if (cmdLen > CMD_MAX_LEN) {
        return ERR_INVALID_PARAMETERS;
    }
    
    // Send command
    if (transmit(_channel, 0xB9, 0x00, 0x00, cmd, cmdLen, 0x00) &&
        getStatusWord() == SW_EXECUTION_OK)
    {
        uint16_t dataLen = getResponseLength();
        uint8_t *data = (uint8_t *)malloc(dataLen * sizeof(uint8_t));
        if (data == nullptr) 
        {
            return ERR_OUT_OF_MEMORY;
        }
        getResponse(data);
        uint16_t index = 0;
        // Get private key ID
        if (data[index++] == 0x84)
        {
            *privKeyIdLen = data[index++];
            memcpy(privKeyId, data + index, *privKeyIdLen);
            index += *privKeyIdLen;
        }
        // Get public key ID
        if (data[index++] == 0x85)
        {
            *pubKeyIdLen = data[index++];
            memcpy(pubKeyId, data + index, *pubKeyIdLen);
            index += *pubKeyIdLen;
        }
        // Get public key data
        if (data[index++] == 0x34)
        {
            *pubKeyDataLen = data[index++];
            if (*pubKeyDataLen != 0x45)
            {
                result = ERR_INVALID_RESPONSE;
            }
            memcpy(pubKeyData, data + index, *pubKeyDataLen);
            index += *pubKeyDataLen;
        }
        if (data)
        {
            free(data);
        }
        result = ERR_NOERR;
    }
    return result;
}


int ROT::computeSignatureInit(const uint8_t *keyId, uint16_t keyIdLen,
                              const uint8_t *keyLbl, uint16_t keyLblLen,
                              uint8_t operationMode, uint16_t hashAlgo, uint8_t signAlgo)
{
    int result = ERR_GENERIC;
    // Construct command
    uint16_t cmdLen = keyIdLen + keyLblLen + 3 + 4 + 3; // 3 for TLV tag and len
    cmdLen += (keyIdLen > 0) ? 2 : 0;                   // + tag length
    cmdLen += (keyLblLen > 0) ? 2 : 0;                  // + tag length
    if (cmdLen > CMD_MAX_LEN)
    {
        return ERR_INVALID_LENGTH;
    }
    uint8_t cmd[CMD_MAX_LEN];
    uint16_t index = 0;
    if (keyIdLen > 0)
    {
        if (!keyId)
        {
            return ERR_INVALID_PARAMETERS;
        }
        cmd[index++] = 0x84;
        cmd[index++] = keyIdLen;
        memcpy(cmd + index, keyId, keyIdLen);
        index += keyIdLen;
    }
    if (keyLblLen > 0)
    {
        if (!keyLbl)
        {
            return ERR_INVALID_PARAMETERS;
        }
        cmd[index++] = 0x74;
        cmd[index++] = keyLblLen;
        memcpy(cmd + index, keyLbl, keyLblLen);
        index += keyLblLen;
    }
    //// mode of operation
    cmd[index++] = 0xA1;
    cmd[index++] = 0x01;
    cmd[index++] = operationMode;
    //// hash algorithm
    cmd[index++] = 0x91;
    cmd[index++] = 0x02;
    cmd[index++] = (uint8_t)(hashAlgo >> 8);
    cmd[index++] = (uint8_t)(hashAlgo & 0xFF);
    //// signature algorithm
    cmd[index++] = 0x92;
    cmd[index++] = 0x01;
    cmd[index++] = signAlgo;

    if (cmdLen > CMD_MAX_LEN) {
        return ERR_INVALID_PARAMETERS;
    }

    //TODO:Check
    // Send command
    if (transmit(_channel, 0x2A, 0x00, 0x00, cmd, cmdLen))
    {
        if (getStatusWord() == SW_EXECUTION_OK)
        {
            result = ERR_NOERR;
        }
        else
        {
            result = ERR_INVALID_PARAMETERS;
        }
    }
    return result;
}

uint32_t constructTlvLength(uint8_t *tlv, uint32_t valLen)
{
    uint32_t lenBytesToCopy = 0;
    if (tlv == nullptr)
    {
        // Error: null pointer
        return lenBytesToCopy;
    }
    if (valLen < 0x80)
    {
        tlv[0] = (uint8_t)valLen;
        lenBytesToCopy = 1;
    }
    else if (valLen < 0x100)
    {
        tlv[0] = 0x81;
        tlv[1] = (uint8_t)valLen;
        lenBytesToCopy = 2;
    }
    else if (valLen < 0x10000)
    {
        tlv[0] = 0x82;
        tlv[1] = (uint8_t)(valLen / 0x100);
        tlv[2] = (uint8_t)(valLen % 0x100);
        lenBytesToCopy = 3;
    }
    else if (valLen < 0x1000000)
    {
        tlv[0] = 0x83;
        tlv[1] = (uint8_t)(valLen / 0x10000);
        tlv[2] = (uint8_t)(valLen / 0x100);
        tlv[3] = (uint8_t)(valLen % 0x100);
        lenBytesToCopy = 4;
    }
    return lenBytesToCopy;
}

uint32_t tlvParserLength(uint8_t* tlv, uint32_t* length) {
    uint32_t numOfBytes = 1;
    uint32_t numOfEncodingBytes = 1;
    
    if ((*tlv) == 0x80) {
        // Unknown length
        *length = 0;
        return numOfBytes;
    }
    
    if (((*tlv) & 0x80) != 0) {
        // First bit of byte 1 is on, the total encoding bytes is the diff with 0x80
        numOfEncodingBytes = ((*tlv) - 0x80);
        numOfBytes = numOfEncodingBytes + 1; // Adding the first byte
        tlv = &(tlv[1]);
    }

    // Big-endian encoding assumed (though not specifically documented in ISO)
    uint32_t ret = 0;
    uint32_t curByte = 0;
    for (; curByte < numOfEncodingBytes; curByte++) {
        ret = (ret << 8) + tlv[curByte];
    }
    
    // Return
    *length = ret;
    return numOfBytes;
}

//TODO:Check
int ROT::computeSignatureUpdate(uint8_t operationMode,
                                const uint8_t *data, uint32_t dataLen,
                                const uint8_t *intermediateHash, uint16_t intermediateHashLen,
                                uint32_t hashedBytes,
                                uint8_t *sign, uint16_t *signLen)
{
    int result = ERR_GENERIC;
    uint8_t cmd[CMD_MAX_LEN];
    uint8_t response[512];
    uint16_t index = 0;

    if (data == nullptr) 
    {
        return ERR_INVALID_PARAMETERS;
    }

    if (operationMode == OPERATION_MODE_FULL_TEXT) {
        // Construct command 
        cmd[index++] = 0x9B;
        int tlvLength = constructTlvLength(cmd + index, dataLen);
        index += tlvLength;
        // first command length if the command cannot fit into one command
        bool isSingleCmd = false;
        int firstCmdLen = CMD_MAX_LEN - tlvLength - 1;  
        if (dataLen < firstCmdLen) {
            isSingleCmd = true;
            firstCmdLen = dataLen + 2;  //frist command length if one command can done
        }
        memcpy(cmd+index, data, firstCmdLen);
        uint32_t dataOffset = firstCmdLen;
        if (!isSingleCmd) {
            // multi command process
            while (dataOffset < dataLen) {
                memcpy(cmd, data + dataOffset, CMD_MAX_LEN);
                if(transmit(0x00, 0x2B, 0x00, 0x00, cmd, CMD_MAX_LEN, 0x00)) {
                    if(getStatusWord() != 0x9000) {
                        return ERR_INVALID_RESPONSE;
                    }
                }
                dataOffset += CMD_MAX_LEN;
            }
            // Last command
            index = dataLen - dataOffset;
            memcpy(cmd, data + dataOffset, index);
        } else {
            index = dataLen + 2;  // command length 
        }
    } else if (operationMode == OPERATION_MODE_LAST_BLOCK) {
        if (intermediateHash == nullptr) 
        {
            return ERR_INVALID_PARAMETERS;
        }
	// Construct command 
        //// Last block to hash
        cmd[index++] = 0x9A;
        if (dataLen > 0x80) {
            return ERR_INVALID_LENGTH;
        }
        cmd[index++] = dataLen;
        memcpy(cmd+index, data, dataLen);
        index += dataLen;
        //// Intermediate hash
        cmd[index++] = 0x9C;
        if (intermediateHashLen < 0x20 || intermediateHashLen > 0x40) {
            return ERR_INVALID_LENGTH;
        }
        cmd[index++] = intermediateHashLen;
        memcpy(cmd+index, intermediateHash, intermediateHashLen);
        index += intermediateHashLen;
        //// Number of bytes already hashed
        cmd[index++] = 0x9D;
        cmd[index++] = 0x04;
        cmd[index++] = (uint8_t) (hashedBytes >> 24);
        cmd[index++] = (uint8_t) (hashedBytes >> 16);
        cmd[index++] = (uint8_t) (hashedBytes >> 8);
        cmd[index++] = (uint8_t) (hashedBytes & 0xFF);
    } else if (operationMode == OPERATION_MODE_PADDING) {
        // Construct command 
        cmd[index++] = 0x9E;
        if (dataLen > 0x40) {
            return ERR_INVALID_LENGTH;
        }
        cmd[index++] = dataLen;
        memcpy(cmd+index, data, dataLen);
        index += dataLen;
    } else {
        return ERR_INVALID_OPERATION;
    }

    // Send command
    if(transmit(0x00, 0x2B, 0x80, 0x00, cmd, index, 0x00)) {
        if(getStatusWord() == 0x9000) {
            uint16_t responseLength = getResponse(response);
            uint32_t length = 0;
            if (response[0] != 0x33) {
                return ERR_INVALID_RESPONSE;
            }
            //handle leading 0
            if(response[1] == 0x00){
                //move the remaining data to the front
                memcpy((response + 1),(response + 2),responseLength-2);
            }

            uint8_t numOfBytes = tlvParserLength(response + 1, &length);
            uint8_t* respSign = response + 1 + numOfBytes;
            
            index = 0;
            sign[index++] = 0x30;
            index++;  // length of remaining data
            sign[index++] = 0x02;
            sign[index++] = length / 2; // length of r
            if (respSign[0] & 0x80) {
                sign[index - 1] += 1;   // length of r
                sign[index++] = 0x00;
            }
            memcpy(sign+index, respSign, length / 2);
            index += length / 2;
            sign[index++] = 0x02;
            sign[index++] = length / 2; // length of s
            if (respSign[length / 2] & 0x80) {
                sign[index - 1] += 1;   // length of s
                sign[index++] = 0x00;
            }
            memcpy(sign+index, respSign + length / 2, length / 2);
            index += length / 2;
            sign[1] = index - 2;   // length of remaining data
            *signLen = index;
            result = ERR_NOERR;
        }
    }
    return result;
}

int ROT::computeDH(const uint8_t *privKeyId, uint16_t privKeyIdLen,
                   const uint8_t *pubKeyId, uint16_t pubKeyIdLen,
                   const uint8_t *privLbl, uint16_t privLblLen,
                   const uint8_t *pubLbl, uint16_t pubLblLen,
                   uint8_t *sharedSecret, uint16_t *sharedSecretLen)
{
    bool result = ERR_INVALID_RESPONSE;
    uint8_t cmd[CMD_MAX_LEN];
    uint16_t index = 0;

    if (privKeyIdLen > 0)
    {
        if (!privKeyId)
        {
            return ERR_INVALID_PARAMETERS;
        }
        cmd[index++] = 0x84;
        cmd[index++] = privKeyIdLen;
        memcpy(cmd + index, privKeyId, privKeyIdLen);
        index += privKeyIdLen;
    }
    if (pubKeyIdLen > 0)
    {
        if (!pubKeyId)
        {
            return ERR_INVALID_PARAMETERS;
        }
        cmd[index++] = 0x85;
        cmd[index++] = pubKeyIdLen;
        memcpy(cmd + index, pubKeyId, pubKeyIdLen);
        index += pubKeyIdLen;
    }
    if (privLblLen > 0)
    {
        if (!privLbl)
        {
            return ERR_INVALID_PARAMETERS;
        }
        cmd[index++] = 0x74;
        cmd[index++] = privLblLen;
        memcpy(cmd + index, privLbl, privLblLen);
        index += privLblLen;
    }
    if (pubLblLen > 0)
    {
        if (!pubLbl)
        {
            return ERR_INVALID_PARAMETERS;
        }
        cmd[index++] = 0x75;
        cmd[index++] = pubLblLen;
        memcpy(cmd + index, pubLbl, pubLblLen);
        index += pubLblLen;
    }

    if (index > CMD_MAX_LEN) {
        return ERR_INVALID_PARAMETERS;
    }

    // Send command
    if (transmit(_channel, 0x46, 0x00, 0x00, cmd, index, 0x00) &&
        getStatusWord() == SW_EXECUTION_OK)
    {
        *sharedSecretLen = getResponse(sharedSecret);
        if (*sharedSecretLen > 0)
        {
            return ERR_NOERR;
        }
    }

    return result;
}

int ROT::computePRF(uint8_t mode,
                    const uint8_t *secretId, uint16_t secretIdLen,
                    const uint8_t *secretLbl, uint16_t secretLblLen,
                    const uint8_t *secret, uint16_t secretLen,
                    const uint8_t *pms, uint16_t pmsLen,
                    const uint8_t *lblSeed, uint16_t lblSeedLen,
                    uint8_t *pRandom, uint16_t pRandomLen)
{
    bool result = ERR_INVALID_RESPONSE;
    uint8_t cmd[CMD_MAX_LEN];
    uint16_t index = 0;

    if (secretIdLen > 0)
    {
        if (!secretId)
        {
            return ERR_INVALID_PARAMETERS;
        }
        cmd[index++] = 0x86;
        cmd[index++] = secretIdLen;
        memcpy(cmd + index, secretId, secretIdLen);
        index += secretIdLen;
    }
    if (secretLblLen > 0)
    {
        if (!secretLbl)
        {
            return ERR_INVALID_PARAMETERS;
        }
        cmd[index++] = 0x76;
        cmd[index++] = secretLblLen;
        memcpy(cmd + index, secretLbl, secretLblLen);
        index += secretLblLen;
    }
    if (secretLen > 0)
    {
        if (!secret)
        {
            return ERR_INVALID_PARAMETERS;
        }
        cmd[index++] = 0xD1;
        cmd[index++] = secretLen;
        memcpy(cmd + index, secret, secretLen);
        index += secretLen;
    }
    if (pmsLen > 0)
    {
        if (!pms)
        {
            return ERR_INVALID_PARAMETERS;
        }
        cmd[index++] = 0xD4;
        cmd[index++] = pmsLen;
        memcpy(cmd + index, pms, pmsLen);
        index += pmsLen;
    }
    // Construct lable and seed
    cmd[index++] = 0xD2;
    cmd[index++] = lblSeedLen;
    memcpy(cmd + index, lblSeed, lblSeedLen);
    index += lblSeedLen;
    // construct pseudo random length
    cmd[index++] = 0xD3;
    cmd[index++] = 0x01;
    cmd[index++] = pRandomLen;

    if (index > CMD_MAX_LEN) {
        return ERR_INVALID_PARAMETERS;
    }

    // Send command
    if (transmit(_channel, 0x48, mode, 0x00, cmd, index, 0x00) &&
        getStatusWord() == SW_EXECUTION_OK)
    {
        int length = getResponse(pRandom);
        if (length == pRandomLen)
        {
            return ERR_NOERR;
        }
    }

    return result;
}

int ROT::putPublicKeyInit(const uint8_t *pubKeyId, uint16_t pubKeyIdLen,
                          const uint8_t *pubKeyLbl, uint16_t pubKeyLblLen)
{
    bool result = ERR_INVALID_RESPONSE;
    uint8_t cmd[CMD_MAX_LEN];
    uint16_t index = 0;

    if (pubKeyIdLen > 0)
    {
        if (!pubKeyId)
        {
            return ERR_INVALID_PARAMETERS;
        }
        cmd[index++] = 0x85;
        cmd[index++] = pubKeyIdLen;
        memcpy(cmd + index, pubKeyId, pubKeyIdLen);
        index += pubKeyIdLen;
    }
    if (pubKeyLblLen > 0)
    {
        if (!pubKeyLbl)
        {
            return ERR_INVALID_PARAMETERS;
        }
        cmd[index++] = 0x75;
        cmd[index++] = pubKeyLblLen;
        memcpy(cmd + index, pubKeyLbl, pubKeyLblLen);
        index += pubKeyLblLen;
    }

    if (index > CMD_MAX_LEN) {
        return ERR_INVALID_PARAMETERS;
    }

    // Send command
    if (transmit(_channel, 0x24, 0x00, 0x00, cmd, index, 0x00) &&
        getStatusWord() == SW_EXECUTION_OK)
    {
        return ERR_NOERR;
    }

    return result;
}

int ROT::putPublicKeyUpdate(const uint8_t *pubKey, uint16_t pubKeyLen)
{
    bool result = ERR_INVALID_RESPONSE;
    uint8_t cmd[CMD_MAX_LEN];
    uint16_t index = 0;
    // Null pointer checking
    if (!pubKey)
    {
        return ERR_INVALID_PARAMETERS;
    }

    // Construct command
    cmd[index++] = 0x34;
    uint32_t tlvLength = constructTlvLength(cmd + index, pubKeyLen);
    index += tlvLength;
    // first command length if the command cannot fit into one command
    bool isSingleCmd = false;
    int firstCmdLen = CMD_MAX_LEN - tlvLength - 1;
    if (pubKeyLen < firstCmdLen)
    {
        isSingleCmd = true;
        firstCmdLen = pubKeyLen + 2; //first command length if one command can done
    }
    memcpy(cmd + index, pubKey, firstCmdLen);
    uint32_t dataOffset = firstCmdLen;
    if (!isSingleCmd)
    {
        // multi command process
        while (dataOffset < pubKeyLen)
        {
            memcpy(cmd, pubKey + dataOffset, CMD_MAX_LEN);
            if (transmit(_channel, 0xD8, 0x00, 0x00, cmd, CMD_MAX_LEN, 0x00))
            {
                if (getStatusWord() != SW_EXECUTION_OK)
                {
                    return ERR_INVALID_RESPONSE;
                }
            }
            dataOffset += CMD_MAX_LEN;
        }
        // Last command
        index = pubKeyLen - dataOffset;
        memcpy(cmd, pubKey + dataOffset, index);
    }
    else
    {
        index = pubKeyLen + 2; // command length
    }

    if (index > CMD_MAX_LEN) {
        return ERR_INVALID_PARAMETERS;
    }

    // Send command
    if (transmit(_channel, 0xD8, 0x80, 0x00, cmd, index, 0x00) &&
        getStatusWord() == SW_EXECUTION_OK)
    {
        return ERR_NOERR;
    }

    return result;
}

/** Public *******************************************************************/
int ROT::getCertificateByContainerId(const uint8_t *containerId, uint16_t containerIdLen, uint8_t **cert, uint16_t *certLen)
{
    	printf("getCertificateByContainerId %d\r\n", *containerId);
	return readFile(const_cast<uint8_t *>(AID), sizeof AID, containerId, containerIdLen, nullptr, 0, cert, certLen);
}

int ROT::generateRandom(uint8_t *data, uint16_t dataLen)
{
    return getRandom(data, dataLen);
}


int ROT::signInit(const uint8_t *containerId, uint16_t containerIdLen, uint32_t algorithm)
{
    uint8_t operationMode = OPERATION_MODE_PADDING;

    uint16_t hashAlgo = algorithm >> 8;
    uint8_t signAlgo = algorithm & 0xFF;

    return computeSignatureInit(containerId, containerIdLen,
                                nullptr, 0,
                                operationMode, hashAlgo, signAlgo);
}

int ROT::signFinal(const uint8_t *hash, uint16_t hash_len,
                   uint8_t *signature, uint16_t *signature_len)
{
    uint8_t operationMode = OPERATION_MODE_PADDING;

    return computeSignatureUpdate(operationMode,
                                  hash, hash_len,
                                  nullptr, 0,
                                  0,
                                  signature, signature_len);
}

int ROT::generateKeyPairByContainerId(const uint8_t *containerId, uint16_t containerIdLen, RotKeyPair *kp)
{
    if (kp == nullptr)
    {
        return ERR_INVALID_PARAMETERS;
    }

    RotKeyPair keyPair;
    int result = generateKeypair(
        containerId, containerIdLen,
        nullptr, 0,
        keyPair.priv_key_id, &(keyPair.priv_key_id_len),
        keyPair.pub_key_id, &(keyPair.pub_key_id_len),
        keyPair.pub_key_data, &(keyPair.pub_key_data_len));

    if (result == ERR_NOERR) {
        memcpy(kp, &keyPair, sizeof(RotKeyPair));
    }

    return result;
}

int ROT::putServerPublicKey(const uint8_t *containerId, uint16_t containerIdLen, const uint8_t *pubKey, uint16_t pubKeyLen)
{
    int result = putPublicKeyInit(containerId, containerIdLen, nullptr, 0);
    if (result != ERR_NOERR)
    {
        return result;
    }

    result = putPublicKeyUpdate(pubKey, pubKeyLen);

    return result;
}


int ROT::computeDHforKeypair(
    const uint8_t *clientEphContainerId,
    uint16_t clientEphContainerIdLen,
    const uint8_t *serverEphContainerId,
    uint16_t serverEphContainerIdLen,
    uint8_t *sharedSecret,
    uint16_t *sharedSecretLen)
{
    int result = computeDH(clientEphContainerId, clientEphContainerIdLen,
                           serverEphContainerId, serverEphContainerIdLen,
                           nullptr, 0,
                           nullptr, 0,
                           sharedSecret, sharedSecretLen);
    return result;
}

int ROT::computePRFwithSecret(const uint8_t *secret, uint16_t secretLen,
                              const uint8_t *label, uint16_t labelLen,
                              const uint8_t *seed, uint16_t seedLen,
                              uint8_t *data, uint16_t dataLen)
{
    uint16_t lblSeedLen = labelLen + seedLen;
    uint8_t *lblSeed = (uint8_t *)malloc(lblSeedLen);
    if (lblSeed == nullptr)
    {
        return ERR_OUT_OF_MEMORY;
    }
    memcpy(lblSeed, label, labelLen);
    memcpy(lblSeed + labelLen, seed, seedLen);

    int result = computePRF(PRF_MODE_GENERAL,
                            nullptr, 0,
                            nullptr, 0,
                            secret, secretLen,
                            nullptr, 0,
                            lblSeed, lblSeedLen,
                            data, dataLen);
    free(lblSeed);
    return result;
}

int ROT::computePRFwithPSK(const uint8_t *secretId, uint16_t secretIdLen,
                           const uint8_t *label, uint16_t labelLen,
                           const uint8_t *seed, uint16_t seedLen,
                           uint8_t *data, uint16_t dataLen)
{
    uint16_t lblSeedLen = labelLen + seedLen;
    uint8_t *lblSeed = (uint8_t *)malloc(lblSeedLen);
    if (lblSeed == nullptr)
    {
        return ERR_OUT_OF_MEMORY;
    }
    memcpy(lblSeed, label, labelLen);
    memcpy(lblSeed + labelLen, seed, seedLen);

    int result = computePRF(PRF_MODE_PSK_PLAIN,
                            secretId, secretIdLen,
                            nullptr, 0,
                            nullptr, 0,
                            nullptr, 0,
                            lblSeed, lblSeedLen,
                            data, dataLen);
    free(lblSeed);
    return result;
}

int ROT::computePRFwithPSKECDHE(const uint8_t *secretId, uint16_t secretIdLen,
                                const uint8_t *premaster, uint16_t pmsLen,
                                const uint8_t *label, uint16_t labelLen,
                                const uint8_t *seed, uint16_t seedLen,
                                uint8_t *data, uint16_t dataLen)
{
    uint16_t lblSeedLen = labelLen + seedLen;
    uint8_t *lblSeed = (uint8_t *)malloc(lblSeedLen);
    if (lblSeed == nullptr)
    {
        return ERR_OUT_OF_MEMORY;
    }
    memcpy(lblSeed, label, labelLen);
    memcpy(lblSeed + labelLen, seed, seedLen);

    int result = computePRF(PRF_MODE_PSK_ECDHE,
                            secretId, secretIdLen,
                            nullptr, 0,
                            nullptr, 0,
                            premaster, pmsLen,
                            lblSeed, lblSeedLen,
                            data, dataLen);
    free(lblSeed);
    return result;
}

/** C Accessors	***************************************************************/

extern "C" ROT* ROT_create(void) {
	return new ROT();
}

extern "C" void ROT_destroy(ROT* rot) {
	delete rot;
}

extern "C" int ROT_get_certificate_by_container_id(ROT* rot, const uint8_t *container_id, uint16_t containerIdLen, uint8_t **cert, uint16_t *cert_len) {
	return rot->getCertificateByContainerId(container_id, containerIdLen, cert, cert_len);
}

extern "C" bool ROT_generate_random(ROT* rot, uint8_t* data, uint16_t dataLen) {
    return rot->generateRandom(data, dataLen);
}

extern "C" int ROT_generate_key_pair_by_container_id(ROT* rot, const uint8_t* container_id, uint16_t containerIdLen, RotKeyPair* kp) {
    return rot->generateKeyPairByContainerId(container_id, containerIdLen, kp);
}

extern "C" int ROT_sign_init(ROT* rot, const uint8_t *containerId, uint16_t containerIdLen, uint32_t algorithm) {
	return rot->signInit(containerId, containerIdLen, algorithm);
}

extern "C" int ROT_sign_final(ROT* rot, const uint8_t* hash, uint16_t hash_len, uint8_t* signature, uint16_t* signature_len) {
	return rot->signFinal(hash, hash_len, signature, signature_len);
}

extern "C" int ROT_sign_final_ECDSA(ROT* rot, const uint8_t* hash, uint16_t hash_len, uint8_t* signature, uint16_t* signature_len) {
	return rot->signFinal(hash, hash_len, signature, signature_len);
}


extern "C" int ROT_compute_DH_for_keypair(ROT* rot, const uint8_t *clientEphContainerId, uint16_t clientEphContainerIdLen, 
					const uint8_t *serverEphContainerId, uint16_t serverEphContainerIdLen,
    					uint8_t *sharedSecret, uint16_t *sharedSecretLen) {
    return rot->computeDHforKeypair(clientEphContainerId, clientEphContainerIdLen, serverEphContainerId, serverEphContainerIdLen, sharedSecret, sharedSecretLen);
}

extern "C" int ROT_compute_prf_with_secret(ROT* rot, const uint8_t* secret, uint16_t secretLen, 
                                            const uint8_t* label, uint16_t labelLen,
                                            const uint8_t* seed, uint16_t seedLen,
                                            uint8_t* data, uint16_t dataLen) {
    return rot->computePRFwithSecret(secret, secretLen, label, labelLen, seed, seedLen, data, dataLen);
}

