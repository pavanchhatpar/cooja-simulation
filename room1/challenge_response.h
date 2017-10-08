#define KEY 3
static int cr[50] = {50, 15, 93, 83, 32, 50, 63, 21, 42, 25, 16, 97, 73, 52, 14, 64, 64, 16, 32, 31, 12, 55, 20, 92, 40, 97, 69, 21, 22, 57, 81, 11, 95, 87, 18, 93, 63, 70, 17, 78, 83, 17, 28, 61, 59, 78, 44, 16, 92, 90};
static int ord[5] = {0, 4, 2, 1, 3};

static char encrypt(char c) {
	return ((c - '0') + KEY) % 10 + '0';
}

static char decrypt(char c) {
	int t = ((c - '0') - KEY) % 10;
	if (t < 0) {
		t += 10;	
	}
	return t + '0';
}
