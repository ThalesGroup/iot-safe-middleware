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
#ifndef __OPENSSL_UTIL_H__
#define __OPENSSL_UTIL_H__

int convertDERtoX509(std::vector<uint8_t> pub_key_in, std::string &cert, std::string &pub_key);
int convertDERToCompactSignature(std::vector<uint8_t> sign_in, std::vector<uint8_t> &sign_out);
std::string encodeBase64 (std::string msg,bool url=true, bool padding=false);
std::string encodeBase64 (const char* msg, uint16_t len,bool url=true, bool padding=false);
std::string decodeBase64 (std::string b64msg);
void computeSha256(unsigned char const* bytes_to_encode, unsigned int in_len, unsigned char * out_result,unsigned int *out_len);
void computeSha256(std::string msg, std::vector<uint8_t> &hash_res) ;

#endif // __OPENSSL_UTIL_H__