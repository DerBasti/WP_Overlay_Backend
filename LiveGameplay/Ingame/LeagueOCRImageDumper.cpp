#include "LeagueOCRImageDumper.h"
#include "LeagueOCRPixelData.h"
#include "LeagueMoneyOCRBoundingBox.h"
#include <stack>
#include <functional>

LeagueOCRImageDumper::LeagueOCRImageDumper(const LeagueOCRPixelData* pixelData, TeamType teamType) {
	this->pixelData = pixelData;
	this->teamType = teamType;
}

LeagueOCRImageDumper::~LeagueOCRImageDumper() {

}

bool LeagueOCRImageDumper::dumpImage(const wchar_t* outputPath, LeagueOCRImageDumpType dumpType) const {
	auto bitmapInfo = createBitmapInfoStruct(pixelData->getHBitmap());
	HANDLE outputFile = createAndPrepareOutputFile(outputPath, bitmapInfo);
	if (!outputFile) {
		return false;
	}

	uint32_t* imageBytes = pixelData->getPixelMemoryAsSinglechannel();
	uint32_t dataLength = pixelData->getWidth() * pixelData->getHeight();

	uint32_t* copiedPixel = new uint32_t[dataLength];
	memcpy(copiedPixel, imageBytes, dataLength * sizeof(uint32_t));
	uint32_t startX = 0;
	DWORD dwTmp;

	LeagueOCRPixelData copyData((uint8_t*)copiedPixel, pixelData->getOCRRect());
	LetterBoundingBoxScanner boundingBoxScanner(&copyData, (teamType == TeamType::BLUE) ? LetterBoundingBoxColor::BLUE : LetterBoundingBoxColor::RED);
	auto boundingBoxes = boundingBoxScanner.detectBoundingBoxes(LeagueOCRLocaleDetectionMode::OCR_DETECTION_WESTERN_RAYFILL);

	for (auto boundingBox : boundingBoxes) {
		switch (dumpType) {
			case LeagueOCRImageDumpType::WITH_FILL_ONLY:
				spanFill(&copyData, boundingBoxScanner.findFirstValidPixelInsideBoundingBox(&boundingBox));
			break;
			case LeagueOCRImageDumpType::WITH_BOUNDING_BOXES_ONLY:
				drawBoundingBox(&copyData, &boundingBox);
			break;
		}
	}

	size_t cb = bitmapInfo->bmiHeader.biSizeImage;
	byte* hp = (uint8_t*)copiedPixel;
	if (!WriteFile(outputFile, (LPSTR)hp, (int)cb, (LPDWORD)&dwTmp, NULL)) {
		CloseHandle(outputFile);
		return false;
	}

	// Close the .BMP file.  
	if (!CloseHandle(outputFile)) {
		return false;
	}
	return true;
}


void LeagueOCRImageDumper::spanFill(LeagueOCRPixelData* copiedPixelData, std::pair<uint32_t, uint32_t> firstValidPixel) const {
	if (firstValidPixel.first < 0 || firstValidPixel.first >= copiedPixelData->getWidth() || firstValidPixel.second < 0 || firstValidPixel.second >= copiedPixelData->getHeight()) {
		return;
	}
	std::stack<RECT> spanStack;
	auto rect = RECT{ (LONG)firstValidPixel.first, (LONG)firstValidPixel.second, 0, 0 };
	spanStack.push(RECT{ rect.left, rect.top, rect.left, 1 });
	spanStack.push(RECT{ rect.left, rect.top - 1, rect.left, -1 });

	const uint32_t PIXEL_SET_COLOR = 0xFFFF00;
	std::function<uint32_t(LONG, LONG)> ExtractPixelColor = [&](LONG x, LONG y) {
		return copiedPixelData->getPixelMemoryAsSinglechannel()[(copiedPixelData->getHeight() - y - 1) * copiedPixelData->getWidth() + x] & 0xFFFFFF;
	};

	auto dominantColorToLookFor = (teamType == TeamType::BLUE) ? LetterBoundingBoxColor::BLUE : LetterBoundingBoxColor::RED;
	std::function<bool(LONG, LONG)> isInside = [&](LONG x, LONG y) {
		bool pixelSet = (ExtractPixelColor(x, y) & PIXEL_SET_COLOR) == PIXEL_SET_COLOR;
		bool validColor = LetterBoundingBoxScanner::IsValidColorAtPosition(copiedPixelData, dominantColorToLookFor, x, y);
		return !pixelSet && validColor;
	};

	RECT expectedBox = RECT{ rect.left, rect.top, 0, 0 };
	while (!spanStack.empty()) {
		auto currentPosition = spanStack.top();
		spanStack.pop();
		LONG x = currentPosition.left;
		if (isInside(x, currentPosition.top)) {
			while (isInside(x - 1, currentPosition.top)) {
				x--;
				drawColorAtPosition(copiedPixelData, PIXEL_SET_COLOR, x, currentPosition.top);
			}
			if (x < currentPosition.left) {
				spanStack.push(RECT{ x, currentPosition.top - currentPosition.bottom, currentPosition.left - 1, -currentPosition.bottom });
			}
		}

		if (currentPosition.left < expectedBox.left) {
			expectedBox.left = currentPosition.left;
		}
		if (currentPosition.top < expectedBox.top) {
			expectedBox.top = currentPosition.top;
		}
		if (currentPosition.left > expectedBox.right) {
			expectedBox.right = currentPosition.left;
		}
		if (currentPosition.top > expectedBox.bottom) {
			expectedBox.bottom = currentPosition.top;
		}
		while (currentPosition.left <= currentPosition.right) {
			while (isInside(currentPosition.left, currentPosition.top)) {
				drawColorAtPosition(copiedPixelData, PIXEL_SET_COLOR, currentPosition.left, currentPosition.top);
				currentPosition.left++;
			}
			if (currentPosition.left > x) {
				spanStack.push(RECT{ x, currentPosition.top + currentPosition.bottom, currentPosition.left - 1, currentPosition.bottom });
			}
			if ((currentPosition.left - 1) > currentPosition.right) {
				spanStack.push(RECT{ currentPosition.right, currentPosition.top - currentPosition.bottom, currentPosition.left - 1, -currentPosition.bottom });
			}
			currentPosition.left++;
			while (currentPosition.left < currentPosition.right && !isInside(currentPosition.left, currentPosition.top)) {
				currentPosition.left++;
			}
			x = currentPosition.left;
		}

		if (currentPosition.left < expectedBox.left) {
			expectedBox.left = currentPosition.left;
		}
		if (currentPosition.top < expectedBox.top) {
			expectedBox.top = currentPosition.top;
		}
		if (currentPosition.left > expectedBox.right) {
			expectedBox.right = currentPosition.left;
		}
		if (currentPosition.top > expectedBox.bottom) {
			expectedBox.bottom = currentPosition.top;
		}
	}
	expectedBox.left = (std::max)(0l, expectedBox.left - 1);
	expectedBox.top = (std::max)(0l, expectedBox.top - 1);
	expectedBox.right--;
	expectedBox.bottom--;
}

void LeagueOCRImageDumper::drawColorAtPosition(LeagueOCRPixelData* pixelData, uint32_t color, uint32_t x, uint32_t y) const {
	pixelData->getPixelMemoryAsSinglechannel()[(pixelData->getHeight() - y - 1) * pixelData->getWidth() + x] = color;
}

void LeagueOCRImageDumper::drawBoundingBox(LeagueOCRPixelData* pixelData, const LetterBoundingBox* box) const {
	const uint32_t PIXEL_SET_COLOR = 0xFFFF00;
	for (LONG x = box->getBoundingBox().left; x < box->getBoundingBox().right; x++) {
		for (LONG y = box->getBoundingBox().top; y < box->getBoundingBox().bottom; y++) {
			drawColorAtPosition(pixelData, PIXEL_SET_COLOR, x, y);
		}
	}
}

HANDLE LeagueOCRImageDumper::createAndPrepareOutputFile(const wchar_t* outputPath, std::shared_ptr<BITMAPINFO> bitmapInfo) const {
	HANDLE hf;                 // file handle  
	BITMAPFILEHEADER hdr;       // bitmap file-header  
	PBITMAPINFOHEADER pbih;     // bitmap info-header  
	DWORD dwTotal;              // total count of bytes  
	DWORD cb;                   // incremental count of bytes  
	BYTE* hp;                   // byte pointer  
	DWORD dwTmp;

	pbih = (PBITMAPINFOHEADER)bitmapInfo.get();

	// Create the .BMP file.  
	hf = CreateFile(outputPath,
		GENERIC_READ | GENERIC_WRITE,
		(DWORD)0,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		(HANDLE)NULL);
	if (hf == INVALID_HANDLE_VALUE)
		throw;
	hdr.bfType = 0x4d42;        // 0x42 = "B" 0x4d = "M"  
	// Compute the size of the entire file.  
	hdr.bfSize = (DWORD)(sizeof(BITMAPFILEHEADER) +
		pbih->biSize + pbih->biClrUsed
		* sizeof(RGBQUAD) + pbih->biSizeImage);
	hdr.bfReserved1 = 0;
	hdr.bfReserved2 = 0;

	// Compute the offset to the array of color indices.  
	hdr.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) +
		pbih->biSize + pbih->biClrUsed
		* sizeof(RGBQUAD);

	// Copy the BITMAPFILEHEADER into the .BMP file.  
	if (!WriteFile(hf, (LPVOID)&hdr, sizeof(BITMAPFILEHEADER),
		(LPDWORD)&dwTmp, NULL))
	{
		throw;
	}

	// Copy the BITMAPINFOHEADER and RGBQUAD array into the file.  
	if (!WriteFile(hf, (LPVOID)pbih, sizeof(BITMAPINFOHEADER)
		+ pbih->biClrUsed * sizeof(RGBQUAD),
		(LPDWORD)&dwTmp, (NULL)))
		throw;

	return hf;
}

std::shared_ptr<BITMAPINFO> LeagueOCRImageDumper::createBitmapInfoStruct(HBITMAP hBmp) const {
	BITMAP bmp;
	std::shared_ptr<BITMAPINFO> pbmi;
	DWORD    cClrBits;

	// Retrieve the bitmap color format, width, and height.  
	if (!GetObject(hBmp, sizeof(BITMAP), (LPSTR)&bmp)) {
		return pbmi;
	}

	// Convert the color format to a count of bits.  
	cClrBits = (WORD)(bmp.bmPlanes * bmp.bmBitsPixel);
	if (cClrBits == 1)
		cClrBits = 1;
	else if (cClrBits <= 4)
		cClrBits = 4;
	else if (cClrBits <= 8)
		cClrBits = 8;
	else if (cClrBits <= 16)
		cClrBits = 16;
	else if (cClrBits <= 24)
		cClrBits = 24;
	else cClrBits = 32;

	// Allocate memory for the BITMAPINFO structure. (This structure  
	// contains a BITMAPINFOHEADER structure and an array of RGBQUAD  
	// data structures.)  

	size_t pbmiSize = 0;
	if (cClrBits < 24) {
		pbmiSize = sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * (1 << cClrBits);
	}
	else {
		pbmiSize = sizeof(BITMAPINFOHEADER);
	}
	pbmi = std::shared_ptr<BITMAPINFO>(new BITMAPINFO[pbmiSize], std::default_delete<BITMAPINFO[]>());
	memset(pbmi.get(), 0x00, pbmiSize);

	// Initialize the fields in the BITMAPINFO structure.  

	pbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	pbmi->bmiHeader.biWidth = bmp.bmWidth;
	pbmi->bmiHeader.biHeight = bmp.bmHeight;
	pbmi->bmiHeader.biPlanes = bmp.bmPlanes;
	pbmi->bmiHeader.biBitCount = bmp.bmBitsPixel;
	if (cClrBits < 24)
		pbmi->bmiHeader.biClrUsed = (1 << cClrBits);

	// If the bitmap is not compressed, set the BI_RGB flag.  
	pbmi->bmiHeader.biCompression = BI_RGB;

	// Compute the number of bytes in the array of color  
	// indices and store the result in biSizeImage.  
	// The width must be DWORD aligned unless the bitmap is RLE 
	// compressed. 
	pbmi->bmiHeader.biSizeImage = ((pbmi->bmiHeader.biWidth * cClrBits + 31) & ~31) / 8
		* pbmi->bmiHeader.biHeight;
	// Set biClrImportant to 0, indicating that all of the  
	// device colors are important.  
	pbmi->bmiHeader.biClrImportant = 0;
	return pbmi;
}