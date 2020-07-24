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
#ifndef __SIM_ACCESS_UTIL_H__
#define __SIM_ACCESS_UTIL_H__

int computeSignature(std::vector<uint8_t> hash_val, std::vector<uint8_t> &sign_res) ;
int getClientPublicKey(std::vector<uint8_t> &pub_key_data);
int initialize(const char *modem_port);
void cleanup();

#endif //  __SIM_ACCESS_UTIL_H__