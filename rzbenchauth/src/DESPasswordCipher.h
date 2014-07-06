#ifndef DESPASSWORDCIPHER_H
#define DESPASSWORDCIPHER_H

#include <stdlib.h>
#include <openssl/des.h>
#include <string.h>

class DESPasswordCipher
{
	public:
//		DesPasswordCipher(const char *password = NULL) { if(password) init(password); }

//		void init(const char *password);
//		bool encrypt(void *buf, int len);
//		bool decrypt(void *buf, int len);

		DESPasswordCipher(const char *password = NULL) { if(password) init(password); }

		void init(const char *password) {
			unsigned char    key64bit[8];
			unsigned char   *key ;
			int     i;

			memset(key64bit, 0, sizeof(key64bit));

			key = key64bit ;
			for(i = 0; *password && i < 40; i++) {
				key[i % 8] ^= *password ;
				password++ ;
			}

			DES_set_odd_parity(&key64bit);
			DES_set_key_checked(&key64bit, &keySchedule);

		}

		bool encrypt(void *buf, int len) {
			len = len & 0xfffffff8;
			unsigned char desBlock[8];
			unsigned char *ptr = (unsigned char *)buf;
			for (; len > 0; len -= 8, ptr += 8) {                // encrypt/decrypt 1 buffer-full 8-byte blocks
				memcpy(desBlock, ptr, 8);
				DES_ecb_encrypt(&desBlock, &desBlock, &keySchedule, DES_ENCRYPT);
				memcpy(ptr, desBlock, 8);
			}
			return true;
		}

		bool decrypt(void *buf, int len) {
			len = len & 0xfffffff8;
			unsigned char desBlock[8];
			unsigned char *ptr = (unsigned char *)buf;
			for (; len > 0; len -= 8, ptr += 8) {                // encrypt/decrypt 1 buffer-full 8-byte blocks
				memcpy(desBlock, ptr, 8);
				DES_ecb_encrypt(&desBlock, &desBlock, &keySchedule, DES_DECRYPT);
				memcpy(ptr, desBlock, 8);
			}
			return true;
		}


	private:
		DES_key_schedule keySchedule;
/*
		// End of DES algorithm (except for calling desinit below)
		void DesMem( void *buf, int mlen, int isencrypting );

		// 32-bit permutation at end
		void perm32(char *inblock, char *outblock);

		// contract f from 48 to 32 bits
		void contract(char *in48, char *out32);

		// critical cryptographic trans
		void f(char *right, int num, char *fret);

		// 1 churning operation
		void iter(int num, char *inblock, char *outblock);

		// initialize s1-s8 arrays
		void sinit(void);

		// initialize key schedule array
		void kinit(char *key64bit);

		// initialize 32-bit permutation
		void p32init(void);

		// encrypt 64-bit inblock
		void endes(char *inblock, char *outblock);

		// decrypt 64-bit inblock
		void dedes(char *inblock, char *outblock);

		// inital and final permutations
		char m_iperm[16][16][8];
		char m_fperm[16][16][8];

		char m_s[4][4096];				// S1 thru S8 precomputed
		char m_p32[4][256][4];			// for permuting 32-bit f output
		char m_kn[16][6];				// key selections
*/

};


#endif // DESPASSWORDCIPHER_H
