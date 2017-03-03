#include "AesPasswordCipher.h"
#include <openssl/evp.h>
#include <string.h>
#include <stdlib.h>
#include "Core/ScopeGuard.h"

AesPasswordCipher::AesPasswordCipher()
    : evpCipher(nullptr, &EVP_CIPHER_CTX_free)
{
}

AesPasswordCipher::~AesPasswordCipher() {
}

void AesPasswordCipher::init(const uint8_t* key) {
	memcpy(this->key, key, 32);
}

void AesPasswordCipher::init() {
	for(size_t i = 0; i < sizeof(key); i++)
		key[i] = rand();
}

void AesPasswordCipher::getKey(std::vector<uint8_t>& key) {
	key.assign(this->key, this->key + sizeof(this->key));
}

bool AesPasswordCipher::encrypt(const std::vector<uint8_t>& input, std::vector<uint8_t>& output) {
	if(!evpCipher)
		evpCipher.reset(EVP_CIPHER_CTX_new());

	guard_on_exit(evpReset, [this] {
		EVP_CIPHER_CTX_reset(evpCipher.get());
	});

	if(EVP_EncryptInit_ex(evpCipher.get(), EVP_aes_128_cbc(), nullptr, key, key + 16) <= 0)
		return false;

	int bytesWritten = 0;
	size_t totalLength = 0;
	int blockSize = EVP_CIPHER_CTX_block_size(evpCipher.get());
	output.resize(input.size() + blockSize - 1);

	if(EVP_EncryptUpdate(evpCipher.get(), &output[0], &bytesWritten, &input[0], input.size()) <= 0)
		return false;

	totalLength += bytesWritten;

	if(EVP_EncryptFinal_ex(evpCipher.get(), &output[0] + totalLength, &bytesWritten) <= 0)
		return false;

	totalLength += bytesWritten;
	output.resize(totalLength);

	return true;
}

bool AesPasswordCipher::decrypt(const std::vector<uint8_t>& input, std::vector<uint8_t>& output) {
	if(!evpCipher)
		evpCipher.reset(EVP_CIPHER_CTX_new());

	guard_on_exit(evpReset, [this] {
		EVP_CIPHER_CTX_reset(evpCipher.get());
	});

	if(EVP_DecryptInit_ex(evpCipher.get(), EVP_aes_128_cbc(), nullptr, key, key + 16) <= 0)
		return false;

	int bytesWritten = 0;
	size_t totalLength = 0;
	int blockSize = EVP_CIPHER_CTX_block_size(evpCipher.get());
	output.resize(input.size() + blockSize - 1);

	if(EVP_DecryptUpdate(evpCipher.get(), &output[0], &bytesWritten, &input[0], input.size()) <= 0)
		return false;

	totalLength += bytesWritten;

	if(EVP_DecryptFinal_ex(evpCipher.get(), &output[0] + totalLength, &bytesWritten) <= 0)
		return false;

	totalLength += bytesWritten;
	output.resize(totalLength);

	return true;
}
