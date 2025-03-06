#ifndef __RIOT_API__
#define __RIOT_API__

#include <map>
#include "RiotLockFile.h"
#include "../Logging/Logger.h"
#include "../Data/json/document.h"
#include "../Data/json/stringbuffer.h"
#include "../Data/json/prettywriter.h"

class CURLWrapper;

class RiotAPI {
private:
	CURLWrapper* dataRequester;
	char* templateRequest;
	bool dumpCurrentRequestFlag;
protected:
	ROSEThreadedLogger logger;
	RiotAPI(uint16_t port);

	rapidjson::Document getRequestToApi(const char* uriToAdd) {
		return getRequestToApi(uriToAdd, true);
	}
	rapidjson::Document getRequestToApi(const char* uriToAdd, bool logError);
	rapidjson::Document postRequestToApi(const char* uriToAdd, const char* jsonAsString);
	void dumpDataIntoLog(const wchar_t* logFile);
	void addArgumentsToDataRequests(const std::vector<std::string>& args);
	std::string escapeUrl(const char* urlToEscape) const;
public:
	virtual ~RiotAPI();

	inline const char* getTemplateRequestUrl() const {
		return templateRequest;
	}

	void replaceDataRequester(CURLWrapper* wrapperToReplace);

	inline void dumpNextResponseBody() {
		this->dumpCurrentRequestFlag = true;
	}
	const char* getLastRequestError() const;

	template<class _T>
	static void LogJsonObject(_T* jsonObject) {
#ifdef _DEBUG
		std::cout << TransformJsonToString(jsonObject).c_str() << "\n";
#endif
	}

	template<class _T>
	static std::string TransformJsonToString(_T* jsonObject) {
		rapidjson::StringBuffer sb;
		rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(sb);
		jsonObject->Accept(writer);
		std::string result = std::string(sb.GetString(), sb.GetLength());
		return result;
	}
};

#endif //__RIOT_API__