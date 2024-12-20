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
#include "CppUTest/TestHarness.h"

#include "../include/rot_tests_helper.h"
#include "ROT.h"
#include "GenericModem.h"

using namespace std;
static const char* modem_port=(const char*)"/dev/ttyACM0";
static GenericModem modem;
static ROT* _rot = NULL;
static RotKeyPair _kp;

#define KEY_ID_LEN          0x14
#define KEY_LABLE_LEN       0x3C
#define IOT_DEBUG printf
#define IOT_ERROR printf

TEST_GROUP(AppletTests)
{
    void setup()
    {
        IOT_DEBUG("\n-->Setup AppletTests \n");
        if(!modem.open(modem_port)) {
            IOT_ERROR("\nError modem not found!\n");
            return;
        }

        _rot = new ROT();
        _rot->init(&modem);

        if (!_rot->select(true)) {
            IOT_ERROR("\nError: cannot select applet!\n");
            return;
        }
    }

    void teardown()
    {
        delete _rot;
        modem.close();
    }
};

TEST(AppletTests, GenerateRandom) {
    IOT_DEBUG("\n-->Running AppletTests - GenerateRandom\n");
    uint16_t output_len = 32;
    uint8_t output[32];

    int result = _rot->generateRandom(output, output_len);
    CHECK_TRUE(result == ERR_NOERR);

    printByteArray("Random", output, output_len);

    IOT_DEBUG("\n-->Success \n");
}

const uint8_t TEST_SECRET1[] = {
    0xC0, 0x48, 0x5B, 0x05, 0x48, 0x63, 0xF6, 0xDF, 0xA4, 0x58, 0x78, 0x97, 0x68, 0xB9, 0x03, 0x5C, 
    0x6C, 0xAC, 0xB1, 0x60, 0xDD, 0x1A, 0x01, 0x83, 0x48, 0x11, 0xF5, 0x78, 0x33, 0x09, 0xDB, 0x81};

const uint8_t TEST_SEED1[] = {
    0xCE, 0x15, 0xB4, 0x4D, 0x68, 0x44, 0x0B, 0x65, 0x26, 0x24, 0x24, 0x4D, 0xB8, 0xD2, 0xFB, 0x7D, 
    0xD6, 0x01, 0xC7, 0x59, 0xE9, 0xEB, 0x62, 0x7F, 0xB1, 0x05, 0x29, 0xAB, 0x0D, 0xB7, 0x60, 0x49};

const uint8_t EXPECTED_RESULT1[] = {
    0xa1, 0xf0, 0x2a, 0x76, 0x6d, 0xa0, 0x4c, 0x8f, 0xd8, 0x3a, 0x40, 0xe3, 0x58, 0x26, 0x75, 0x28, 
    0xf8, 0xd8, 0x15, 0xc4, 0x8e, 0x3d, 0xc3, 0x53, 0xd5, 0x5f, 0xe3, 0x86, 0x1c, 0x35, 0x06, 0x22, 
    0xc7, 0x8b, 0x48, 0xfe, 0x6f, 0x49, 0xf6, 0xe1, 0x3e, 0x1b, 0x6b, 0x17, 0x5c, 0xfe, 0x59, 0xfb};
    
const uint8_t TEST_SEED2[] = {
    0xcb, 0x52, 0xe4, 0x51, 0xa7, 0x52, 0xca, 0x81, 0x2e, 0xa9, 0x64, 0x6f, 0x2e, 0x5a, 0x0e, 0xc1, 
    0x38, 0xc3, 0xd4, 0xd8, 0xae, 0xfa, 0x74, 0x4e, 0x17, 0xf0, 0x14, 0x99, 0x86, 0x74, 0x50, 0x81, 
    0x5d, 0xca, 0xde, 0xed, 0x39, 0x17, 0xd0, 0x46, 0x31, 0x1a, 0x4b, 0x5d, 0x73, 0xbb, 0xd7, 0x5e, 
    0x50, 0x0c, 0xe9, 0x63, 0xa7, 0xd0, 0x5d, 0x81, 0x22, 0xea, 0x97, 0x45, 0xf2, 0xe0, 0xd9, 0xe9};

const uint8_t EXPECTED_RESULT2[] = {
    0xed, 0x6c, 0x66, 0x44, 0xc0, 0x75, 0x65, 0x3f, 0x19, 0x19, 0xfd, 0x05, 0xa1, 0x1a, 0x93, 0xc8, 
    0x05, 0x72, 0x67, 0xb9, 0xc2, 0xcb, 0xd0, 0x9e, 0x6d, 0x88, 0x82, 0xb3, 0x55, 0x47, 0x3e, 0x8e, 
    0x17, 0x97, 0x37, 0xcd, 0xc0, 0x06, 0x3b, 0xbf, 0xee, 0x5d, 0x65, 0x24, 0xea, 0xc1, 0x72, 0x4b, 
    0x57, 0x51, 0x41, 0xd7, 0x16, 0x42, 0x7b, 0x9f, 0xaa, 0xf7, 0x5c, 0x9f, 0x0e, 0x09, 0x03, 0xa5, 
    0x41, 0x11, 0x30, 0xf2, 0x73, 0x72, 0x49, 0x4d, 0x8a, 0xf5, 0x20, 0xef, 0xfa, 0xd6, 0x4c, 0xf3, 
    0x4c, 0xa2, 0x23, 0x1d, 0x2b, 0xd7, 0xb4, 0xcf, 0x01, 0x5c, 0x40, 0xe8, 0x5d, 0x4c, 0x62, 0xb2, 
    0xb7, 0x58, 0x88, 0xac, 0xa4, 0xd5, 0x79, 0x95, 0x36, 0x53, 0x42, 0xf6, 0x2f, 0x23, 0x95, 0xe4, 
    0x7f, 0xe0, 0x5e, 0x75, 0x49, 0x53, 0xba, 0x2e, 0x1a, 0xc7, 0xa1, 0xd7, 0xf9, 0x66, 0xe9, 0x2c, 
    0x35, 0x20, 0x20, 0x39, 0x5e, 0x6c, 0xcb, 0x59, 0x70, 0xdd, 0x8a, 0x67, 0x39, 0x8c, 0xe1, 0xce, 
    0x09, 0x58, 0x6a, 0x20, 0xf6, 0xd4, 0xe0, 0x91, 0x2b, 0xaa, 0xdb, 0x68, 0x3b, 0x41, 0x44, 0x20, 
    0x70, 0xf9, 0x59, 0x0b, 0xbb, 0x17, 0x5d, 0x0e, 0xdf, 0xdc, 0x2d, 0x85, 0xc0, 0xb0, 0xc6, 0x84, 
    0x9a, 0x7e, 0x18, 0x71, 0x55, 0xf8, 0xe9, 0x87, 0xb0, 0xd8, 0x1f, 0xc4, 0x3f, 0x51, 0x6c, 0x9a, 
    0x22, 0x96, 0x92, 0xeb, 0xd7, 0xac, 0x88, 0xc5, 0x0c, 0xa1, 0x47, 0x15, 0xdf, 0x32, 0x80, 0x23, 
    0xc4, 0x9d, 0x87, 0x76, 0xa2, 0xab, 0x7c, 0xf8, 0xb5, 0x25, 0x8d, 0x26, 0x9a, 0x2f, 0x17, 0xba, 
    0x6b, 0xd6, 0xf7, 0x53, 0xf3, 0x9d, 0xec, 0x18, 0x90, 0xba, 0xa4, 0x28, 0x3b, 0x31, 0x39, 0x92, 
    0x2d, 0x80, 0xd1, 0xe7, 0xd3, 0xda, 0x69, 0x66, 0x98, 0x77, 0x12, 0x5b, 0x8c, 0x0b, 0x41};

const uint8_t SERVER_PUBLIC_KEY_TEST[] = {0x49, 0x43, 0x86, 0x41, 0x04, 0x9E, 0x5B, 0x7F, 0x77, 0x8B, 0x79, 0x97, 0x61, 0x7C, 0x89, 0x93, 0xB6, 
0x40, 0x66, 0x08, 0xEF, 0xE2, 0xD3, 0xF2, 0xD6, 0x75, 0x1D, 0x6B, 0x3F, 0x5F, 0x53, 0x20, 0xFF, 0xFD, 0x6A, 0x7D, 0xC9, 0xA8, 0xF6, 0xC5, 0xC1,
0x47, 0x4F, 0x64, 0xD6, 0x68, 0x36, 0x4A, 0xE2, 0xED, 0x8D, 0xC4, 0xC1, 0xC6, 0xC4, 0x76, 0x74, 0x23, 0xB0, 0x13, 0x93, 0x7B, 0x44, 0x99, 0xF5, 
0x2C, 0x4F, 0x89, 0xA0};

TEST(AppletTests, ComputePRF) {
    IOT_DEBUG("\n-->Running AppletTests - ComputePRF\n");

    uint16_t dataLen = 0xFF;
    uint8_t data[0x100];

    string label1 = "extended master secret";

    int result = _rot->computePRFwithSecret((uint8_t*)TEST_SECRET1, sizeof(TEST_SECRET1),
                            (uint8_t*)label1.c_str(), label1.length(),
                            (uint8_t*)TEST_SEED1, sizeof(TEST_SEED1),
                            data, dataLen);
    printByteArray("master secret", data, dataLen);
    for (int i = 0; i < sizeof(EXPECTED_RESULT1); i++) {
        CHECK_EQUAL(data[i], EXPECTED_RESULT1[i]);
    }

    dataLen = 0xFF;
    string label2 = "key expansion";

    result = _rot->computePRFwithSecret((uint8_t*)EXPECTED_RESULT1, sizeof(EXPECTED_RESULT1),
                            (uint8_t*)label2.c_str(), label2.length(),
                            (uint8_t*)TEST_SEED2, sizeof(TEST_SEED2),
                            data, dataLen);
    printByteArray("master key", data, dataLen);
    for (int i = 0; i < dataLen; i++) {
        CHECK_EQUAL(data[i], EXPECTED_RESULT2[i]);
    }

    // uint8_t secretId = 1;

    // result = _rot->computePRFwithPSKECDHE(&secretId, 1,
    //                         (uint8_t*)TEST_SECRET1, sizeof(TEST_SECRET1),
    //                         (uint8_t*)label.c_str(), label.length(),
    //                         (uint8_t*)seed.c_str(), seed.length(),
    //                         data, dataLen);
    // CHECK_EQUAL(0, result);
    // printByteArray("Pre master secret", data, dataLen);

    // result = _rot->computePRFwithPSK(&secretId, 1,
    //                         (uint8_t*)label.c_str(), label.length(),
    //                         (uint8_t*)seed.c_str(), seed.length(),
    //                         data, dataLen);
    // CHECK_EQUAL(0, result);
    // printByteArray("Pre master secret", data, dataLen);

    IOT_DEBUG("\n-->Success \n");
}

/**
* The test case always make app crash, commented now
*/
TEST(AppletTests, GenerateKeypair) {
    IOT_DEBUG("\n-->Running AppletTests - GenerateKeypair\n");
    uint8_t clEph[CONTAINER_ID_LENGTH] = {CONTAINER_ID_CLIENT_EPHEMERAL_KEY};
    uint8_t svrEph[CONTAINER_ID_LENGTH] = {CONTAINER_ID_SERVER_EPHEMERAL_KEY};
    uint8_t container_id[CONTAINER_ID_LENGTH] = {CONTAINER_ID_CLIENT_EPHEMERAL_KEY};
    int result = 0;
    
    result = _rot->generateKeyPairByContainerId(container_id, CONTAINER_ID_LENGTH, &_kp);
    CHECK_EQUAL(0, result);

    result = _rot->putServerPublicKey(svrEph, CONTAINER_ID_LENGTH, (uint8_t*)SERVER_PUBLIC_KEY_TEST, sizeof(SERVER_PUBLIC_KEY_TEST));
    CHECK_EQUAL(0, result);

    uint8_t sharedSecret[0x60];
    uint16_t sharedSecretLen = 0x60;
    
    
    result = _rot->computeDHforKeypair(clEph, CONTAINER_ID_LENGTH, svrEph, CONTAINER_ID_LENGTH, sharedSecret, &sharedSecretLen);
    CHECK_EQUAL(0, result);
    //printByteArray("Shared secret", sharedSecret, sharedSecretLen);

    IOT_DEBUG("\n-->Success \n");
}



TEST(AppletTests, PutServerPublicKey) {
    IOT_DEBUG("\n-->Running AppletTests - PutServerPublicKey\n");

    int result = 0;
    uint8_t svrEph[CONTAINER_ID_LENGTH] = {CONTAINER_ID_SERVER_EPHEMERAL_KEY};
    result = _rot->putServerPublicKey(svrEph, CONTAINER_ID_LENGTH, (uint8_t*)SERVER_PUBLIC_KEY_TEST, sizeof(SERVER_PUBLIC_KEY_TEST));
    
    IOT_DEBUG("\n-->Success \n");
}


TEST(AppletTests, VerifySignature) {
    IOT_DEBUG("\n-->Running AppletTests - VerifySignature\n");
    uint32_t rot_algo = ROT_ALGO_SHA256_WITH_ECDSA;
    uint8_t keyId[CONTAINER_ID_LENGTH] = {CONTAINER_ID_KEY};
    
    int result = _rot->signInit(keyId, CONTAINER_ID_LENGTH, rot_algo);
    IOT_DEBUG("\n-->Running AppletTests - VerifySignature: SignInit\n");
    CHECK_EQUAL(0, result);

    uint8_t hash[] = {0x77, 0x12, 0xaa, 0xe3, 0xbb, 0xaa, 0xe5, 0xc0, 0x07, 0x47, 0x5a, 0x73, 0x36, 0xf3, 0xdd, 0xe0,
                     0xbc, 0x63, 0x38, 0x0a, 0x34, 0x8d, 0x23, 0x90, 0xc3, 0x51, 0x9e, 0x78, 0x2e, 0x9a, 0x82, 0x98};
    uint16_t hashLen = 0x20;
    uint16_t signLen = 0x60;
    uint8_t signature[0x60];      
    IOT_DEBUG("\n-->Running AppletTests - VerifySignature: SignFinal\n");           
    result = _rot->signFinal(hash, hashLen, signature, &signLen);
    CHECK_EQUAL(0, result);
    printByteArray("Signature", signature, signLen);

    IOT_DEBUG("\n-->Success \n");
}




