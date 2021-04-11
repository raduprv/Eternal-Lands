#ifndef __XOR_CIPHER_HH
#define	__XOR_CIPHER_HH

#include <vector>
#include <string>

namespace XOR_Cipher
{
	class Cipher
	{
		public:
			Cipher(const std::string& key_hex_str);
			Cipher(const std::string& file_name, size_t the_key_size);
			std::vector<unsigned char> encrypt(const std::string& plain_text);
			std::string decrypt(const std::vector<unsigned char>& cipher_text);
			std::string cipher_to_hex(const std::vector<unsigned char>& cipher_text) const;
			std::vector<unsigned char> hex_to_cipher(const std::string& hex_str);
			bool get_status_ok(void) const { return status_ok; }
			void set_status_ok(void) { status_ok = true; }
		private:
			std::string key_file_name;
			bool status_ok;
			std::vector<unsigned char> key;
	};
}

#endif
