#pragma once

#include <atlpath.h>

#include "Common.h"

#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"

typedef rapidjson::GenericDocument<rapidjson::UTF16<> > WDocument;
typedef rapidjson::GenericValue<rapidjson::UTF16<> > WValue;

class AppConfig {
public:
	AppConfig(const tstring& path = L"");
	~AppConfig();

	bool ReadConfig(const tstring& path = L"");

private:
	void ParseLog(const WValue& logConf);
	tstring GetMemberValueString(const WValue& obj, const tstring& name);

private:
	CPath _configFilePath;

	static const tstring _CONF_FILE_NAME;

	static const tstring _STR_LOG;
	static const tstring _STR_FILENAME;
	static const tstring _STR_FORMAT;
	static const tstring _STR_FLUSH_THRESHOLD;
	static const tstring _STR_MAX_LOG_SIZE;
};

