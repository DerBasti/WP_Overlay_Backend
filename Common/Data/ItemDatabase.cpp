 #include "ItemDatabase.h"
#include "json/document.h"
#include "CURLWrapper.h"
#include "VersionDatabase.h"
#include <iostream>

using namespace rapidjson;

const std::wstring ItemDatabase::DEFAULT_FILE_PATH = ProjectFilePathHandler::GetDefaultFilePathUnicode() + std::wstring(L"Icons/");

ItemDatabase* ItemDatabase::instance = nullptr;
const char* itemUrl = "http://ddragon.leagueoflegends.com/cdn/%s/data/en_US/item.json";

Item::Item(uint32_t id, std::string name) {
	itemId = id;
	wchar_t buffer[0x200] = { 0x00 };
	swprintf_s(buffer, L"%s/%i.png", ItemDatabase::GetItemIconDefaultFilePath().c_str(), itemId);
	this->imageFilePath = std::wstring(buffer);
	this->image = nullptr;
	this->name = name;
	legendaryFlag = 0;
	this->mythicFlag = false;
	buyGoldAmount = 0;
	this->legendaryFlag = false;
}

Item::~Item() {
	unloadImage();
}

ItemDatabase::ItemDatabase() {
	itemImagesLoadedFlag = false;
	readAndAddItems();
}

ItemDatabase::~ItemDatabase() {
	unloadItemImages();
	for (auto it : itemList) {
		delete it.second;
		it.second = nullptr;
	}
}

void ItemDatabase::readAndAddItems() {
	char url[0x200] = { 0x00 };
	sprintf_s(url, itemUrl, VersionDatabase::getInstance()->getLatestVersion().c_str());
	CURLBufferedWrapper wrapper(url);
	wrapper.fireRequest();
	Document jsonFile; jsonFile.Parse(wrapper.getReadData().c_str());
	Value& dataIt = jsonFile["data"];
	for(auto itemIt = dataIt.MemberBegin();itemIt != dataIt.MemberEnd();itemIt++) {
		auto& x = *itemIt;
		Item* item = new Item(atoi(x.name.GetString()), x.value["name"].GetString());
		if (x.value.FindMember("depth") != x.value.MemberEnd()) {
			auto itemDepth = x.value["depth"].GetInt();
			item->setIsLegendary(itemDepth >= 3);
		}
		if (x.value.FindMember("into") != x.value.MemberEnd()) {
			Value& itemDataBuildsIntoIterator = x.value["into"];
			for(auto itemInto = itemDataBuildsIntoIterator.Begin();itemInto != itemDataBuildsIntoIterator.End();itemInto++) {
				uint32_t itemBuildsIntoId = atoi(itemInto->GetString());
				item->addItemIdItBuildsInto(itemBuildsIntoId);
				if (itemBuildsIntoId >= 7000 && itemBuildsIntoId <= 7049) {
					item->setIsMythic(true);
				}
			}
		}
		if (x.value.FindMember("gold") != x.value.MemberEnd() && x.value["gold"].GetObj().FindMember("total") != x.value["gold"].GetObj().MemberEnd()) {
			item->setBuyGoldAmount(x.value["gold"].GetObj()["total"].GetInt());
		}
		logger.logDebug("Adding item ", *item, " to database.");
		this->onItemAdd(item->getId(), item);
		this->itemList.insert(std::make_pair(item->getId(), item));
	}
}

void ItemDatabase::onItemAdd(uint32_t id, Item* item) {

}

void ItemDatabase::printAllItems() {
	for (auto it : itemList) {
		logger.logInfo(*(it.second));
	}
}

#ifndef __LEAGUE_DIRECT2D_ENABLE__
void ItemDatabase::loadItemImages() {
	unloadItemImages();
	for (auto it : itemList) {
		it.second->loadImage();
	}
	itemImagesLoadedFlag = true;
}
#else
void ItemDatabase::loadItemImages(Microsoft::WRL::ComPtr<ID2D1RenderTarget> renderer) {
	unloadItemImages();
	auto dxInstance = Direct2DFactory::GetInstance();
	for (auto it : itemList) {
		auto image = dxInstance->createBitmap((ProjectFilePathHandler::GetDefaultFilePathUnicode() + it.second->getImageFilePath()).c_str(), renderer);
		it.second->setImage(image);
	}
	itemImagesLoadedFlag = true;
}
void ItemDatabase::loadItemImages(std::function<std::shared_ptr<Direct2DBitmap>(std::wstring)> loadingCallback) {
	for (auto it : itemList) {
		auto image = loadingCallback(ProjectFilePathHandler::GetDefaultFilePathUnicode() + it.second->getImageFilePath());
		it.second->setImage(image);
	}
}
#endif

void ItemDatabase::unloadItemImages() {
	for (auto it : itemList) {
		it.second->unloadImage();
	}
	itemImagesLoadedFlag = false;
}

std::ostream& operator<<(std::ostream& out, const Item& item) {
	out << "[ID: " << item.getId() << ", Name: " << item.getName().c_str() << ", IsLegendary: " << std::boolalpha << item.isLegendary() << ", IsMythic: " << item.isMythic() << "]" << std::noboolalpha;
	return out;
}

std::wostream& operator<<(std::wostream& out, const Item& item) {
	out << "[ID: " << item.getId() << ", Name: " << item.getName().c_str() << ", IsLegendary: " << std::boolalpha << item.isLegendary() << ", IsMythic: " << item.isMythic() << "]" << std::noboolalpha;
	return out;
}