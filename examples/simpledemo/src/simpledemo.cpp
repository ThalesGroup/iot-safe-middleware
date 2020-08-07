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
#include <string.h>
#include "ROT.h"
#include "GenericModem.h"
#include "util.h"
using namespace std;

static const char* modem_port = (const char*)"/dev/ttyUSB2"; //"/dev/ttyACM0";
static GenericModem modem;
static ROT* _rot = NULL;
static RotKeyPair _kp;

#define KEY_ID_LEN          0x14
#define KEY_LABLE_LEN       0x3C
#define IOT_DEBUG printf
#define IOT_ERROR printf

void initialize()
{
    printf("\n-->initialize \n");
    if(!modem.open(modem_port)) {
        printf("\nError modem not found!\n");
        return;
    }

    _rot = new ROT();
    _rot->init(&modem);

    if (!_rot->select(false)) { // true - basic channel, false - new logical channel
        printf("\nError: cannot select applet!\n");
        return;
    }
}

void cleanup()
{
    delete _rot;
    modem.close();
}

void GenerateRandom() {
    printf("\n-->Running AppletTests - GenerateRandom\n");
    uint16_t output_len = 32;
    uint8_t output[32];

    int result = _rot->generateRandom(output, output_len);
    if (result == ERR_NOERR)
    {

        printByteArray("Random", output, output_len);
        printf("\n-->Success \n");
    }
    
}


/**
* The test case always make app crash, commented now
*/
void GenerateKeypair() {
    printf("\n-->Running AppletTests - GenerateKeypair\n");
    
    int result = 0;
    uint8_t container_id[CONTAINER_ID_LENGTH] = {CONTAINER_ID_CLIENT_EPHEMERAL_KEY};
    result = _rot->generateKeyPairByContainerId(container_id, CONTAINER_ID_LENGTH, &_kp);
    if (result == 0) {
        // IOT_DEBUG("\nGet key id len->private: %d, public: %d", _kp->kid);

        uint8_t sharedSecret[0x60];
        uint16_t sharedSecretLen = 0x60;

        uint8_t clEph[CONTAINER_ID_LENGTH] = {CONTAINER_ID_CLIENT_EPHEMERAL_KEY};
        uint8_t svrEph[CONTAINER_ID_LENGTH] = {CONTAINER_ID_SERVER_EPHEMERAL_KEY};
        result = _rot->computeDHforKeypair(clEph, CONTAINER_ID_LENGTH, svrEph, CONTAINER_ID_LENGTH, sharedSecret, &sharedSecretLen);
        if (result == 0) {
            printByteArray("Shared secret", sharedSecret, sharedSecretLen);
            printf("\n-->Success \n");
        } else {
            printf("\nError: Compute Diffie-Hellman Key Pair Failed\n");
        }
    }else {
        printf("\nError: Generate Key Pair Failed\n");
    }
    
}

const uint8_t SERVER_PUBLIC_KEY_TEST[] = {0x49, 0x43, 0x86, 0x41, 0x04, 0x9E, 0x5B, 0x7F, 0x77, 0x8B, 0x79, 0x97, 0x61, 0x7C, 0x89, 0x93, 0xB6, 
0x40, 0x66, 0x08, 0xEF, 0xE2, 0xD3, 0xF2, 0xD6, 0x75, 0x1D, 0x6B, 0x3F, 0x5F, 0x53, 0x20, 0xFF, 0xFD, 0x6A, 0x7D, 0xC9, 0xA8, 0xF6, 0xC5, 0xC1,
0x47, 0x4F, 0x64, 0xD6, 0x68, 0x36, 0x4A, 0xE2, 0xED, 0x8D, 0xC4, 0xC1, 0xC6, 0xC4, 0x76, 0x74, 0x23, 0xB0, 0x13, 0x93, 0x7B, 0x44, 0x99, 0xF5, 
0x2C, 0x4F, 0x89, 0xA0};

void PutServerPublicKey() {
    printf("\n-->Running AppletTests - PutServerPublicKey\n");

    int result = 0;
    uint8_t svrEph[CONTAINER_ID_LENGTH] = {CONTAINER_ID_SERVER_EPHEMERAL_KEY};
    result = _rot->putServerPublicKey(svrEph, CONTAINER_ID_LENGTH, (uint8_t*)SERVER_PUBLIC_KEY_TEST, sizeof(SERVER_PUBLIC_KEY_TEST));
    
    if (result == 0)
        printf("\n-->Success \n");
    else
        printf("\n-->Error: Put Server Public Key Failed \n");
}


void computeSignature() {
    printf("\n-->Running AppletTests - VerifySignature\n");
    uint32_t rot_algo = ROT_ALGO_SHA256_WITH_ECDSA;
    uint8_t keyId[CONTAINER_ID_LENGTH] = {CONTAINER_ID_KEY};
    
    int result = _rot->signInit(keyId, CONTAINER_ID_LENGTH, rot_algo);
    printf("\n-->Running AppletTests - VerifySignature: SignInit\n");
    if (result == 0) {
            
        uint8_t hash[] = {0x77, 0x12, 0xaa, 0xe3, 0xbb, 0xaa, 0xe5, 0xc0, 0x07, 0x47, 0x5a, 0x73, 0x36, 0xf3, 0xdd, 0xe0,
                        0xbc, 0x63, 0x38, 0x0a, 0x34, 0x8d, 0x23, 0x90, 0xc3, 0x51, 0x9e, 0x78, 0x2e, 0x9a, 0x82, 0x98};
        uint16_t hashLen = 0x20;
        uint16_t signLen = 0x60;
        uint8_t signature[0x60];      
        printf("\n-->Running AppletTests - VerifySignature: SignFinal\n");           
        result = _rot->signFinal(hash, hashLen, signature, &signLen);
        if (result == 0) {
            printByteArray("Signature", signature, signLen);
            printf("\n-->Success \n");
        }
        else {
            printf("\nError: signFinal Failed\n");
        }
    }
}


int main(int argc, char *argv[])
{

    initialize();
    GenerateRandom();
    cleanup();
    return 0;
}



