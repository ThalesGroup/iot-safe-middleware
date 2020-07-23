#include <iostream>
#include <vector>
#include "sim-access-util.h" 
#include "openssl-util.h"


// Serial Port to which Modem is connected and IoT Safe SIM is plugged in
static const char* modem_port = (const char*)"/dev/ttyACM0";

 
 
int main() {
    

    std::cout << "JWT Siging using IoT Safe Applet on a UICC" << std::endl;

    // Initialize communication with Modem/SIM
    if (initialize(modem_port) != 0)
        return -1;


    // Sample JWT Header and Payload 
    const std::string headerJwt = "{\"alg\":\"ES256\",\"typ\":\"JWT\"}";
    const std::string bodyJwt = "{\"id\": 42,\"claimType\": \"claimType\",\"claimUrl\":\"claimUrl\" }";

    std::cout << "JWT Header = " <<   headerJwt  << std::endl;
    std::cout << "JWT Claim/Body = " << bodyJwt << std::endl;

    // Base64 URL encoded header and payload
    std::string encoded_header = encodeBase64(headerJwt);
    std::string encoded_body = encodeBase64(bodyJwt);
    
    std::string header_body_encoded = encoded_header + "." + encoded_body;
 
    std::cout << "Base64 Url Encoded Header = " <<  encoded_header << std::endl;
    std::cout << "Base64 Url Encoded Body = " <<  encoded_body << std::endl;
    std::cout << "Encoded (Header + . + Body) = " <<  header_body_encoded << std::endl;

  
    // Compute SHA256 of b64_header+"."+b64_bodey

    std::vector<uint8_t> hash_val(0x20);
    computeSha256(header_body_encoded, hash_val); 

    
    // Compute Signature using SIM IoT Safe Applet
    std::vector<uint8_t> sign_val (0x60); // Value will be resized
    int res = computeSignature(hash_val, sign_val);
    std::cout << "Converting to Compact signature" << std::endl;
    std::vector<uint8_t> compact_sign (64);
    res = convertDERToCompactSignature(sign_val, compact_sign);
     
    if (res == 0) {
            std::string encoded_signature = encodeBase64(reinterpret_cast<const char*>(compact_sign.data()),  compact_sign.size());//base64_encode(reinterpret_cast<const unsigned char*>(out_sign), out_len,true);
            std::string jwt_val = header_body_encoded+"."+ encoded_signature;
            std::cout << "Signature" << encoded_signature << std::endl;
            std::cout << "JWT = " << jwt_val << std::endl;           
    }


    //  For JWT verification purposes
    //  Retrieve Public Key from SIM IoT Safe
    
    std::vector<uint8_t> pub_key(512); // Place hoder for public will be resized to actual size.
    res = getClientPublicKey(pub_key) ;
    if (res == 0) {
        std::string cert_pem;
        std::string pub_key_pem;
	convertDERtoX509(pub_key, cert_pem,pub_key_pem);
	std::cout << "Public Key: " << std::endl << pub_key_pem << std::endl;
	std::cout << "Client Certificate: " << std::endl << cert_pem << std::endl;
    }
    else {
        std::cout << std::endl << "Error: Get Client Public Key Failed" << std::endl;
    }
    
    // Cleanup: Close modem Port
     cleanup();
     

}


