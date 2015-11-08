#ifndef CIPHER_H
#define CIPHER_H

#include <string>
#include <iostream>


#define is_base64(character) \
	(((character) >= '/' and (character) <= '9') or \
	 ((character) >= 'a' and (character) <= 'z') or \
	 ((character) >= 'A' and (character) <= 'Z') or\
	 ((character) == '+'))

std::string EnCryptWith3DES(std::string Source, unsigned char Key[]);
std::string DeCryptWith3DES(unsigned char *Source, int length, unsigned char Key[]);
void DeCryptWith3DES_java(unsigned char *Source, int length, unsigned char key[], std::string & output);
void EnCryptWith3DES_java(const std::string & src, unsigned char Key[], std::string & output);
void krys3des_encryption (const std::string & source, const char* key24bytes, std::string & output);
void krys3des_decryption (const std::string & input, const char* key24bytes, std::string & output);


void percent_code (const char* input, int len, std::string& output);
int gbk_len (const std::string& gbk);

#ifdef __cplusplus
extern "C" {
#endif

/*----------------------------------function definitions-------------------------------------------*/


int krys3des_raw_encryption (const void* src, int input_length, void* key24bytes, void* out_buffer);
int krys3des_raw_decryption (const void* src, unsigned int input_length, void* key24bytes, void*out_buffer);

/*----------------------------------------------END------------------------------------------------*/



#ifdef __cplusplus
}
#endif


void base64_encode(unsigned char const* bytes_to_encode, unsigned int in_len, std::string & ret);
void base64_decode(std::string const& encoded_string, std::string & ret);


#endif /*CIPHER_H*/
