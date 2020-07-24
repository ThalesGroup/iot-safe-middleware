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
#include <iostream>
#include <vector>
#include "ROT.h"
#include "GenericModem.h"
#include "sim-access-util.h"

static GenericModem modem;
static ROT* _rot = NULL;
 

/** 
* Initialize communication with Modem
*
*/

int initialize(const char *modem_port)
{
    printf("\n-->initialize \n");
    if(!modem.open(modem_port)) {
        printf("\nError modem not found!\n");
        return -1;
    }

    _rot = new ROT();
    _rot->init(&modem);

    if (!_rot->select(true)) {
        printf("\nError: cannot select applet!\n");
        return -1;
    }
    return 0;
}

void cleanup()
{
    delete _rot;
    modem.close();
}



/** 
* Compute Signature using SIM IoT Safe
*
*/
int computeSignature(std::vector<uint8_t> hash_val, std::vector<uint8_t> &sign_res) {
    // Set the Signature Algo and Key Container to use
    // In the sample IoT Safe SIM, ID=1 is a Key Container with ECDSA P256 R1 curve 
    uint32_t rot_algo = ROT_ALGO_SHA256_WITH_ECDSA;
    uint8_t keyId[CONTAINER_ID_LENGTH] = {CONTAINER_ID_KEY};
    
    int result = _rot->signInit(keyId, CONTAINER_ID_LENGTH, rot_algo);
    //printf("\n-->Running AppletTests - VerifySignature: SignInit\n");
    if (result == 0) {
       // printf("\n-->Running AppletTests - VerifySignature: SignFinal\n");
        uint8_t * out_sign = sign_res.data();
        uint16_t out_len = sign_res.size();           
        result = _rot->signFinal((const uint8_t*)hash_val.data(), (uint16_t)hash_val.size(), out_sign, &out_len);
        if (result == 0)
        {
            if (out_len != sign_res.size())
                sign_res.resize(out_len);
        }
     
    }
    return result;
}

int computeSignature(uint8_t *hash, uint16_t hash_len, uint8_t* out_sign, uint16_t *out_len) {
    printf("\n-->Running AppletTests - VerifySignature\n");
    uint32_t rot_algo = ROT_ALGO_SHA256_WITH_ECDSA;
    uint8_t keyId[CONTAINER_ID_LENGTH] = {CONTAINER_ID_KEY};
    
    int result = _rot->signInit(keyId, CONTAINER_ID_LENGTH, rot_algo);
    printf("\n-->Running AppletTests - VerifySignature: SignInit\n");
    if (result == 0) {
        printf("\n-->Running AppletTests - VerifySignature: SignFinal\n");           
        result = _rot->signFinal(hash, hash_len, out_sign, out_len);
     
    }
    return result;
}
/**
* Retrieve the public key from the SIM IoT Safe
*
*/ 

int getClientPublicKey(std::vector<uint8_t> &pub_key_data) 
{
    uint8_t* pp_pub_key; // Memory will be allocated by middleware
    uint16_t pub_key_len=512; // Sufficient for P256 R1 Key
    printf("\n-->Running AppletTests - gettServerPublicKey\n");

    int result = 0;
    uint8_t containerId[CONTAINER_ID_LENGTH] = {CONTAINER_ID_CERT_CLIENT};
 
 
    result = _rot->getCertificateByContainerId(containerId, CONTAINER_ID_LENGTH, &pp_pub_key, &pub_key_len);

    if ((result == 0) && (pub_key_len > 0))
    {
        pub_key_data.resize(pub_key_len);
        uint8_t *pub_keyp = pub_key_data.data();
        memcpy(pub_keyp, pp_pub_key, pub_key_len);
        free(pp_pub_key);
    }
    
    return result;
}
