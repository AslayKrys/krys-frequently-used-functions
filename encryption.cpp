#include "des.h"
#include <string>
#include <cstring>
#include <unistd.h>
#include <stdlib.h>
#include <cstdio>
#include <memory>

#include "encryption.h"


int krys3des_raw_encryption (const void* src, int input_length, void* key24bytes, void* out_buffer)
{
	if (src == nullptr or input_length <= 0 or key24bytes == nullptr or out_buffer == nullptr)
	{
		errno = EINVAL;
		return -1;
	}

	/*------------------------------generating des3 context--------------------------------------------*/
	des3_context context;
	int des3_length;
	/*-------------------------------------------------------------------------------------------------*/


	/*-------------------------------------------init data---------------------------------------------*/
	des3_set_3keys (&context, (unsigned char*)key24bytes);
	des3_length = input_length + (8 - input_length % 8);
	auto source_3des = std::make_unique<char[]> (des3_length);		/*des3 formatted input*/
	/*----------------------------------------------END------------------------------------------------*/


	/*--------------------------------------------encryption-------------------------------------------*/
	memcpy (source_3des.get(), src, input_length);
	memset (source_3des.get() + input_length, des3_length - input_length, des3_length - input_length);

	for (int i = 0; i < des3_length; i += 8)
	{
		des3_encrypt (&context, (unsigned char*)source_3des.get () + i, (unsigned char*)out_buffer + i);
	}
	/*----------------------------------------------END------------------------------------------------*/

	return des3_length;
}


/*
 *
 *	out_buffer's length must be the same as the input buffer (aka src)
 *	
 *
 *
 *
 *
 *
 *
 */

int krys3des_raw_decryption (const void* src, unsigned int input_length, void* key24bytes, void*out_buffer)
{
	if (src == nullptr  or input_length == 0 or input_length % 8 != 0 
			or key24bytes == nullptr or out_buffer == nullptr)
	{
		errno = EINVAL;
		return -1;
	}

	des3_context context;
	des3_set_3keys (&context, (const unsigned char*)key24bytes);

	for (unsigned int i = 0; i < input_length; i += 8)
	{
		des3_decrypt (&context, (unsigned char*)src + i, (unsigned char*)out_buffer + i);
	}

    if (((unsigned char*)out_buffer)[input_length - 1] < 1 or ((unsigned char*)out_buffer)[input_length - 1] > 8)
	{
		errno = EINVAL;
		return -1;
	}

	for (char* p = &((char*)out_buffer)[input_length - 1]; 
			p > &((char*)out_buffer)[input_length - 1 - ((char*)out_buffer)[input_length - 1] ]; p--)
	{
		if (*p != ((char*)out_buffer)[input_length - 1])
		{
			errno = EINVAL;
			return -1;
		}
	}


	return input_length - ((unsigned char*)out_buffer)[input_length - 1];
}




void krys3des_decryption (const std::string & input, const char* key24bytes, std::string & output)
{
	
	/*--------------------------------------------init data--------------------------------------------*/
	des3_context context;
	des3_set_3keys (&context, (const unsigned char*)key24bytes);
	int length = input.length()/2;

	char tmp[3];
	tmp[2] = 0;

	auto source_3des = std::make_unique<char[]>(length);
	auto output_3des = std::make_unique<char[]>(length);
	/*----------------------------------------------END------------------------------------------------*/



	/*------------------------------------------length examine-----------------------------------------*/
	if (length % 8 != 0 or length == 0)
	{
		output.clear();
		return;
	}
	/*----------------------------------------------END------------------------------------------------*/


	/*---------------------------------hex to bytestream converting------------------------------------*/
	for (unsigned long i=0; i<input.size()/2; i++)
	{
		strncpy (tmp, input.c_str() + 2 * i, 2);
		sscanf (tmp, "%hhx", source_3des.get() + i);
	}
	/*----------------------------------------------END------------------------------------------------*/


	/*-------------------------------------------decryption loop---------------------------------------*/
	for (int i = 0; i < length; i += 8)
	{
		des3_decrypt (&context, (unsigned char*)source_3des.get() + i, (unsigned char*)output_3des.get() + i);
	}
	/*----------------------------------------------END------------------------------------------------*/

	/*-----------------------------------------validating result---------------------------------------*/
    if (output_3des[length - 1] < 1 or output_3des[length - 1] > 8)
	{
		output = "";
		return;
	}

	for (char* p = &output_3des[length - 1]; p > &output_3des[length - 1 - output_3des[length - 1] ]; p--)
	{
		if (*p != output_3des[length - 1])
		{
			output = "";
			return;
		}
	}
	/*----------------------------------------------END------------------------------------------------*/


	/*---return value----------------------------------------------------------------------------------*/
	output_3des[length - output_3des[length - 1] ] = '\0';
	output = output_3des.get();
	/*----------------------------------------------END------------------------------------------------*/

}

void base64_encode(unsigned char const* bytes_to_encode, unsigned int in_len, std::string & ret) 
{
	std::string base64_chars = 
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz"
		"0123456789+/";

	int i = 0;
	int j = 0;
	unsigned char char_array_3[3];
	unsigned char char_array_4[4];

	while (in_len--) 
	{
		char_array_3[i++] = *(bytes_to_encode++);
		if (i == 3) 
		{
			char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
			char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
			char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
			char_array_4[3] = char_array_3[2] & 0x3f;

			for(i = 0; (i <4) ; i++)
			  ret += base64_chars[char_array_4[i]];
			i = 0;
		}
	}

	if (i)
	{
		for(j = i; j < 3; j++)
		  char_array_3[j] = '\0';

		char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
		char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
		char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
		char_array_4[3] = char_array_3[2] & 0x3f;

		for (j = 0; (j < i + 1); j++)
		  ret += base64_chars[char_array_4[j]];

		while((i++ < 3))
		  ret += '=';

	}

}

void base64_decode(std::string const& encoded_string, std::string & ret) 
{

	std::string base64_chars = 
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz"
		"0123456789+/";

	int in_len = encoded_string.size();
	int i = 0;
	int j = 0;
	int in_ = 0;
	unsigned char char_array_4[4], char_array_3[3];

	while (in_len-- && ( encoded_string[in_] != '=') && is_base64(encoded_string[in_])) 
	{
		char_array_4[i++] = encoded_string[in_]; in_++;
		if (i ==4) 
		{
			for (i = 0; i <4; i++)
			  char_array_4[i] = base64_chars.find(char_array_4[i]);

			char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
			char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
			char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

			for (i = 0; (i < 3); i++)
			  ret += char_array_3[i];
			i = 0;
		}
	}

	if (i) 
	{
		for (j = i; j <4; j++)
		  char_array_4[j] = 0;

		for (j = 0; j <4; j++)
		  char_array_4[j] = base64_chars.find(char_array_4[j]);

		char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
		char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
		char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

		for (j = 0; (j < i - 1); j++) ret += char_array_3[j];
	}
}

void krys3des_encryption (const std::string & source, const char* key24bytes, std::string & output)
{
	if (key24bytes == nullptr)
	{
		errno = EINVAL;
		output.clear();
		return;
	}

	/*------------------------------generating des3 context--------------------------------------------*/
	des3_context context;
	int input_length;
	int des3_length;
	/*-------------------------------------------------------------------------------------------------*/


	/*-------------------------------------------init data---------------------------------------------*/
	des3_set_3keys (&context, (unsigned char*)key24bytes);
	input_length = source.length();
	des3_length = input_length + (8 - input_length % 8);

	char hexbyte[3];  /*hex temp*/
	output.clear();

	auto source_3des = std::make_unique<char[]> (des3_length);		/*des3 formatted input*/
	auto output_3des = std::make_unique<char[]> (des3_length);		/*des3 formatted output*/
	/*----------------------------------------------END------------------------------------------------*/



	/*-----------------------------------set memory with fixed length----------------------------------*/
	memcpy (source_3des.get(), source.c_str(), input_length);
	memset (source_3des.get() + input_length, des3_length - input_length, des3_length - input_length);
	/*----------------------------------------------END------------------------------------------------*/


	/*-------------------------------------encryption blockwise----------------------------------------*/
	for (int i = 0; i < des3_length; i += 8)
	{
		des3_encrypt (&context, (unsigned char*)source_3des.get () + i, (unsigned char*)output_3des.get() + i);
	}
	/*----------------------------------------------END------------------------------------------------*/


	/*--------------------------------------------hex output-------------------------------------------*/
	for (int i = 0; i < des3_length; i++)
	{
		sprintf (hexbyte, "%02hhx", (unsigned char)output_3des[i]);
		output += hexbyte;
	}
	/*----------------------------------------------END------------------------------------------------*/

}