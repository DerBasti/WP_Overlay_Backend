#ifndef __LEAGUE_ITEM_DATABASE__
#define __LEAGUE_ITEM_DATABASE__

#include <inttypes.h>
#include <cstring>
#include <unordered_map>
#include <unordered_set>
#include "../Logging/Logger.h"
#include "../../Common/GUI/Drawable.h"
#include "../ProjectFilePathHandler.h"

class DXGIRenderer;

class Item {
private:
	uint32_t itemId;
	std::string name;
	std::wstring imageFilePath;
	bool mythicFlag;
	bool legendaryFlag;
	uint32_t buyGoldAmount;
#ifndef __LEAGUE_DIRECT2D_ENABLE__
	Gdiplus::Image* image;
#else
	std::shared_ptr<Direct2DBitmap> image;
#endif
	std::unordered_set<uint32_t> buildsIntoItemIds;
public:
	Item(uint32_t id, std::string name);
	virtual ~Item();
#ifndef __LEAGUE_DIRECT2D_ENABLE__
	__inline void loadImage() {
		this->image = new Bitmap((ProjectFilePathHandler::GetDefaultFilePathUnicode() + imageName).c_str());
	}
	__inline void unloadImage() {
		if (this->image) {
			delete this->image;
			this->image = nullptr;
		}
	}
	__inline Gdiplus::Image* getImage() const {
		return image;
	}
#else
	__inline void setImage(std::shared_ptr<Direct2DBitmap> bitmap) {
		this->image = bitmap;
	}
	__inline void unloadImage() {
		this->image = std::shared_ptr<Direct2DBitmap>();
	}
	__inline std::shared_ptr<Direct2DBitmap> getImage() const {
		return image;
	}
#endif
	__inline constexpr uint32_t getId() const {
		return itemId;
	}
	__inline const std::string& getName() const {
		return name;
	}
	__inline bool isLegendary() const {
		return legendaryFlag && !mythicFlag;
	}
	__inline void setIsLegendary(bool legendaryFlag) {
		this->legendaryFlag = legendaryFlag;
	}
	__inline bool isMythic() const {
		return mythicFlag;
	}
	__inline uint32_t getBuyGoldAmount() const {
		return buyGoldAmount;
	}
	__inline void setBuyGoldAmount(uint32_t amount) {
		this->buyGoldAmount = amount;
	}
	__inline std::wstring getImageFilePath() const {
		return imageFilePath;
	}
	__inline void setIsMythic(bool mythicFlag) {
		this->mythicFlag = mythicFlag;
	}
	__inline void addItemIdItBuildsInto(uint32_t id) {
		this->buildsIntoItemIds.insert(id);
	}
	__inline const std::unordered_set<uint32_t>& getItemIdsItBuildsInto() const {
		return buildsIntoItemIds;
	}
};

std::ostream& operator<<(std::ostream& out, const Item& item);
std::wostream& operator<<(std::wostream& out, const Item& item);

class ItemDatabase {
private:
	std::unordered_map<uint32_t, Item*> itemList;
	static ItemDatabase* instance;
	void readAndAddItems();
	ROSELogger logger;
	bool itemImagesLoadedFlag;
	static const std::wstring DEFAULT_FILE_PATH;
protected:
	ItemDatabase();
	virtual void onItemAdd(uint32_t id, Item* item);
public:
	virtual ~ItemDatabase();

	__inline static ItemDatabase* getInstance() {
		if (instance == nullptr) {
			instance = new ItemDatabase();
		}
		return instance;
	}

	static std::wstring GetItemIconDefaultFilePath() {
		return DEFAULT_FILE_PATH;
	}

	static void ReleaseInstance() {
		if (instance) {
			delete instance;
			instance = nullptr;
		}
	}

	void printAllItems();
#ifndef __LEAGUE_DIRECT2D_ENABLE__
	void loadItemImages();
#else
	void loadItemImages(Microsoft::WRL::ComPtr<ID2D1RenderTarget> renderer);
	void loadItemImages(std::function<std::shared_ptr<Direct2DBitmap>(std::wstring)> loadingCallback);
#endif
	void unloadItemImages();
	
	__inline Item* getItem(uint32_t id) const {
		auto it = itemList.find(id);
		return (it == itemList.cend() ? nullptr : it->second);
	}
	const std::unordered_map<uint32_t, Item*>& getItemList() const {
		return itemList;
	}
	__inline bool isItemImagesLoaded() const {
		return itemImagesLoadedFlag;
	}
};

#endif //__LEAGUE_ITEM_DATABASE__