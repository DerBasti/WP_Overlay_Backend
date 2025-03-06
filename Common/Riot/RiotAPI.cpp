#include "RiotAPI.h"
#include "../Data/CURLWrapper.h"
#include "Base64Authentication.h"
#include "../ProjectFilePathHandler.h"
#include "../Utility.h"

RiotAPI::RiotAPI(uint16_t port) {
	templateRequest = new char[0x300];
	sprintf_s(templateRequest, 0x300, "https://127.0.0.1:%i/%s", port, "%s");
	dumpCurrentRequestFlag = false;
	dataRequester = new CURLBufferedWrapper(templateRequest);
	dataRequester->setReuseConnectionEnabled(true);
	dataRequester->setConnectionTimeoutInMilliseconds(200);
}

RiotAPI::~RiotAPI() {
	delete dataRequester;
	dataRequester = nullptr;

	delete[] templateRequest;
	templateRequest = nullptr;
}

void RiotAPI::replaceDataRequester(CURLWrapper* wrapperToReplace) {
	wrapperToReplace->addRequestArguments(dataRequester->getRequestArguments());
	delete dataRequester;
	this->dataRequester = wrapperToReplace;
	dataRequester->setReuseConnectionEnabled(true);
	dataRequester->setConnectionTimeoutInMilliseconds(200);
}

#include "../PerformanceClock.h"
#include <shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")

rapidjson::Document RiotAPI::getRequestToApi(const char* uriToAdd, bool logError) {
	char url[0x400] = { 0x00 };
	sprintf_s(url, templateRequest, uriToAdd);
	PerformanceClock clock;
	dataRequester->setUrl(url);
	dataRequester->setHttpMethod(CURLWrapperHttpMethod::GET);
	dataRequester->setSuppressCallErrors(!logError);
	dataRequester->fireRequest();
	clock.reset();
	rapidjson::Document doc;
	if (dataRequester->getReadDataLength() > 0) {
		doc.Parse(dataRequester->getReadData().c_str());
		if (doc.IsObject() && doc.HasMember("errorCode") && doc.HasMember("httpStatus")) {
			doc.Parse(""); //ErrorObject from Riot, nothing to parse
		}
#ifdef _DEBUG
		if (dumpCurrentRequestFlag) {
			dumpCurrentRequestFlag = false;
			dumpDataIntoLog((ProjectFilePathHandler::GetDefaultFilePathUnicode() + std::wstring(L"api_response.json")).c_str());
		}
#endif
	}
	else {
		doc.Parse("");
	}
	return doc;
}

rapidjson::Document RiotAPI::postRequestToApi(const char* uriToAdd, const char* jsonAsString) {
	char url[0x400] = { 0x00 };
	sprintf_s(url, templateRequest, uriToAdd);
	dataRequester->setUrl(url);
	dataRequester->setHttpMethod(CURLWrapperHttpMethod::POST);
	dataRequester->setPostData(jsonAsString);
	dataRequester->fireRequest();
	rapidjson::Document doc;
	if (dataRequester->getReadDataLength() > 0) {
		doc.Parse(dataRequester->getReadData().c_str());
		if (doc.IsObject() && doc.HasMember("errorCode") && doc.HasMember("httpStatus")) {
			doc.Parse(""); //ErrorObject from Riot, nothing to parse
		}
#ifdef _DEBUG
		if (dumpCurrentRequestFlag) {
			dumpCurrentRequestFlag = false;
			dumpDataIntoLog((ProjectFilePathHandler::GetDefaultFilePathUnicode() + std::wstring(L"api_response.json")).c_str());
		}
#endif
	}
	else {
		doc.Parse("");
	}
	return doc;
}

void RiotAPI::dumpDataIntoLog(const wchar_t* logFilePath) {
	SynchronizedLogFile logFile(logFilePath);
	logFile.write<char>(dataRequester->getReadData().c_str());
}

void RiotAPI::addArgumentsToDataRequests(const std::vector<std::string>& args) {
	dataRequester->addRequestArguments(args);
}

std::string RiotAPI::escapeUrl(const char* urlToEscape) const {
	return dataRequester->escapeUrl(urlToEscape);
}

const char* RiotAPI::getLastRequestError() const {
	return dataRequester->getLastResponseErrorMessage();
}