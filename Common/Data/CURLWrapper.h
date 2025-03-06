#ifndef __LEAGUE_CURL_READER__
#define __LEAGUE_CURL_READER__

#include "./curl/curl.h"
#include "../Logging/Logger.h"
#include <functional>
#include <memory>
#include <vector>
#include <string>

#pragma comment(lib, "ws2_32.lib")

struct CURLWrapperDataHolder {
	std::string content;
	size_t length;
	std::function<void(const char*, size_t)> writeCallback;
};

enum class CURLWrapperHttpMethod : uint8_t {
	GET,
	POST,
	HEAD
};

class CURLWrapper {
private:
	ROSEThreadedLogger logger;
	std::shared_ptr<char> pemFilePath;
	size_t dataLength;
	CURL* curl;
	bool debugOutput;
	bool suppressCallErrors;
	bool tooManyRequestsRetryFlag;
	bool saveHeaderFlag;
	uint32_t retryAmount;
	CURLWrapperHttpMethod currentRequestMethod;
	CURLWrapperDataHolder responseDataHolder;
	CURLWrapperDataHolder responseHeaderHolder;
	std::unordered_map<std::string, std::string> lastHeaderResponse;
	std::vector<std::string> requestArguments;
	static std::shared_ptr<CURLWrapper> instance;
	bool globalInit;
	CURLcode lastResponseCode;
	uint64_t lastModificationTimestamp;
	int32_t httpResponseCode;
	float avgDownloadSpeed;
	std::string currentUrl;
	curl_slist *headers;
	CURLWrapper() {
		dataLength = 0x00;
		responseDataHolder.content.clear();
		responseDataHolder.length = 0;
		responseHeaderHolder.content.clear();
		responseHeaderHolder.length = 0;
		httpResponseCode = 200;
		suppressCallErrors = false;
		lastResponseCode = CURLE_OK;
		avgDownloadSpeed = 0;
		retryAmount = 0;
		currentRequestMethod = CURLWrapperHttpMethod::GET;
		lastModificationTimestamp = 0;
		curl = nullptr;
		curl_global_init(CURL_GLOBAL_ALL);
		globalInit = true;
		headers = nullptr;
		debugOutput = false;
		tooManyRequestsRetryFlag = false;
	}
	void tokenizeHeaderResponse();
protected:
	void setWriteCallback(std::function<void(const char*, size_t)> callback) {
		responseDataHolder.writeCallback = callback;
	}
	virtual void onRequestStart() {}
	virtual void onRequestFinished(bool requestSuccessfulFlag) {
		responseDataHolder.content.append("\0");
		responseHeaderHolder.content.append("\0");
	}
public:
	CURLWrapper(const char* url) : CURLWrapper(url, std::vector<std::string>()) {}
	CURLWrapper(const char* url, std::vector<std::string> arguments);
	virtual ~CURLWrapper() {
		if (headers) {
			curl_slist_free_all(headers);
			headers = nullptr;
		}
		curl_easy_cleanup(curl);
		if (globalInit) {
			curl_global_cleanup();
		}
	}
	void addRequestArguments(const std::vector<std::string>& requestArgs);
	inline std::vector<std::string> getRequestArguments() const {
		return requestArguments;
	}

	bool fireRequest();
	bool isLastRequestErrorPage();
	void setHttpMethod(CURLWrapperHttpMethod method);

	std::string getETagFromLastRequest() const;
	std::string escapeUrl(const char* urlToEscape) const;

	inline CURL* getCurlHandle() const {
		return curl;
	}

	__inline void setUrl(const char* url) {
		curl_easy_setopt(curl, CURLOPT_URL, url);
		currentUrl = std::string(url);
	}
	inline CURLcode setPostData(const char* postData) {
		CURLcode code = curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postData);
		return code;
	}
	inline void setReuseConnectionEnabled(bool flag) {
		curl_easy_setopt(curl, CURLOPT_FRESH_CONNECT, (long)!flag);
		curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, (long)flag);
	}
	inline bool isSaveHeaderResponse() const {
		return saveHeaderFlag;
	}
	inline void setSaveHeaderResponse(bool flag) {
		saveHeaderFlag = flag;
		curl_easy_setopt(curl, CURLOPT_HEADER, (long)saveHeaderFlag);
	}
	inline const std::string& getLastSavedHeaderResponseAsString() const {
		return responseHeaderHolder.content;
	}
	inline const std::unordered_map<std::string, std::string>& getLastSavedHeaderResponse() const {
		return lastHeaderResponse;
	}
	inline void setConnectionTimeoutInMilliseconds(uint32_t timeoutInMilliseconds) {
		curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, timeoutInMilliseconds);
	}
	inline void setDebugOutput() {
		debugOutput = !debugOutput;
		curl_easy_setopt(curl, CURLOPT_VERBOSE, (uint8_t)debugOutput);
	}
	virtual const std::string& getReadData() const {
		return responseDataHolder.content;
	}
	__inline std::string getUrl() {
		return currentUrl;
	}
	inline float getDownloadSpeed() const {
		return avgDownloadSpeed;
	}

	void setSslEnabled(bool enabledFlag);
	void setFollowRedirectEnabled(bool enabledFlag);
	void setLastModificationCheckTime(uint64_t timestamp);

	inline bool isRetryUponTooManyRequests() const {
		return tooManyRequestsRetryFlag;
	}

	inline void setRetryUponTooManyRequests(bool enabledFlag) {
		tooManyRequestsRetryFlag = enabledFlag;
	}

	inline void setSuppressCallErrors(bool suppress) {
		suppressCallErrors = suppress;
	}
	inline CURLcode getLastResponseCode() const {
		return lastResponseCode;
	}
	inline const char* getLastResponseErrorMessage() const {
		return curl_easy_strerror(getLastResponseCode());
	}
	virtual size_t getReadDataLength() const {
		return 0;
	}
	inline uint32_t getLastHttpResponseCode() const {
		return httpResponseCode;
	}
};

class CURLBufferedWrapper : public CURLWrapper {
protected:
	virtual void onRequestStart();
	virtual void onRequestFinished(bool requestSuccessfulFlag);
public:
	CURLBufferedWrapper(const char* url);
	virtual ~CURLBufferedWrapper();

	virtual size_t getReadDataLength() const {
		return getReadData().length();
	}
};

class CURLFileOutputWrapper : public CURLWrapper {
private:
	FILE* fileHandle;
	std::wstring filePath;
	size_t fileSize;
protected:
	virtual void onRequestStart();
	virtual void onRequestFinished(bool requestSuccessfulFlag);
public:
	CURLFileOutputWrapper(const char* url, const wchar_t* filePath);
	virtual ~CURLFileOutputWrapper();

	inline std::wstring getFileOutputPath() const {
		return filePath;
	}
	virtual size_t getReadDataLength() const {
		return fileSize;
	}
};

class CURLMultiThreadedWrapper {
private:
	CURLM* multiHandle;
	std::vector<CURLBufferedWrapper*> children;
public:
	CURLMultiThreadedWrapper();
	virtual ~CURLMultiThreadedWrapper();

	CURLBufferedWrapper* createChildWrapper(const char* url);
	void runDownloads();
	bool isDownloadsFinished();
};

#endif 