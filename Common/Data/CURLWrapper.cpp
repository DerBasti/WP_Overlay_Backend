	#include "CURLWrapper.h"
#include "../ProjectFilePathHandler.h"
#include "../PerformanceClock.h"
#include <iostream>

//#pragma comment(lib, "D:\\Programmieren\\CPP\\Broadcaster\\Common\\lib\\curl\\x64\\libcurl.dll.a")
#pragma comment(lib, __FILE__"/../../lib/curl/x64/libcurl.dll.a")

std::shared_ptr<CURLWrapper> CURLWrapper::instance = nullptr;

static size_t
WriteMemoryCallback(void *contents, size_t position, size_t nmemb, void *userp)
{
	size_t realsize = position * nmemb;
	CURLWrapperDataHolder *ptr = (CURLWrapperDataHolder*)userp;
	if (!ptr) {
		return 0;
	}
	ptr->content.append((char*)contents, realsize);
	ptr->length += realsize;
	if (ptr->writeCallback) {
		ptr->writeCallback((char*)contents, realsize);
	}
	return realsize;
}

static
void dump(const char *text,
	FILE *stream, unsigned char *ptr, size_t size,
	char nohex)
{
	size_t i;
	size_t c;

	unsigned int width = 0x10;

	if (nohex)
		/* without the hex output, we can fit more on screen */
		width = 0x40;

	fprintf(stream, "%s, %10.10lu bytes (0x%8.8lx)\n",
		text, (unsigned long)size, (unsigned long)size);

	for (i = 0; i < size; i += width) {

		fprintf(stream, "%4.4lx: ", (unsigned long)i);

		if (!nohex) {
			/* hex not disabled, show it */
			for (c = 0; c < width; c++)
				if (i + c < size)
					fprintf(stream, "%02x ", ptr[i + c]);
				else
					fputs("   ", stream);
		}

		for (c = 0; (c < width) && (i + c < size); c++) {
			/* check for 0D0A; if found, skip past and start a new line of output */
			if (nohex && (i + c + 1 < size) && ptr[i + c] == 0x0D &&
				ptr[i + c + 1] == 0x0A) {
				i += (c + 2 - width);
				break;
			}
			fprintf(stream, "%c",
				(ptr[i + c] >= 0x20) && (ptr[i + c] < 0x80) ? ptr[i + c] : '.');
			/* check again for 0D0A, to avoid an extra \n if it's at width */
			if (nohex && (i + c + 2 < size) && ptr[i + c + 1] == 0x0D &&
				ptr[i + c + 2] == 0x0A) {
				i += (c + 3 - width);
				break;
			}
		}
		fputc('\n', stream); /* newline */
	}
	fflush(stream);
}

static
int my_trace(CURL *handle, curl_infotype type,
	char *data, size_t size,
	void *userp)
{
	const char *text;
	(void)handle; /* prevent compiler warning */

	switch (type) {
	case CURLINFO_TEXT:
		fprintf(stderr, "== Info: %s", data);
		/* FALLTHROUGH */
	default: /* in case a new one is introduced to shock us */
		return 0;

	case CURLINFO_HEADER_OUT:
		text = "=> Send header";
		break;
	case CURLINFO_DATA_OUT:
		text = "=> Send data";
		break;
	case CURLINFO_SSL_DATA_OUT:
		text = "=> Send SSL data";
		break;
	case CURLINFO_HEADER_IN:
		text = "<= Recv header";
		break;
	case CURLINFO_DATA_IN:
		text = "<= Recv data";
		break;
	case CURLINFO_SSL_DATA_IN:
		text = "<= Recv SSL data";
		break;
	}

	dump(text, stderr, (unsigned char *)data, size, 1);
	return 0;
}

CURLWrapper::CURLWrapper(const char* url, std::vector<std::string> arguments) {
	if (!instance) {
		instance = std::shared_ptr<CURLWrapper>(new CURLWrapper());
	}
	debugOutput = false;
	globalInit = false;
	saveHeaderFlag = false;
	tooManyRequestsRetryFlag = false;
	lastResponseCode = CURLE_OK;
	httpResponseCode = 200;
	avgDownloadSpeed = 0.0f;
	dataLength = 0;
	lastModificationTimestamp = 0;
	currentRequestMethod = CURLWrapperHttpMethod::GET;
	suppressCallErrors = false;
	retryAmount = 0;

	curl = curl_easy_init();
	uint32_t bufferSize = 128 * 1024;
	char* buffer = new char[bufferSize];
	memset(buffer, 0x00, bufferSize);
	responseDataHolder.content.reserve(bufferSize);
	responseDataHolder.writeCallback = nullptr;
	responseDataHolder.length = 0x00;
	char* headerBuffer = new char[bufferSize];
	memset(headerBuffer, 0x00, bufferSize);
	responseHeaderHolder.content.reserve(bufferSize);
	responseHeaderHolder.length = 0x00;
	responseHeaderHolder.writeCallback = nullptr;

	headers = nullptr;

	if (curl) {
		setUrl(url);
		//curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, my_trace);
		auto bufResult = curl_easy_setopt(curl, CURLOPT_BUFFERSIZE, bufferSize - 1);
		std::string certificatePath = (ProjectFilePathHandler::GetDefaultFilePath() + std::string("cacert.pem"));
		pemFilePath = std::shared_ptr<char>(new char[certificatePath.length() + 1], std::default_delete<char[]>());
		strncpy_s(pemFilePath.get(), certificatePath.length() + 1, certificatePath.c_str(), certificatePath.length());
		curl_easy_setopt(curl, CURLOPT_HEADERDATA, &responseHeaderHolder);
		curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, WriteMemoryCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseDataHolder);
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
		curl_easy_setopt(curl, CURLOPT_FAILONERROR, 0L);

		setSaveHeaderResponse(false);
		setReuseConnectionEnabled(true);
		setSslEnabled(false);

		addRequestArguments(arguments);
		addRequestArguments({ "User-Agent: libcurl" });
	}
}

bool CURLWrapper::fireRequest() {
	lastHeaderResponse.clear();
	responseDataHolder.content.clear();
	responseHeaderHolder.content.clear();
	responseHeaderHolder.length = 0x00;
	responseDataHolder.length = 0x00;
	onRequestStart();
	PerformanceClock timer;
	lastResponseCode = curl_easy_perform(curl);
	tokenizeHeaderResponse();
	this->avgDownloadSpeed = ((float)responseDataHolder.length / float(timer.getDuration() / 1000.0f)) / 1024.0f;
	bool success = lastResponseCode == CURLE_OK;
	curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpResponseCode);
	success &= (httpResponseCode / 100) < 4 || (lastModificationTimestamp != 0 && httpResponseCode == 200);
	if (!success) {
		lastResponseCode = CURLE_HTTP_RETURNED_ERROR;
		if (!suppressCallErrors) {
			logger.logWarn("UrlCall to url ", this->currentUrl.c_str(), " wasn't successful. HttpResponseCode: ", httpResponseCode);
		}
		responseDataHolder.length = 0;
	}
	this->dataLength = responseDataHolder.length;
	onRequestFinished(success);
	return success;
}

void CURLWrapper::tokenizeHeaderResponse() {
	if (responseHeaderHolder.content.find("HTTP") == 0) {
		responseHeaderHolder.content = responseHeaderHolder.content.substr(responseHeaderHolder.content.find('\n')+1);
	}
	while (responseHeaderHolder.content.length() > 5) {
		std::string currentLine = responseHeaderHolder.content.substr(0, responseHeaderHolder.content.find('\n')+1);
		responseHeaderHolder.content = responseHeaderHolder.content.substr(currentLine.length());
		std::string key = currentLine.substr(0, currentLine.find(':'));
		std::string value = currentLine.substr(currentLine.find(':') + 2);
		if (value.back() == '\n') {
			value = value.substr(0, value.length() - 2);
		}
		lastHeaderResponse.insert_or_assign(key, value);
	}
}

bool CURLWrapper::isLastRequestErrorPage() {
	return lastResponseCode != CURLE_OK;
}

void CURLWrapper::setHttpMethod(CURLWrapperHttpMethod method) {
	switch (method) {
		case CURLWrapperHttpMethod::GET:
			curl_easy_setopt(curl, CURLOPT_HTTPGET, 1);
			setSaveHeaderResponse(false);
		break;
		case CURLWrapperHttpMethod::HEAD:
			curl_easy_setopt(curl, CURLOPT_HTTPGET, 1);
			curl_easy_setopt(curl, CURLOPT_NOBODY, 1);
			setSaveHeaderResponse(true);
		break;
		case CURLWrapperHttpMethod::POST:
			curl_easy_setopt(curl, CURLOPT_HTTPPOST, 1);
			setSaveHeaderResponse(false);
		break;
	}
	currentRequestMethod = method;
}

std::string CURLWrapper::getETagFromLastRequest() const {
	std::string eTag = "";
	if (currentRequestMethod == CURLWrapperHttpMethod::HEAD) {
		std::string header = getReadData();
		size_t nextLinePosition = std::string::npos;
		while (!header.empty() && (nextLinePosition = header.find('\r')) != std::string::npos) {
			std::string keyValue = header.substr(0, nextLinePosition);
			size_t splitToken = keyValue.find(":");
			if (splitToken != std::string::npos) {
				std::string key = keyValue.substr(0, splitToken);
				std::string value = keyValue.substr(splitToken+2);
				if (_stricmp(key.c_str(), "etag") == 0) {
					eTag = keyValue.substr(keyValue.find("\"") + 1);
					eTag.pop_back();
					break;
				}
			}
			header = header.substr(nextLinePosition+2);
		}
	}
	return eTag;
}

std::string CURLWrapper::escapeUrl(const char* urlToEscape) const {
	char* escapedUrl = curl_easy_escape(curl, urlToEscape, 0);
	std::string escapedUrlStr = std::string(escapedUrl);
	curl_free(escapedUrl);
	return escapedUrl;
}

void CURLWrapper::addRequestArguments(const std::vector<std::string>& requestArgs) {
	if (!requestArgs.empty()) {
		for (auto it : requestArgs) {
			requestArguments.push_back(it);
			headers = curl_slist_append(headers, it.c_str());
			curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
		}
	}
}

void CURLWrapper::setSslEnabled(bool enabledFlag) {
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, (long)enabledFlag);
	if (enabledFlag) {
		auto curlCode = curl_easy_setopt(curl, CURLOPT_CAINFO, pemFilePath.get());
		curlCode = curl_easy_setopt(curl, CURLOPT_SSL_VERIFYSTATUS, 0L);
		curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_2TLS);
		curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_TRY);
		curl_easy_setopt(curl, CURLOPT_SSLENGINE_DEFAULT, 1L);
	}
}

void CURLWrapper::setFollowRedirectEnabled(bool enabledFlag) {
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, (long)enabledFlag);
}

void CURLWrapper::setLastModificationCheckTime(uint64_t timestamp) {
	this->lastModificationTimestamp = timestamp;
	time_t time = (time_t)(timestamp / 1000);
	std::tm localTime{ 0 };
	localtime_s(&localTime, &time);
	char buffer[0x100] = { 0x00 };
	strftime(buffer, 0x100, "If-Modified-Since: %a, %d %b %Y %H:%M:%S GMT", &localTime);
	addRequestArguments({ buffer });
}

CURLBufferedWrapper::CURLBufferedWrapper(const char* url) : CURLWrapper(url) {

}

CURLBufferedWrapper::~CURLBufferedWrapper() {

}

void CURLBufferedWrapper::onRequestStart() {
}

void CURLBufferedWrapper::onRequestFinished(bool requestSuccessfulFlag) {

}

CURLFileOutputWrapper::CURLFileOutputWrapper(const char* url, const wchar_t* filePath) : CURLWrapper(url) {
	fileHandle = nullptr;
	this->filePath = std::wstring(filePath);
	this->fileSize = 0;

	std::function<void(const char*, size_t)> writeCallback = [&](const char* data, size_t readBytes) {
		if (fileHandle) {
			fwrite(data, sizeof(char), readBytes, fileHandle);
		}
	};
	this->setWriteCallback(writeCallback);
}

CURLFileOutputWrapper::~CURLFileOutputWrapper() {
	if (fileHandle) {
		fclose(fileHandle);
	}
}

#pragma warning(disable:4996)
void CURLFileOutputWrapper::onRequestStart() {
	fileHandle = _wfopen(filePath.c_str(), L"wb+");
}

void CURLFileOutputWrapper::onRequestFinished(bool requestSuccessfulFlag) {
	fileSize = _ftelli64(fileHandle);
	fclose(fileHandle);
	fileHandle = nullptr;
}


CURLMultiThreadedWrapper::CURLMultiThreadedWrapper() {
	this->multiHandle = curl_multi_init();
}

CURLMultiThreadedWrapper::~CURLMultiThreadedWrapper() {
	for (auto child : children) {
		curl_multi_remove_handle(multiHandle, child->getCurlHandle());
		delete child;
	}
	curl_multi_cleanup(multiHandle);
	multiHandle = nullptr;
}

CURLBufferedWrapper* CURLMultiThreadedWrapper::createChildWrapper(const char* url) {
	CURLBufferedWrapper* childWrapper = new CURLBufferedWrapper(url);

	auto returnCode = curl_multi_add_handle(multiHandle, childWrapper->getCurlHandle());

	children.push_back(childWrapper);
	return childWrapper;
}

void CURLMultiThreadedWrapper::runDownloads() {
	int32_t stillRunningFlag = 0;
	curl_multi_perform(multiHandle, &stillRunningFlag);
}

bool CURLMultiThreadedWrapper::isDownloadsFinished() {
	int32_t stillRunningFlag = 0;
	curl_multi_perform(multiHandle, &stillRunningFlag);
	CURLMcode mc = curl_multi_poll(multiHandle, NULL, 0, 1000, NULL);
	return !stillRunningFlag && !mc;
}