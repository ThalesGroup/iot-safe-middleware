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
#include <algorithm>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/ec.h>
#include <openssl/bn.h>
#include <openssl/ecdsa.h>
#include "openssl-util.h"
#include <stdio.h>
#include <string.h>
char *X509_to_PEM(X509 *cert) ;
char *EVPKEY_to_PEM(EVP_PKEY *pub_key);



/***
* SHA266 hashing
* void computeSha256(unsigned char const* bytes_to_encode, unsigned int in_len, unsigned char * out_result,unsigned int *out_len)
*/
void computeSha256(std::string msg, std::vector<uint8_t> &hash_res) 
{

      static bool openssl_algo_loaded = false;

      if (openssl_algo_loaded == false) 
      {
            OpenSSL_add_all_algorithms();
            openssl_algo_loaded = true;
      }
      
      EVP_MD_CTX *mdctx;
      const EVP_MD *md;
      const char* digest_name = "SHA256";
   
      unsigned int i;

  
      md = EVP_get_digestbyname(digest_name);

      if(!md) {
              printf("Unknown message digest %s\n",digest_name);
              exit(1);
      }
      uint8_t *out_res = hash_res.data();
      unsigned int out_len = hash_res.size(); 
      mdctx = EVP_MD_CTX_create();
      EVP_DigestInit_ex(mdctx, md, NULL);
      EVP_DigestUpdate(mdctx, msg.c_str(), msg.length());
      EVP_DigestFinal_ex(mdctx, out_res,&out_len);
      EVP_MD_CTX_destroy(mdctx);
}

 
/***
* std::string encodeBase64 (std::string msg, bool url=true, bool padding=false)
*/
std::string encodeBase64 (std::string msg, bool url, bool padding)
{

    return encodeBase64(msg.c_str(), msg.length(),url, padding);

}
std::string encodeBase64 (const char* msg,uint16_t len, bool url, bool padding)
{

    BIO* mbio = BIO_new(BIO_s_mem());
    BIO* b64bio = BIO_new(BIO_f_base64());
    BIO_set_flags(b64bio, BIO_FLAGS_BASE64_NO_NL);
    BIO* bio = BIO_push(b64bio, mbio);

    BIO_write(bio, msg, len);
    BIO_flush(bio);

    char* data = NULL;
    size_t datalen = 0;
    datalen = BIO_get_mem_data(mbio, &data);
    data[datalen] = '\0';
    std::string b64str = std::string(data,datalen);
    if (!padding) 
    {
        // Remove padding
        b64str.erase(std::remove(b64str.begin(), b64str.end(), '='), b64str.end());
    }
    if (url)
    {
        // URL encoding
        std::replace(b64str.begin(),b64str.end(),'+','-');
        std::replace(b64str.begin(),b64str.end(),'/','_');
    }
    BIO_free(mbio);
    BIO_free(b64bio);
 //   BIO_free(bio);
    return b64str;

}

/***
* std::string encodeBase64 (std::string b64msg)
*/
std::string decodeBase64 (std::string b64msg)
{
    std::string msg;

    BIO *mbio = BIO_new_mem_buf(b64msg.c_str(), b64msg.length());

    BIO *b64bio = BIO_new(BIO_f_base64());

    BIO_set_flags(b64bio, BIO_FLAGS_BASE64_NO_NL);

    BIO *bio = BIO_push(b64bio, mbio);

    int16_t decoded_len = b64msg.length(); // Rough estimation and it will be smaller
    char* p_decoded_msg = (char*)malloc(decoded_len);
    if (p_decoded_msg != NULL) 
    {
        decoded_len = BIO_read(bio, p_decoded_msg, decoded_len);
        p_decoded_msg[decoded_len] = '\0';
        msg = std::string(p_decoded_msg,decoded_len);
        free(p_decoded_msg);
    }

    BIO_free(mbio);
    BIO_free(b64bio);
 //   BIO_free(bio);
    return msg;

}


/**
* Convert DER Coded certificate into X.509 cert and extract Pub Key in PEM format
* Reference : https://www.openssl.org/docs/man1.1.0/man3/d2i_X509.html
*/
int convertDERtoX509(std::vector<uint8_t> pub_key_in, std::string &cert, std::string &pub_key)
{
	X509 *x;

 	unsigned char *p;

	long len = pub_key_in.size();
 	p = (unsigned char*) pub_key_in.data();


	x = d2i_X509(NULL,(const unsigned char**)  &p, len);

        if (x == NULL)
        {
                printf("\n Error converting to x509\n");
                return -1;
        }
        else
        {
                char *pem= X509_to_PEM(x);
                if (pem != NULL)
                    cert = pem;
          
                EVP_PKEY* pkey = X509_get_pubkey(x);

                char *pub_pem = EVPKEY_to_PEM(pkey); 
                if (pub_pem != NULL)
                    pub_key = pub_pem;
                return 0;

        }

}


#define MAX_READ_SIZE 10000
/**
* Convert X509 Coded certificate into  PEM format
* Reference: https://www.openssl.org/docs/man1.1.1/man3/PEM_write_bio_X509.html
*/
char *X509_to_PEM(X509 *cert) 
{

    BIO *bio = NULL;
    char *pem = NULL;

    if (NULL == cert) {
        return NULL;
    }

    bio = BIO_new(BIO_s_mem());
    if (NULL == bio) {
        return NULL;
    }

    if (0 == PEM_write_bio_X509(bio, cert)) {
        BIO_free(bio);
        return NULL;
    }

    pem = (char *) malloc(MAX_READ_SIZE);
    if (NULL == pem) {
        BIO_free(bio);
        return NULL;    
    }

    memset(pem, 0, MAX_READ_SIZE);
    int n=BIO_read(bio, pem, MAX_READ_SIZE);
    BIO_free(bio);
    return pem;
}

/**
* Convert DER Coded Pub key  into  PEM format
* Reference https://www.openssl.org/docs/man1.1.1/man3/PEM_write_bio_X509.html
*/

char *EVPKEY_to_PEM(EVP_PKEY *pub_key)
{

    BIO *bio = NULL;
    char *pem = NULL;

    if (NULL == pub_key) {
        return NULL;
    }

    bio = BIO_new(BIO_s_mem());
    if (NULL == bio) {
        return NULL;
    }

    if (0 == PEM_write_bio_PUBKEY(bio, pub_key)) {
        BIO_free(bio);
        return NULL;
    }

    pem = (char *) malloc(MAX_READ_SIZE);
    if (NULL == pem) {
        BIO_free(bio);
        return NULL;
    }

    memset(pem, 0, MAX_READ_SIZE);
    int n=BIO_read(bio, pem, MAX_READ_SIZE);
    BIO_free(bio);
    return pem;
}
 

/**
* Convert DER encoded signature to compact signature with R, S valuds (32 bytes each)
*/
int convertDERToCompactSignature(std::vector<uint8_t> sign_in, std::vector<uint8_t> &sign_out)
{
    int res=-1;
    int r_len;
    int s_len;
    unsigned int bn_len;
    ECDSA_SIG *ecdsa_sig = NULL;
    const unsigned char* sig_data_in = (const unsigned char*)sign_in.data();
    long sig_len = (long) sign_in.size();
   
    ecdsa_sig = d2i_ECDSA_SIG(NULL, (const unsigned char**)&sig_data_in, sig_len);
    if (ecdsa_sig == NULL) {
      return -1;
    }
    const BIGNUM *r = NULL;
    const BIGNUM *s = NULL;
    ECDSA_SIG_get0(ecdsa_sig,&r, &s);
    /* Store the two BIGNUMs in raw_buf. */
    r_len = BN_num_bytes(r);
    s_len = BN_num_bytes(s);
    bn_len = 32; // P256 R1
  
    uint8_t* sign_out_buf =sign_out.data();
  
    if (sign_out.size() <  (2*bn_len))
        sign_out.resize(2*bn_len);
     
    /* Pad the bignums with leading zeroes. */
    if (!BN_bn2binpad(r, (unsigned char*)sign_out_buf, bn_len) ||
        !BN_bn2binpad(s, (unsigned char*)sign_out_buf + bn_len, bn_len)) {
        res = -1; 
    }
    else {
        res = 0;
    }
    ECDSA_SIG_free(ecdsa_sig);
    return res;
 
}
