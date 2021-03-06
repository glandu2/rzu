#pragma once

#include "../Extern.h"
#include "Core/Object.h"
#include <memory>
#include <stddef.h>  // for size_t
#include <stdint.h>
#include <vector>

typedef struct rsa_st RSA;

class RZU_EXTERN RsaCipher : public Object {
public:
	RsaCipher();
	~RsaCipher();

	bool loadKey(const std::vector<uint8_t>& pemKey);
	bool getPemPublicKey(std::vector<uint8_t>& outKey);
	int generateKey();
	bool isInitialized();

	bool publicEncrypt(const uint8_t* input, size_t input_size, std::vector<uint8_t>& output);
	bool publicDecrypt(const uint8_t* input, size_t input_size, std::vector<uint8_t>& output);
	bool privateEncrypt(const uint8_t* input, size_t input_size, std::vector<uint8_t>& output);
	bool privateDecrypt(const uint8_t* input, size_t input_size, std::vector<uint8_t>& output);

protected:
	void printError();

private:
	std::unique_ptr<RSA, void (*)(RSA*)> rsaCipher;
};

