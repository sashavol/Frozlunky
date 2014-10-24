#include "../debug.h"

#include "crypt.h"
#include "unscrambler.h"
#include "base64.h"

#include <sstream>

#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/aes.h>
#include <openssl/rand.h>

#include <boost/assign.hpp>

namespace Frozboards {
	unsigned char pub_scramb[] = {211,208,209,214,215,187,189,176,191,187,212,163,167,179,188,
		166,173,205,167,174,179,196,197,202,203,200,238,174,171,168,162,150,180,156,146,153,
		189,178,169,191,189,188,147,234,165,225,146,142,159,136,138,138,139,134,139,134,151,
		253,133,142,139,136,130,252,217,246,255,250,235,252,249,194,230,193,205,241,202,192,
		199,150,246,203,156,196,239,157,217,230,255,151,230,169,148,229,143,206,215,207,209,
		171,212,211,208,226,230,211,216,250,212,229,245,206,234,198,235,179,206,202,228,194,
		191,199,227,232,201,206,200,26,55,36,41,44,20,10,0,16,0,66,50,58,2,25,22,6,27,1,7,39,
		16,37,44,19,21,36,32,42,104,10,79,106,52,21,108,14,9,96,55,35,4,61,1,32,48,20,52,54,
		29,117,8,63,8,25,39,108,19,9,125,21,24,25,9,74,106,119,123,10,85,119,8,96,3,89,65,100,
		88,85,70,64,27,24,89,98,101,125,93,117,31,65,18,87,105,43,87,82,124,78,77,42,126,110,
		90,123,87,95,39,102,119,121,119,125,86,67,72,96,65,94,35,115,119,83,65,98,82,96,119,141,
		155,144,147,175,173,154,150,187,181,145,184,187,181,159,198,142,156,187,157,217,142,186,
		195,150,210,143,210,138,181,163,234,139,159,148,176,173,174,142,157,166,229,182,165,185,
		133,189,187,130,189,155,189,248,129,165,134,179,180,142,173,251,134,148,247,233,245,197,
		251,220,235,247,211,244,132,255,245,208,229,137,220,201,248,230,232,239,152,206,242,144,
		232,234,214,144,241,146,148,149,216,200,216,232,226,221,213,162,244,192,228,221,203,194,
		196,236,232,212,235,229,239,228,248,238,194,236,221,241,200,198,249,20,19,7,51,43,52,42,
		58,66,37,33,49,36,4,33,59,88,25,63,26,63,11,57,88,42,7,93,5,84,17,3,49,60,84,62,43,18,30,
		24,9,22,20,95,121,126,127,124,125,10,0,9,108,27,31,11,4,14,5,101,15,6,27,108,109,18,19,16,54};

	unsigned char key_scramb[] = {64,47,65,236,140,44,197,112,113,22,170,37,217,203,57,214};

	std::string RSA_Encrypt(const std::string& plain) 
	{
		using namespace Frozboards;

		Unscrambler::membuf mem(pub_scramb, pub_scramb+sizeof(pub_scramb)/sizeof(unsigned char));
		{
			Unscrambler unscramb(mem);
			unscramb.cycle_unscramble();
			mem = unscramb.data();
		}
		
		BIO* bufio;
		RSA* rsa;
		rsa = RSA_new();

		unsigned char* pub = mem.data();
		bufio = BIO_new_mem_buf(pub, mem.size());
		rsa = PEM_read_bio_RSA_PUBKEY(bufio, &rsa, NULL, NULL);

#ifdef DEBUG_MODE
		if(rsa == NULL) {
			ERR_print_errors_fp(stdout);
			return "";
		}
#else
		if(rsa == NULL) {
			return "";
		}
#endif

		const char* plainstr = plain.c_str();
		int size = RSA_size(rsa);
		unsigned char* encoded = new unsigned char[size];
		RSA_public_encrypt(plain.size(), (const unsigned char*)plainstr, encoded, rsa, RSA_PKCS1_OAEP_PADDING);
		std::string ret = Base64::Encode(encoded, size);

		delete[] encoded;

		BIO_free(bufio);
		RSA_free(rsa);

		return ret;
	}

	std::string random_str() {
		srand((unsigned int)time(nullptr));

		std::stringstream str;

		int len = 24;
		while(len > 0) {
			str << (char)((rand()%(0x7A-0x61))+0x61);
			len--;
		}

		return str.str();
	}

	std::string Encrypt(const std::string& plain) {
		unsigned int plain_size = plain.size();
		unsigned char* aes_enc = new unsigned char[plain_size+128];

		unsigned char key[16], iv[16];
		RAND_bytes(key, 16);
		RAND_bytes(iv, 16);
		
		AES_KEY akey;
		if(AES_set_encrypt_key(key, 128, &akey) < 0) {
#ifdef DEBUG_MODE
			std::cout << "Warning: Error setting AES encryption key." << std::endl;
#endif
			return "";
		}

		//!TODO add expiry token to crypt

		unsigned char* plain_data = new unsigned char[plain_size+72];
		std::memcpy(plain_data, iv, 16);
		std::memcpy(plain_data+16, &plain_size, sizeof(unsigned int));
		std::memcpy(plain_data+16+4, plain.c_str(), plain_size);
		std::memcpy(plain_data+16+4+plain_size, key, 16);

		AES_cbc_encrypt(plain_data, aes_enc, plain_size+72, &akey, iv, AES_ENCRYPT);
		delete[] plain_data;

		std::stringstream ss;
		ss << Base64::Encode(key, sizeof(key));
		ss << "-!-";
		ss << Base64::Encode(iv, sizeof(iv));
		

		std::string aes_key_b64 = RSA_Encrypt(ss.str());
		std::string aes_data = Base64::Encode(aes_enc, plain_size+64);

		delete[] aes_enc;

		return aes_key_b64 + "-@-" + aes_data;
	}




	/////////////
	//DECRYPT
	////////////

	std::string Decrypt(std::string b64message) {
		if(b64message.find("!~~") != 0) {
			return "";
		}

		b64message = b64message.substr(3);

		unsigned char key[16];
		unsigned char iv[16];

		{
			Unscrambler::membuf mem(key_scramb, key_scramb+sizeof(key_scramb)/sizeof(unsigned char));
			{
				Unscrambler unscramb(mem);
				unscramb.cycle_unscramble();
				mem = unscramb.data();
			}

			std::memcpy(key, mem.data(), sizeof(key));
		}

		std::string data = Base64::Decode(b64message);
		std::memcpy(iv, data.c_str(), 16);

		char* decrypted = new char[data.size()+128];

		AES_KEY akey;
		AES_set_decrypt_key(key, 128, &akey);
		AES_cbc_encrypt((const unsigned char*)(data.c_str()+16), (unsigned char*)decrypted, data.size()-16, &akey, iv, AES_DECRYPT);
		
		std::string decstr(decrypted);
		delete[] decrypted;
		
		return decstr;
	}
}