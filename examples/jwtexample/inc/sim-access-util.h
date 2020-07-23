#ifndef __SIM_ACCESS_UTIL_H__
#define __SIM_ACCESS_UTIL_H__

int computeSignature(std::vector<uint8_t> hash_val, std::vector<uint8_t> &sign_res) ;
int getClientPublicKey(std::vector<uint8_t> &pub_key_data);
int initialize(const char *modem_port);
void cleanup();

#endif //  __SIM_ACCESS_UTIL_H__