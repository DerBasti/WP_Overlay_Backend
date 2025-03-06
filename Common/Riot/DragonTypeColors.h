#ifndef __LEAGUE_DRAGON_TYPE_COLORS__
#define __LEAGUE_DRAGON_TYPE_COLORS__

#include "DragonTypes.h"
#include "../GUI/GUIColor.h"

class DragonTypeColor {
private:
	DragonTypeColor() {}
public:
	virtual ~DragonTypeColor() {}
	static GUIColor FromTypeToColor(DragonType type) {
		GUIColor gradientColor(0, 0, 0);
		switch (type) {
		case DragonType::CHEMTECH:
			gradientColor = GUIColor(64, 92, 48);
			break;
		case DragonType::CLOUD:
			gradientColor = GUIColor::GRAY;
			break;
		case DragonType::HEXTECH:
			gradientColor = GUIColor(24, 134, 214);
			break;
		case DragonType::INFERNAL:
			gradientColor = GUIColor(190, 56, 41);
			break;
		case DragonType::MOUNTAIN:
			gradientColor = GUIColor(106, 70, 52);
			break;
		case DragonType::OCEAN:
			gradientColor = GUIColor(64,192,176);
			break;
		case DragonType::ELDER:
			gradientColor = GUIColor(0, 192, 224, 0);
			break;
		}
		return gradientColor;
	}
};

#endif