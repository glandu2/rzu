#ifndef DESPASSWORDCIPHER_H
#define DESPASSWORDCIPHER_H

#include "../Extern.h"

class RZU_EXTERN DesPasswordCipher {
public:
	DesPasswordCipher(const char* password = nullptr);
	~DesPasswordCipher();

	void init(const char* password);
	bool encrypt(void* buf, int len);
	bool decrypt(void* buf, int len);

private:
	void* const keySchedule;
};

#endif  // DESPASSWORDCIPHER_H
