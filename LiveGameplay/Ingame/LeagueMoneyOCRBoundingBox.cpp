#include "LeagueMoneyOCRBoundingBox.h"
#include "LeagueOCRPixelData.h"
#include <queue>
#include <stack>
#include <set>
#include <functional>

LetterBoundingBox::LetterBoundingBox(const LeagueOCRPixelData* pixelData, LetterBoundingBoxColor dominantColor) {
	this->pixel = pixelData->getPixelMemoryAsSinglechannel();
	this->width = pixelData->getWidth();
	this->height = pixelData->getHeight();
	this->dominantColor = dominantColor;
	detectionMode = LeagueOCRLocaleDetectionMode::OCR_DETECTION_WESTERN_RAYFILL;
	boundingBox = { 0 };
}

LetterBoundingBox::~LetterBoundingBox() {
	pixel = nullptr;
	width = height = 0;
}

LetterBoundingBoxScanner::LetterBoundingBoxScanner(const LeagueOCRPixelData* pixelData, LetterBoundingBoxColor dominantColor) {
	this->pixelData = pixelData;
	this->dominantColor = dominantColor;
}

LetterBoundingBoxScanner::~LetterBoundingBoxScanner() {

}

std::vector<LetterBoundingBox> LetterBoundingBoxScanner::detectBoundingBoxes(LeagueOCRLocaleDetectionMode detectionMode) {
	uint32_t xStart = 0;
	std::vector<LetterBoundingBox> detectedBoxes;
	while (true) {
		LetterBoundingBox boundingBox = startRayCasting(detectionMode, xStart);
		if (!boundingBox.isValid()) {
			break;
		}
		xStart = boundingBox.getBoundingBox().right + 1;
		detectedBoxes.push_back(boundingBox);
	}
	return detectedBoxes;
}

std::pair<uint32_t, uint32_t> LetterBoundingBoxScanner::findFirstValidPixelInsideBoundingBox(const LetterBoundingBox* box) const {
	LetterBoundingBox copyBox = *box;
	if (castCenterRay(&copyBox, box->getBoundingBox().left)) {
		return std::make_pair(copyBox.getBoundingBox().left, copyBox.getBoundingBox().top);
	}
	return std::make_pair(-1, -1);
}

LetterBoundingBox LetterBoundingBoxScanner::startRayCasting(LeagueOCRLocaleDetectionMode detectionMode, uint32_t xStart) {
	LetterBoundingBox boundingBox(pixelData, dominantColor);
	switch (detectionMode) {
		case LeagueOCRLocaleDetectionMode::OCR_DETECTION_WESTERN_RAYFILL:
			rayFill(&boundingBox, xStart);
		break;
		case LeagueOCRLocaleDetectionMode::OCR_DETECTION_KOREAN:
			if (castCenterRay(&boundingBox, xStart)) {
				quadFill(&boundingBox);
			}
		break;
		default:
			if (castCenterRay(&boundingBox, xStart)) {
				spanFill(&boundingBox);
			}
	}
	return boundingBox;
}

void LetterBoundingBoxScanner::rayFill(LetterBoundingBox* letterBox, uint32_t xStart) {
	uint32_t pixelValue = 0x00;
	findBoundingBoxLower(letterBox, xStart);
	findBoundingBoxUpper(letterBox, xStart);
	findBoundingBoxCenter(letterBox, xStart);
	letterBox->boundingBox.left--;
	letterBox->boundingBox.top--;
	letterBox->boundingBox.right++;
	letterBox->boundingBox.bottom++;
}

void LetterBoundingBoxScanner::spanFill(LetterBoundingBox* letterBox) {
	std::stack<RECT> spanStack;
	spanStack.push(RECT{ letterBox->boundingBox.left, letterBox->boundingBox.top, letterBox->boundingBox.left, 1 });
	spanStack.push(RECT{ letterBox->boundingBox.left, letterBox->boundingBox.top-1, letterBox->boundingBox.left, -1 });

	std::set<uint32_t> pixelFilled;

	std::function<bool(LONG, LONG)> isInside = [&](LONG x, LONG y) {
		bool pixelSet = pixelFilled.find((x << 16) | y) != pixelFilled.cend();
		bool validColor = isValidColorAtPosition(x, y);
		return !pixelSet && validColor;
	};

	RECT expectedBox = RECT{ letterBox->boundingBox.left, letterBox->boundingBox.top, 0, 0 };
	while (!spanStack.empty()) {
		auto currentPosition = spanStack.top();
		spanStack.pop();
		LONG x = currentPosition.left; 
		if (isInside(x, currentPosition.top)) {
			while (isInside(x - 1, currentPosition.top)) {
				x--;
				pixelFilled.insert(x << 16 | currentPosition.top);
			}
			if (x < currentPosition.left) {
				spanStack.push(RECT{ x, currentPosition.top - currentPosition.bottom, currentPosition.left - 1, -currentPosition.bottom });
			}
		}
		if (currentPosition.left < expectedBox.left) {
			expectedBox.left = currentPosition.left;
		}
		if (x < expectedBox.left) {
			expectedBox.left = x;
		}
		if (currentPosition.top < expectedBox.top) {
			expectedBox.top = currentPosition.top;
		}
		if (currentPosition.left > expectedBox.right) {
			expectedBox.right = currentPosition.left;
		}
		if (x > expectedBox.right) {
			expectedBox.right = currentPosition.left;
		}
		if (currentPosition.top > expectedBox.bottom) {
			expectedBox.bottom = currentPosition.top;
		}
		while (currentPosition.left <= currentPosition.right) {
			while(isInside(currentPosition.left, currentPosition.top)) {
				pixelFilled.insert(currentPosition.left << 16 | currentPosition.top);
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
	expectedBox.left = (std::max)(0l, expectedBox.left-1);
	expectedBox.top = (std::max)(0l, expectedBox.top-1);
	expectedBox.right--;
	expectedBox.bottom--;
	letterBox->boundingBox = expectedBox;
}

void LetterBoundingBoxScanner::quadFill(LetterBoundingBox* letterBox) {
	std::queue<std::pair<LONG, LONG>> q;
	std::set<LONG> pixelFilled;

	std::function<LONG(LONG, LONG)> CoordinateConverter = [](LONG x, LONG y) {
		return (x << 16) | y;
	};
	std::function<bool(LONG, LONG)> isInside = [&](LONG x, LONG y) {
		if (x < 0 || y < 0) {
			return false;
		}
		bool pixelSet = pixelFilled.find((x << 16) | y) != pixelFilled.cend();
		bool validColor = isValidColorAtPosition(x, y);
		return !pixelSet && validColor;
	};

	q.push(std::make_pair(letterBox->boundingBox.left, letterBox->boundingBox.top));
	RECT expectedBox = RECT{ letterBox->boundingBox.left, letterBox->boundingBox.top, 0, 0 };
	while (!q.empty()) {
		auto n = q.front();
		q.pop();
		if (isInside(n.first, n.second)) {
			pixelFilled.insert(CoordinateConverter(n.first, n.second));
			q.push(std::make_pair(n.first - 1, n.second));
			q.push(std::make_pair(n.first - 1, n.second-1));
			q.push(std::make_pair(n.first, n.second-1));
			q.push(std::make_pair(n.first + 1, n.second-1));
			q.push(std::make_pair(n.first + 1, n.second));
			q.push(std::make_pair(n.first + 1, n.second+1));
			q.push(std::make_pair(n.first, n.second + 1));
			q.push(std::make_pair(n.first-1, n.second + 1));

			if (n.first < expectedBox.left) {
				expectedBox.left = n.first;
			}
			if (n.second < expectedBox.top) {
				expectedBox.top = n.second;
			}
			if (n.first > expectedBox.right) {
				expectedBox.right = n.first;
			}
			if (n.second > expectedBox.bottom) {
				expectedBox.bottom = n.second;
			}
		}
	}
	expectedBox.left = (std::max)(0l, expectedBox.left - 1);
	expectedBox.top = (std::max)(0l, expectedBox.top - 1);
	expectedBox.right++;
	expectedBox.bottom++;
	letterBox->boundingBox = expectedBox;
}

bool LetterBoundingBoxScanner::castCenterRay(LetterBoundingBox* letterBox, uint32_t xStart) const {
	for (LONG i = (xStart); i < (LONG)pixelData->getWidth(); i++) {
		auto pixelColor = extractPixelColorAndValueFromCoordinate(i, pixelData->getHeight() / 2);
		if (isValidColor(pixelColor.first.asRGBA(), pixelColor.second)) {
			if (letterBox) {
				letterBox->boundingBox.left = i;
				letterBox->boundingBox.top = pixelData->getHeight() / 2;
				letterBox->boundingBox.right = i;
				letterBox->boundingBox.bottom = letterBox->boundingBox.top + 1;
			}
			return true;
		}
	}
	return false;
}


std::pair<GUIColor, uint32_t> LetterBoundingBoxScanner::extractPixelColorAndValueFromCoordinate(uint32_t x, uint32_t y) const {
	return ExtractPixelColorAndValueFromCoordinate(pixelData, x, y);
}

std::pair<GUIColor, uint32_t> LetterBoundingBoxScanner::ExtractPixelColorAndValueFromCoordinate(const LeagueOCRPixelData* pixelData, uint32_t x, uint32_t y) {
	uint32_t translatedCoordinate = TranslateBitmapToActualYCoordinate(y, pixelData->getHeight());
	uint32_t pixelColor = GetPixelColor(pixelData->getPixelMemoryAsSinglechannel(), x, translatedCoordinate, pixelData->getWidth());
	uint32_t pixelValue = ExtractPixelValue(pixelColor);
	return std::make_pair(GUIColor(pixelColor), pixelValue);
}

bool LetterBoundingBoxScanner::isValidColorAtPosition(uint32_t x, uint32_t y) {
	auto color = extractPixelColorAndValueFromCoordinate(x, y);
	return isValidColor(color.first.asRGBA(), color.second);
}

bool LetterBoundingBoxScanner::IsValidColorAtPosition(const LeagueOCRPixelData* pixel, LetterBoundingBoxColor dominantColorToCheckFor, uint32_t x, uint32_t y) {
	auto color = ExtractPixelColorAndValueFromCoordinate(pixel, x, y);
	return IsValidColor(color.first.asRGBA(), color.second, dominantColorToCheckFor);
}

bool LetterBoundingBoxScanner::isValidColor(uint32_t color, uint32_t colorValue) const {
	return IsValidColor(color, colorValue, dominantColor);
}

bool LetterBoundingBoxScanner::IsValidColor(uint32_t color, uint32_t colorValue, LetterBoundingBoxColor dominantColorToCheckFor) {
	if (colorValue < 0xA0) {
		return false;
	}
	uint8_t separatedColor[3] = { (color & 0xFF), ((color >> 8) & 0xFF), ((color >> 16) & 0xFF) };
	switch (dominantColorToCheckFor) {
	case LetterBoundingBoxColor::BLUE:
		return separatedColor[0] > (separatedColor[1] + separatedColor[2]) && separatedColor[0] > 0x90 && (separatedColor[1] < (separatedColor[0] / 2) || separatedColor[2] < (separatedColor[0] / 2));
		break;
	case LetterBoundingBoxColor::GREEN:
		return separatedColor[1] > (separatedColor[2] + separatedColor[0]) && separatedColor[1] > 0x90 && (separatedColor[2] < (separatedColor[1] / 2) || separatedColor[0] < (separatedColor[1] / 2));
		break;
	case LetterBoundingBoxColor::RED:
		return separatedColor[2] > (separatedColor[1] + separatedColor[0]) && separatedColor[2] > 0x90 && (separatedColor[1] < (separatedColor[2] / 2) || separatedColor[0] < (separatedColor[2] / 2));
		break;
	}
	return false;
}

void LetterBoundingBoxScanner::castRightRayFromLower(LetterBoundingBox* letterBox) {
	for (LONG i = letterBox->boundingBox.left + 1; i < (LONG)pixelData->getWidth(); i++) {
		auto pixelValues = extractPixelColorAndValueFromCoordinate(i, letterBox->boundingBox.bottom);
		if (!isValidColor(pixelValues.first.asRGBA(), pixelValues.second)) {
			break;
		}
		if (i > letterBox->boundingBox.right) {
			letterBox->boundingBox.right = i;
		}
	}
}
void LetterBoundingBoxScanner::castRightRayFromUpper(LetterBoundingBox* letterBox) {
	for (LONG i = letterBox->boundingBox.left + 1; i < (LONG)pixelData->getWidth(); i++) {
		uint32_t translatedCoordinate = TranslateBitmapToActualYCoordinate(letterBox->boundingBox.top, pixelData->getHeight());
		uint32_t pixelColor = getPixelColor(i, translatedCoordinate);
		uint32_t pixelValue = ExtractPixelValue(pixelColor);
		if (!isValidColor(pixelColor, pixelValue)) {
			break;
		}
		if (i > letterBox->boundingBox.right) {
			letterBox->boundingBox.right = i;
		}
	}
}
void LetterBoundingBoxScanner::castUpperLeftRayFromLower(LetterBoundingBox* letterBox, uint32_t xStart) {
	uint32_t translatedPixelValue = TranslateBitmapToActualYCoordinate(letterBox->boundingBox.top, pixelData->getHeight());
	for (uint32_t i = translatedPixelValue + 1; i < pixelData->getHeight(); i++) {
		for (int32_t j = (int32_t)(letterBox->boundingBox.left - 1); j > (int32_t)xStart; j--) {
			uint32_t pixelColor = getPixelColor(j, i);
			uint32_t pixelValue = ExtractPixelValue(pixelColor);
			if (!isValidColor(pixelColor, pixelValue)) {
				break;
			}
			letterBox->boundingBox.left = j;
			letterBox->boundingBox.top = TranslateBitmapToActualYCoordinate(i, pixelData->getHeight());
		}
	}
}
void LetterBoundingBoxScanner::castUpperRightRayFromLower(LetterBoundingBox* letterBox) {
	int32_t translatedPixelValue = TranslateBitmapToActualYCoordinate(letterBox->boundingBox.top, pixelData->getHeight());
	for (uint32_t i = translatedPixelValue + 1; i < pixelData->getHeight(); i++) {
		for (uint32_t j = letterBox->boundingBox.right + 1; j < pixelData->getWidth(); j++) {
			uint32_t pixelColor = getPixelColor(j, i);
			uint32_t pixelValue = ExtractPixelValue(pixelColor);
			if (!isValidColor(pixelColor, pixelValue) || !isColorSomehowConnectedLeftside(j, i)) {
				break;
			}
			letterBox->boundingBox.right = j;
			letterBox->boundingBox.top = TranslateBitmapToActualYCoordinate(i, pixelData->getHeight());
		}
	}
}
void LetterBoundingBoxScanner::findBoundingBoxLower(LetterBoundingBox* letterBox, uint32_t xStart) {
	bool found = false;
	for (uint32_t i = 0; !found && i < pixelData->getHeight(); i++) {
		for (uint32_t j = xStart; j < pixelData->getWidth(); j++) {
			uint32_t pixelColor = getPixelColor(j, i);
			uint32_t pixelValue = ExtractPixelValue(pixelColor);
			if (isValidColor(pixelColor, pixelValue)) {
				letterBox->boundingBox.bottom = TranslateBitmapToActualYCoordinate(i, pixelData->getHeight());
				letterBox->boundingBox.top = letterBox->boundingBox.bottom;
				letterBox->boundingBox.left = j;
				letterBox->boundingBox.right = letterBox->boundingBox.left;
				found = true;
				break;
			}
		}
	}
	if (found) {
		castUpperLeftRayFromLower(letterBox, xStart);
		castRightRayFromLower(letterBox);
		castUpperRightRayFromLower(letterBox);
	}
	else {
		letterBox->boundingBox = RECT{ 0,0,0,0 };
	}
}

void LetterBoundingBoxScanner::castLowerLeftRayFromUpper(LetterBoundingBox* letterBox, uint32_t xStart) {
	int32_t translatedPixelValue = TranslateBitmapToActualYCoordinate(letterBox->boundingBox.top, pixelData->getHeight());
	for (int32_t i = translatedPixelValue + 1; i > 0; i--) {
		int32_t currentTranslatedHeight = TranslateBitmapToActualYCoordinate(i, pixelData->getHeight());
		for (int32_t j = letterBox->boundingBox.left - 1; j > (int32_t)xStart; j--) {
			uint32_t pixelColor = getPixelColor(j, i);
			uint32_t pixelValue = ExtractPixelValue(pixelColor);
			if (!isValidColor(pixelColor, pixelValue)) {
				return;
			}
			if (j < letterBox->boundingBox.left) {
				letterBox->boundingBox.left = j;
			}
			if (currentTranslatedHeight > letterBox->boundingBox.bottom) {
				letterBox->boundingBox.bottom = TranslateBitmapToActualYCoordinate(i, pixelData->getHeight());
			}
		}
	}
}
bool LetterBoundingBoxScanner::isColorSomehowConnectedLeftside(uint32_t currentX, uint32_t currentY) {
	uint32_t colors[3] = { this->getPixelColor(currentX - 1, (currentY <= 0 ? 0 : currentY - 1)),
		this->getPixelColor(currentX - 1, currentY),
		this->getPixelColor(currentX - 1, currentY + 1)
	};
	return (currentY > 0 && isValidColor(colors[0], ExtractPixelValue(colors[0]))) || isValidColor(colors[1], ExtractPixelValue(colors[1])) ||
		isValidColor(colors[2], ExtractPixelValue(colors[2]));
}

bool LetterBoundingBoxScanner::isColorSomehowConnectedRightside(uint32_t currentX, uint32_t currentY) {
	uint32_t colors[3] = { getPixelColor(currentX + 1, currentY - 1),
		getPixelColor(currentX + 1, currentY),
		getPixelColor(currentX + 1, currentY + 1)
	};
	return (currentY > 0 && isValidColor(colors[0], ExtractPixelValue(colors[0]))) || isValidColor(colors[1], ExtractPixelValue(colors[1])) ||
		isValidColor(colors[2], ExtractPixelValue(colors[2]));
}

void LetterBoundingBoxScanner::findBoundingBoxUpper(LetterBoundingBox* letterBox, uint32_t xStart) {
	int32_t translatedY = TranslateBitmapToActualYCoordinate(letterBox->boundingBox.top, pixelData->getHeight());
	for (int32_t i = pixelData->getHeight() - 1; i >= translatedY; i--) {
		int32_t currentTranslatedHeight = TranslateBitmapToActualYCoordinate(i, pixelData->getHeight());
		for (uint32_t j = xStart; j < (uint32_t)letterBox->boundingBox.right; j++) {
			uint32_t pixelColor = getPixelColor(j, i);
			uint32_t pixelValue = ExtractPixelValue(pixelColor);
			if (isValidColor(pixelColor, pixelValue) && isColorSomehowConnectedRightside(j, i)) {
				letterBox->boundingBox.top = TranslateBitmapToActualYCoordinate(i, pixelData->getHeight());
				if (j < (uint32_t)letterBox->boundingBox.left) {
					letterBox->boundingBox.left = j;
				}
				castRightRayFromUpper(letterBox);
				return;
			}
		}
	}
}
void LetterBoundingBoxScanner::findBoundingBoxCenter(LetterBoundingBox* letterBox, uint32_t xStart) {
	uint32_t centerY = (letterBox->boundingBox.bottom - letterBox->boundingBox.top) / 2;
	uint32_t initialX = letterBox->boundingBox.left;
	for (int32_t i = initialX - 1; i >= (int32_t)(initialX - 4) && i >= 0; i--) {
		uint32_t pixelColor = getPixelColor(i, centerY);
		uint32_t pixelValue = ExtractPixelValue(pixelColor);
		if (!isValidColor(pixelColor, pixelValue)) {
			return;
		}
		letterBox->boundingBox.left = i;
	}
}