/*
 * A simple implementation of the XOR Cipher, with addition functions to read and write hex cipher text.
 * See https://en.wikipedia.org/wiki/XOR_cipher for an explanation.
 *
 * Two constructors are available:
 *	 * A file name and a key length - if the file does not exist, the key is randomly generated and saved to the file.
 * 	 * A key specified in hex form directly.
 * 
 * Only plain/cipher text less than or equal to the key length can be encrypted / decrypted.
 *
 * Note, protect the key if you don't want someone to be able to decrypt a string.
 * 
 * Author bluap/pjbroad March 2019.
*/

#include <iostream>
#include <vector>
#include <sstream>
#include <string>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <fstream>
#include <cassert>

#include "xor_cipher.hpp"

#ifdef ELC
#include "elloggingwrapper.h"
#endif

namespace XOR_Cipher
{
	//	A wrapper around the EL client LOG_ERROR() function.
	static void show_error(const char * function_name, const std::string& message)
	{
#ifdef ELC
		LOG_ERROR("%s: %s\n", function_name, message.c_str());
#else
		std::cerr << function_name << ": " << message << std::endl;
#endif
	}

	//	Contruct using a pre created key, specified as hex
	//
	Cipher::Cipher(const std::string& key_hex_str) : status_ok(true)
	{
		key = hex_to_cipher(key_hex_str);
	}

	//	Construct using a key stored in a file.  If the file does not exist, create a randon key and save to the file.
	//
	Cipher::Cipher(const std::string& file_name, size_t the_key_size) : key_file_name(file_name), status_ok(true)
	{
		std::ifstream in(key_file_name.c_str());
		if (!in)
		{
			srand (time(NULL));
			for (size_t i=0; i<the_key_size; ++i)
				key.push_back(rand()%256);
			std::ofstream out(key_file_name.c_str(), std::ios_base::out | std::ios_base::trunc);
			if (out)
			{
				out << cipher_to_hex(key) << std::endl;
				out.close();
			}
			else
			{
				show_error(__PRETTY_FUNCTION__, std::string("Failed to created new ciper key file ") + key_file_name);
				status_ok = false;
			}
			return;
		}
		std::string key_hex_str;
		in >> key_hex_str;
		in.close();
		key = hex_to_cipher(key_hex_str);
		if (key.size() != the_key_size)
		{
			show_error(__PRETTY_FUNCTION__, std::string("Key from file does not match required size"));
			status_ok = false;
		}
	}

	//	Convert a binary cipher text vector to a hex string
	//
	std::string Cipher::cipher_to_hex(const std::vector<unsigned char>& cipher_text) const
	{
		std::stringstream ss;
		for (size_t i=0; i<cipher_text.size(); ++i)
			ss << std::setfill('0') << std::setw(2) << std::hex << static_cast<int>(cipher_text[i]);
		return ss.str();
	}

	//	Convert a validated hex string to a binary vector for use in this module
	//
	std::vector<unsigned char> Cipher::hex_to_cipher(const std::string& hex_str)
	{
		std::vector <unsigned char> cipher_text;
		if (((hex_str.size() % 2) == 0) && (hex_str.find_first_not_of("0123456789abcdefABCDEF", 0) == std::string::npos))
			for (size_t i=0; i<hex_str.size(); i+=2)
			{
				int tmp;
				std::stringstream ss;
				ss << std::hex << hex_str.substr(i, 2);
				ss >> tmp;
				cipher_text.push_back(static_cast<unsigned char>(tmp));
			}
		else
		{
			show_error(__PRETTY_FUNCTION__, std::string("Not valid hex string [") + hex_str + std::string("]"));
			status_ok = false;
		}
		return cipher_text;
	}

	//	Return the encryted cipher text for the specifed string
	//
	std::vector<unsigned char> Cipher::encrypt(const std::string& plain_text)
	{
		std::vector<unsigned char> cipher_text;
		if (plain_text.size() <= key.size())
			for (size_t i=0; i<plain_text.size(); ++i)
				cipher_text.push_back(static_cast<unsigned char>(plain_text[i]) ^ key[i]);
		else
		{
			show_error(__PRETTY_FUNCTION__, std::string("Plain text too long [") + plain_text + std::string("]"));
			status_ok = false;
		}
		return cipher_text;
	}

	//	Return the plan text string of the specified cipher text
	//
	std::string Cipher::decrypt(const std::vector<unsigned char>& cipher_text)
	{
		std::string plain_text;
		if (cipher_text.size() <= key.size())
			for (size_t i=0; i<cipher_text.size(); ++i)
				plain_text.push_back(static_cast<char>(cipher_text[i]) ^ key[i]);
		else
		{
			show_error(__PRETTY_FUNCTION__, std::string("Cypher text too long [") + cipher_to_hex(cipher_text) + std::string("]"));
			status_ok = false;
		}
		return plain_text;
	}
}

//	Unit testing
//	g++ XOR_cipher.cpp
//	./a.out
//
#ifndef ELC
int main(int argc, char *argv[])
{
	if (argc > 1)
	{
		std::string help(std::string(argv[0]) + " <-d | -e> <key hex string> <cypher text hex string | message>");
		if (argc > 3)
		{
			XOR_Cipher::Cipher cipher(argv[2]);
			if (std::string(argv[1]) == "-e")
				std::cout << cipher.cipher_to_hex(cipher.encrypt(argv[3])) << std::endl;
			else if (std::string(argv[1]) == "-d")
				std::cout << cipher.decrypt(cipher.hex_to_cipher(argv[3])) << std::endl;
			else
				std::cerr << help << std::endl;
		}
		else
			std::cerr << help << std::endl;
	}
	else
	{
		std::cout << "Unit tests....." << std::endl;
		
		std::string key_file_name("XOR_cipher_test_key_for_testing");
		static size_t key_size = 32;

		XOR_Cipher::Cipher f_cipher(key_file_name, key_size);
		assert(f_cipher.get_status_ok());
		assert(std::ifstream(key_file_name.c_str()).good());
		std::string f_message("My_secret");
		std::vector<unsigned char> f_cipher_text = f_cipher.encrypt(f_message);
		assert(f_message == f_cipher.decrypt(f_cipher_text));
		assert(f_cipher.get_status_ok());

		XOR_Cipher::Cipher f_2_cipher(key_file_name, key_size);
		assert(f_2_cipher.get_status_ok());
		remove(key_file_name.c_str());

		XOR_Cipher::Cipher cipher("0123456789abcdef0123456789abcdef");
		assert(cipher.get_status_ok());

		assert(cipher.encrypt("this is a long long long long long long long long long long long long test, too long").size() == 0);
		assert(!cipher.get_status_ok());
		cipher.set_status_ok();

		std::vector<unsigned char> too_big_cipher;
		for (size_t i=0; i<key_size + 1; ++i)
			too_big_cipher.push_back(32);
		assert(cipher.decrypt(too_big_cipher).empty());
		assert(!cipher.get_status_ok());
		cipher.set_status_ok();

		assert(cipher.encrypt("this is a test").size() != 0);
		assert(cipher.get_status_ok());

		assert(cipher.hex_to_cipher("123").size() == 0);
		assert(!cipher.get_status_ok());
		cipher.set_status_ok();
		assert(cipher.hex_to_cipher("efgh").size() == 0);
		assert(!cipher.get_status_ok());
		cipher.set_status_ok();

		std::string message("My_secret");
		std::vector<unsigned char> cipher_text = cipher.encrypt(message);
		assert(cipher.get_status_ok());
		std::string cipher_hex = cipher.cipher_to_hex(cipher_text);
		assert(cipher.get_status_ok());

		assert(message == cipher.decrypt(cipher_text));
		assert(cipher.get_status_ok());
		assert(cipher_text == cipher.hex_to_cipher(cipher_hex));
		assert(cipher.get_status_ok());
		assert(message == cipher.decrypt(cipher.hex_to_cipher(cipher_hex)));
		assert(cipher.get_status_ok());

		std::cout << "All PASS" << std::endl;
	}
  
	return 0;
}
#endif
