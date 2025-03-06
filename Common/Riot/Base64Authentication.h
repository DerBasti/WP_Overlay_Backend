#ifndef __LEAGUE_BASE64_AUTH__
#define __LEAGUE_BASE64_AUTH__

#include <inttypes.h>
#include <algorithm>

class Base64Character {
private:
	uint8_t baseChar;
	bool filledFlag;

	void convertToBase64(uint8_t base64Index) {
		if (base64Index >= 0 && base64Index <= 0x19) {
			baseChar = 'A' + base64Index;
		}
		else if (base64Index >= 0x1A && base64Index <= 0x33) {
			baseChar = 'a' + (base64Index - 0x1A);
		}
		else if (base64Index >= 0x34 && base64Index <= 0x3D) {
			baseChar = '0' + (base64Index - 0x34);
		}
		else if (base64Index == 0x3E) {
			baseChar = '+';
		}
		else if (base64Index == 0x3F) {
			baseChar = '/';
		}
		if (filledFlag) {
			baseChar = '=';
		}
	}
public:
	Base64Character() : Base64Character(0, true) {

	}
	Base64Character(uint8_t num) : Base64Character(num, false) {}
	Base64Character(uint8_t num, bool zeroFill) {
		filledFlag = zeroFill;
		convertToBase64(num);
	}

	__inline operator char() const {
		return getBase64Character();
	}

	__inline char getBase64Character() const {
		return static_cast<char>(baseChar);
	}
};

class Base64Block {
private:
	uint32_t blockOf24Bits;
	Base64Character convertedBytes[4];
	constexpr static uint8_t FIVE_BITS_ONEFILLED = (1 << 6) - 1;
	constexpr static uint32_t TWENTYFOUR_BITS_ONEFILLED = (1 << 24) - 1;
public:
	Base64Block(uint32_t blockOf24Bits) : Base64Block(blockOf24Bits, -1) {
	}
	Base64Block(uint32_t blockOf24Bits, uint8_t paddingByteStartPosition) {
		this->blockOf24Bits = blockOf24Bits & TWENTYFOUR_BITS_ONEFILLED;
		for (uint8_t i = 0; i < 4; i++) {
			uint8_t rightShiftInBits = (18 - (6 * i));
			int currentBlockAsSixBits = blockOf24Bits >> rightShiftInBits;
			currentBlockAsSixBits &= FIVE_BITS_ONEFILLED;
			convertedBytes[i] = std::move(Base64Character(currentBlockAsSixBits, i > paddingByteStartPosition && currentBlockAsSixBits == 0));
		}
	}
	__inline const Base64Character& getCharacter(size_t index) const {
		return convertedBytes[index];
	}
};

class Base64 {
private:
	char *initialData;
	size_t initialDataLen;
	char *converted;
	size_t convertedLen;
	constexpr static uint8_t DEFAULT_BASE64_BLOCK_LENGTH = 4;
	constexpr static uint8_t DEFAULT_BLOCK_LENGTH_IN_BYTES = 3;
	constexpr static uint8_t BITS_PER_BYTE = 8;

	int calculateNumberFromBytes(char*& iterator) {
		int num = (*iterator) << (BITS_PER_BYTE * 2);
		iterator++;
		num += (*iterator) << BITS_PER_BYTE;
		iterator++;
		num += (*iterator);
		iterator++;
		return num;
	}

	void addConversion(int num, size_t& idx) {
		Base64Block block(num);

		converted[idx++] = block.getCharacter(0);
		converted[idx++] = block.getCharacter(1);
		converted[idx++] = block.getCharacter(2);
		converted[idx++] = block.getCharacter(3);
	}

	void handlePadding(char *iterator) {
		int bytesLeft = initialDataLen % DEFAULT_BLOCK_LENGTH_IN_BYTES;
		int padding = DEFAULT_BLOCK_LENGTH_IN_BYTES - bytesLeft;
		int num = 0;
		for (int32_t i = 0; i < bytesLeft; i++) {
			num <<= BITS_PER_BYTE;
			num += *iterator;
			iterator++;
		}
		num <<= (padding * BITS_PER_BYTE);
		size_t idx = convertedLen - DEFAULT_BASE64_BLOCK_LENGTH;
		addConversion(num, idx);
	}

	void encode() {
		char *iterator = initialData;
		size_t iteratorIndex = 0;
		size_t converterIndex = 0;
		while (*iterator && (iteratorIndex + DEFAULT_BLOCK_LENGTH_IN_BYTES) <= initialDataLen) {
			int num = calculateNumberFromBytes(iterator);
			addConversion(num, converterIndex);
			iteratorIndex += DEFAULT_BLOCK_LENGTH_IN_BYTES;
		}
		if (iteratorIndex != initialDataLen) {
			handlePadding(iterator);
		}
		converted[convertedLen] = 0x00;
	}
public:
	Base64(const char* initial) {
		initialDataLen = strlen(initial);
		initialData = new char[initialDataLen + 1];
		strncpy_s(initialData, initialDataLen + 1, initial, initialDataLen);
		initialData[initialDataLen] = 0x00;

		convertedLen = (DEFAULT_BASE64_BLOCK_LENGTH * initialDataLen) / DEFAULT_BLOCK_LENGTH_IN_BYTES;
		if (convertedLen % DEFAULT_BASE64_BLOCK_LENGTH != 0) {
			convertedLen += DEFAULT_BASE64_BLOCK_LENGTH - (convertedLen % DEFAULT_BASE64_BLOCK_LENGTH);
		}
		converted = new char[convertedLen + 1];

		encode();
	}
	virtual ~Base64() {
		delete[] initialData;
		initialData = nullptr;

		delete[] converted;
		converted = nullptr;
	}

	__inline const char* getBase64String() const {
		return converted;
	}
};

#endif 