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

#ifndef __ROT_H__
#define __ROT_H__

#include "Applet.h"

//#define USE_LEGACY_APPLET

#define CMD_MAX_LEN					255
#define ECC_PUBLIC_KEY_LEN  				0x45
#define MAX_CONTAINER_ID_LEN				0x16

#define CONTAINER_ID_LENGTH			1
#define CONTAINER_ID_KEY          		1 
#define CONTAINER_ID_CERT_CLIENT  		2
#define CONTAINER_ID_CERT_SERVER		3 
#define CONTAINER_ID_CLIENT_EPHEMERAL_KEY	4
#define CONTAINER_ID_SERVER_EPHEMERAL_KEY       5

// Mode of Operation
#define OPERATION_MODE_FULL_TEXT	1
#define OPERATION_MODE_LAST_BLOCK	2
#define OPERATION_MODE_PADDING		3


// HASH ALGORITHM

#define HASH_SHA256                    0x0001
#define HASH_SHA384                    0x0002
#define HASH_SHA512                    0x0004

// SIGN ALGORITHM 

#define SIGN_RSA_PKCS1_PADDING         0x01
#define SIGN_RSA_PSS_PADDING           0x02
#define SIGN_ECDSA                     0x04

#define ROT_ALGO_SHA256_WITH_RSA_PKCS1_PADDING     ((((uint32_t) HASH_SHA256) << 8) | SIGN_RSA_PKCS1_PADDING)
#define ROT_ALGO_SHA384_WITH_RSA_PKCS1_PADDING     ((((uint32_t) HASH_SHA384) << 8) | SIGN_RSA_PKCS1_PADDING)
#define ROT_ALGO_SHA512_WITH_RSA_PKCS1_PADDING     ((((uint32_t) HASH_SHA512) << 8) | SIGN_RSA_PKCS1_PADDING)

#define ROT_ALGO_SHA256_WITH_RSA_PSS_PADDING       ((((uint32_t) HASH_SHA256) << 8) | SIGN_RSA_PSS_PADDING)
#define ROT_ALGO_SHA384_WITH_RSA_PSS_PADDING       ((((uint32_t) HASH_SHA384) << 8) | SIGN_RSA_PSS_PADDING)
#define ROT_ALGO_SHA512_WITH_RSA_PSS_PADDING       ((((uint32_t) HASH_SHA512) << 8) | SIGN_RSA_PSS_PADDING)

#define ROT_ALGO_SHA256_WITH_ECDSA                 ((((uint32_t) HASH_SHA256) << 8) | SIGN_ECDSA)
#define ROT_ALGO_SHA384_WITH_ECDSA                 ((((uint32_t) HASH_SHA384) << 8) | SIGN_ECDSA)
#define ROT_ALGO_SHA512_WITH_ECDSA                 ((((uint32_t) HASH_SHA512) << 8) | SIGN_ECDSA)

// Pseudo Random Function Mode
#define PRF_MODE_GENERAL				0
#define PRF_MODE_PSK_PLAIN				1
#define PRF_MODE_PSK_ECDHE				2

// Session
#define PUT_PUBLIC_KEY_SESSION 0x00
#define COMPUTE_SIGNATURE_SESSION 0x01
#define VERIFY_SIGNATURE_SESSION 0x02

// Key Pair Info
typedef struct
{
	uint8_t priv_key_id[MAX_CONTAINER_ID_LEN];
	uint16_t priv_key_id_len;

	uint8_t pub_key_id[MAX_CONTAINER_ID_LEN];
	uint16_t pub_key_id_len;

	uint8_t pub_key_data[ECC_PUBLIC_KEY_LEN];
	uint16_t pub_key_data_len;
} RotKeyPair;


#ifdef __cplusplus

/**
 * The class is an implementation of GSMA Specification "IoT Security Applet Interface Description".
 * https://www.gsma.com/iot/wp-content/uploads/2019/12/IoT.05-v1-IoT-Security-Applet-Interface-Description.pdf
 */
class ROT: public Applet {
    public:
    	/**
	 * Create an instance of ROT
	 */
	ROT(void);
	/**
	 * Destrcutor
	 */
	~ROT(void) {}

		/**
	 * Get certificate on the container identify by the provided id.
	 * 
	 * @param[in]  containerId specify the container id of the certificate
	 * @param[in]  containerIdLen length of the container
	 * @param[out]  cert a buffer (auto allocated) which will contain the resulted certificate. 
	 *              It must be freed if the operation was sucessful
	 * @param[in, out]  certLen the length of certificate
	 * @return 0 in case operation was successful, error code otherwise.
	 */
	int getCertificateByContainerId(const uint8_t *containerId, uint16_t containerIdLen, uint8_t **cert, uint16_t *certLen);
	
	/**
	 * Generate a random buffer with specified length
	 * 
	 * @param[out]  data the output of random buffer
	 * @param[in]  dataLen the length of random buffer
	 * @return 0 in case operation was successful, error code otherwise.
	 */
	int generateRandom(uint8_t *data, uint16_t dataLen);

	/**
	 * Prepare context in applet prior computing a signature.
	 * 
	 * @param[in]  containerId specify the container id of the certificate
	 * @param[in]  containerIdLen length of the container
	 * @param[in]  algorithm the targetted signature algorithm, 
	 * 				all algorithms are defined in "SIGN ALGORITHM".
	 * @return 0 in case operation was successful, error code otherwise.
	 */
	int signInit(const uint8_t *containerId, uint16_t containerIdLen, uint32_t algorithm);
	
	/**
	 * Compute signature
	 * 
	 * @param[in]  hash a buffer which contain data to encrypt using key to compute signature
	 * @param[in]  hash_len the length of hash buffer
	 * @param[out]  signature a buffer which will contain the resulted signature
	 * @param[in, out]  signature_len the length of signature buffer
	 * @return 0 in case operation was successful, error code otherwise.
	 */
	int signFinal(const uint8_t *hash, uint16_t hashLen, uint8_t *signature, uint16_t *signatureLen);


	/**
	 * Get key pair stored on the container identify by the provided id.
	 * 
	 * @param[in]  containerId specify the container id of the key pair
	 * @param[in]  containerIdLen length of the container
	 * @param[out]  kp a pointer of the found key pair, NULL otherwise.
	 * @return 0 in case operation was successful, error code otherwise.
	 */
	int generateKeyPairByContainerId(const uint8_t *containerId, uint16_t containerIdLen, RotKeyPair *kp);

	/**
	 * Set the server public key to the specified container. 
	 * 
	 * @param[in]  containerId specify the container id of the server public key
	 * @param[in]  containerIdLen length of the container
	 * @param[in]  pubKey a pointer of the buffer holding server public key
	 * @param[in]  pubKeyLen the length of server public key
	 * @return 0 in case operation was successful, error code otherwise.
	 */
	int putServerPublicKey(const uint8_t *containerId, uint16_t containerIdLen, const uint8_t *pubKey, uint16_t pubKeyLen);
	
	/**
	 * Compute Diffie–Hellman key exchange with the specified key pair.
	 * 
	 * @param[in]  clientEphContainerId specify the container id of the client ephemeral keypair
	 * @param[in]  clientEphContainerIdLen length of the clientEphContainerId
	 * @param[in]  serverEphContainerId specify the container id of the server ephemeral public key
	 * @param[in]  serverEphContainerIdLen length of the serverEphContainerId
	 * @param[in]  sharedSecret a buffer of shared secret used to DH computation
	 * @param[in]  sharedSecretLen the length of shared secret
	 * @return 0 in case operation was successful, error code otherwise.
	 */
	int computeDHforKeypair(const uint8_t *clientEphContainerId, uint16_t clientEphContainerIdLen, const uint8_t *serverEphContainerId, uint16_t serverEphContainerIdLen, uint8_t *sharedSecret, uint16_t *sharedSecretLen);

	/**
	 * Use Compute PRF command to generate pseudo-random numbers based on the PRF function.
	 * General mode is used. 
	 * 
	 * @param[in]  secret the secret buffer as an input of PRF function
	 * @param[in]  secretLen the length of secret
	 * @param[in]  label the label buffer as an input of PRF function
	 * @param[in]  labelLen the length of label
	 * @param[in]  seed the seed buffer as an input of PRF function
	 * @param[in]  seedLen the length of seed
	 * @param[out]  data the buffer for generated pseudo-random 
	 * @param[in]  dataLen the length of generated pseudo-random
	 * @return 0 in case operation was successful, error code otherwise.
	 */
	int computePRFwithSecret(const uint8_t *secret, uint16_t secretLen,
				const uint8_t *label, uint16_t labelLen,
				const uint8_t *seed, uint16_t seedLen,
				uint8_t *data, uint16_t dataLen);

	/**
	 * Use Compute PRF command to generate pseudo-random numbers based on the PRF function.
	 * PSK-plain pre-master secret mode is used. 
	 * 
	 * @param[in]  secretId the container ID of pre-shared secret
	 * @param[in]  secretIdLen the length of pre-shared secret
	 * @param[in]  label the label buffer as an input of PRF function
	 * @param[in]  labelLen the length of label
	 * @param[in]  seed the seed buffer as an input of PRF function
	 * @param[in]  seedLen the length of seed
	 * @param[out]  data the buffer for generated pseudo-random 
	 * @param[in]  dataLen the length of generated pseudo-random
	 * @return 0 in case operation was successful, error code otherwise.
	 */
	int computePRFwithPSK(const uint8_t *secretId, uint16_t secretIdLen,
						  const uint8_t *label, uint16_t labelLen,
						  const uint8_t *seed, uint16_t seedLen,
						  uint8_t *data, uint16_t dataLen);

	/**
	 * Use Compute PRF command to generate pseudo-random numbers based on the PRF function.
	 * PSK-ECDHE pre-master secret mode is used. 
	 * 
	 * @param[in]  secretId the container ID of pre-shared secret
	 * @param[in]  secretIdLen the length of pre-shared secret
	 * @param[in]  premaster ECDH computation result
	 * @param[in]  pmsLen the length of ECDH computation result
	 * @param[in]  label the label buffer as an input of PRF function
	 * @param[in]  labelLen the length of label
	 * @param[in]  seed the seed buffer as an input of PRF function
	 * @param[in]  seedLen the length of seed
	 * @param[out]  data the buffer for generated pseudo-random 
	 * @param[in]  dataLen the length of generated pseudo-random
	 * @return 0 in case operation was successful, error code otherwise.
	 */
	int computePRFwithPSKECDHE(const uint8_t *secretId, uint16_t secretIdLen,
							   const uint8_t *premaster, uint16_t pmsLen,
							   const uint8_t *label, uint16_t labelLen,
							   const uint8_t *seed, uint16_t seedLen,
							   uint8_t *data, uint16_t dataLen);
		

    private:
	RotKeyPair _keypairs;
	uint16_t getFileLength(const uint8_t *fileId, uint16_t fileIdLen,
				 const uint8_t *fileLbl, uint16_t fileLblLen);
    int readFile(const uint8_t *path, uint16_t pathLen,
				 const uint8_t *fileId, uint16_t fileIdLen,
				 const uint8_t *fileLbl, uint16_t fileLblLen,
				 uint8_t **data, uint16_t *dataLen);

	int getRandom(uint8_t *data, uint16_t dataLen);
	int generateKeypair(const uint8_t *keyId, uint16_t keyIdLen,
				const uint8_t *keyLbl, uint16_t keyLblLen,
				uint8_t *privKeyId, uint16_t *privKeyIdLen,
				uint8_t *pubKeyId, uint16_t *pubKeyIdLen,
				uint8_t *pubKeyData, uint16_t *pubKeyDataLen);

	int computeSignatureInit(const uint8_t *keyId, uint16_t keyIdLen,
				const uint8_t *keyLbl, uint16_t keyLblLen,
				uint8_t operationMode, uint16_t hashAlgo, uint8_t signAlgo);
	int computeSignatureUpdate(uint8_t operationMode,
				   const uint8_t *data, uint32_t dataLen,
				   const uint8_t *intermediateHash, uint16_t intermediateHashLen,
				   uint32_t hashedBytes,
				   uint8_t *sign, uint16_t *signLen);
	int computeDH(const uint8_t *privKeyId, uint16_t privKeyIdLen,
			const uint8_t *pubKeyId, uint16_t pubKeyIdLen,
			const uint8_t *privLbl, uint16_t privLblLen,
			const uint8_t *pubLbl, uint16_t pubLblLen,
			uint8_t *sharedSecret, uint16_t *sharedSecretLen);

	int computePRF(uint8_t mode,
			const uint8_t *secretId, uint16_t secretIdLen,
			const uint8_t *secretLbl, uint16_t secretLblLen,
			const uint8_t *secret, uint16_t secretLen,
			const uint8_t *pms, uint16_t pmsLen,
			const uint8_t *lblSeed, uint16_t lblSeedLen,
			uint8_t *pRandom, uint16_t pRandomLen);

	int putPublicKeyInit(const uint8_t *pubKeyId, uint16_t pubKeyIdLen,
				const uint8_t *pubKeyLbl, uint16_t pubKeyLblLen);

	int putPublicKeyUpdate(const uint8_t *pubKey, uint16_t pubKeyLen);

};

#else 

typedef struct ROT ROT; 

ROT* ROT_create(void);
void ROT_destroy(ROT* rot);
int ROT_get_certificate_by_container_id(ROT* rot, const uint8_t *container_id, uint16_t containerIdLen, uint8_t **cert, uint16_t *cert_len);
bool ROT_get_key_pair_by_container_id(ROT* rot, uint8_t container_id, mias_key_pair_t** kp);

bool ROT_generate_random(ROT* rot, uint8_t* data, uint16_t dataLen);
int ROT_generate_key_pair_by_container_id(ROT* rot, const uint8_t* container_id, uint16_t containerIdLen,, RotKeyPair* kp);

int ROT_sign_init(ROT* rot, const uint8_t *containerId, uint16_t containerIdLen, uint32_t algorithm);
int ROT_sign_final(ROT* rot, uint8_t* hash, uint16_t hash_len, uint8_t* signature, uint16_t* signature_len);
int ROT_sign_final_ECDSA(ROT* rot, const uint8_t* hash, uint16_t hash_len, uint8_t* signature, uint16_t* signature_len);
int ROT_put_server_public_key(ROT* rot, uint8_t container_id, uint8_t* pubKey, uint16_t pubKeyLen);
int ROT_compute_DH_for_keypair(ROT* rot, ROT* rot, const uint8_t *clientEphContainerId, uint16_t clientEphContainerIdLen, 
					const uint8_t *serverEphContainerId, uint16_t serverEphContainerIdLen,
    					uint8_t *sharedSecret, uint16_t *sharedSecretLen);

int ROT_compute_prf_with_secret(ROT* rot, const uint8_t* secret, uint16_t secretLen, 
                                            const uint8_t* label, uint16_t labelLen,
                                            const uint8_t* seed, uint16_t seedLen,
                                            uint8_t* data, uint16_t dataLen);

#endif

#endif