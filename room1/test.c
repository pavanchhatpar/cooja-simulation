#include <stdio.h>
#include <string.h>
#include "challenge_response.h"

int main() {
	int i;	
	char *hash_inp[3] = {"132|31", "0", "123456789"};
	long hash_exp[3] = {33331376034, 173811, 18429362615268};
	for(i = 0; i < 3; i++) {
		printf("Hash of %s Expected: %ld Actual: %ld\n",hash_inp[i], hash_exp[i], gen_hash(hash_inp[i]));
	}
	char enc_inp[3] = {'5', '0', '9'};
	char dec_inp[3] = {'8', '3', '2'};
	for(i = 0; i < 3; i++) {
		printf("Encrytion of %c Expected: %c Actual: %c\n", enc_inp[i], dec_inp[i], encrypt(enc_inp[i]));
	}

	for(i = 0; i < 3; i++) {
		printf("Decryption of %c Expected: %c Actual: %c\n",dec_inp[i], enc_inp[i], decrypt(dec_inp[i]));
	}
	return 0;
}
