#include "stdafx.h"

#include <iostream>
#include <fstream>
#include <errno.h>

#include "AppConfig.h"

#include "rapidjson/filereadstream.h"
#include "rapidjson/encodedstream.h"
#include "rapidjson/error/en.h"

#include "easylogging++.h"

const tstring AppConfig::_CONF_FILE_NAME = L"ConfigAndLog.cfg";

const tstring AppConfig::_STR_LOG = L"Log";
const tstring AppConfig::_STR_FILENAME = L"FileName";
const tstring AppConfig::_STR_FORMAT = L"Format";
const tstring AppConfig::_STR_FLUSH_THRESHOLD = L"FlushThreshold";
const tstring AppConfig::_STR_MAX_LOG_SIZE = L"MaxLogSize";

using namespace std;

AppConfig::AppConfig(const tstring& path) {
	if (!path.empty()) {
		_configFilePath.Combine(path.c_str(), _CONF_FILE_NAME.c_str());
	}
}

AppConfig::~AppConfig() {
}

bool AppConfig::ReadConfig(const tstring& path) {
	WDocument doc;
	WValue val;
	WValue::ConstMemberIterator it;
	wstringstream buff;
	tstring configStr;
	wifstream ifs;

	if (!path.empty()) {
		_configFilePath.Combine(path.c_str(), _CONF_FILE_NAME.c_str());
	}

	if (!_configFilePath.FileExists()) {
		LOG(ERROR) << L"Not exist configuration file.(Path : " << (LPCWSTR)_configFilePath.m_strPath << L")";
		return false;
	}

	ifs.open((LPCWSTR)_configFilePath.m_strPath, ios_base::in);
	if (!ifs.good()) {
		LOG(ERROR) << L"File open failed.(Path: " << (LPCWSTR)_configFilePath.m_strPath << L")";
		return false;
	}

	buff << ifs.rdbuf();
	configStr = buff.str();
	doc.Parse(configStr.c_str());
	ifs.close();

	if (doc.HasParseError()) {
		LOG(ERROR) << L"Parsing error.(Msg: " << rapidjson::GetParseError_En(doc.GetParseError()) 
			<< ", Position: " << doc.GetErrorOffset()
			<< ", Path: " << (LPCWSTR)_configFilePath.m_strPath << L")";
		return false;
	}

	if (!doc.IsObject()) {
		LOG(ERROR) << L"Invalid configuration file.(Path: " << (LPCWSTR)_configFilePath.m_strPath << L")";
		return false;
	}

	it = doc.FindMember(_STR_LOG.c_str());
	if (it != doc.MemberEnd()) {
		const WValue& logConf = it->value;
		ParseLog(logConf);
	}

	return true;
}

void AppConfig::ParseLog(const WValue& logConf) {
	WValue::ConstMemberIterator itr;
	tstring value;
	string tempStr;

	if (!logConf.IsObject()) {
		LOG(WARNING) << L"Invalid configuration(Log)";
		return;
	}

	el::Loggers::reconfigureAllLoggers(el::ConfigurationType::ToStandardOutput, "false");

	value = GetMemberValueString(logConf, _STR_FORMAT);
	if (!value.empty()) {
		tempStr.assign(value.begin(), value.end());
		el::Loggers::reconfigureAllLoggers(el::ConfigurationType::Format, tempStr);
	}

	value = GetMemberValueString(logConf, _STR_FILENAME);
	if (!value.empty()) {
		TCHAR pathStr[_MAX_PATH] = { 0 };
		CPath logPath;
		DWORD buffLen = ::GetEnvironmentVariable(L"TEMP", pathStr, _MAX_PATH);

		if ((buffLen <= 0) || (buffLen >= _MAX_PATH)) {
			LOG(ERROR) << L"Not enough memory.";
			return;
		}

		logPath.Combine(pathStr, value.c_str());
		tempStr = CT2CA(logPath.m_strPath);

		el::Loggers::reconfigureAllLoggers(el::ConfigurationType::Filename, tempStr);
	}

	value = GetMemberValueString(logConf, _STR_FLUSH_THRESHOLD);
	if (!value.empty()) {
		tempStr.assign(value.begin(), value.end());
		el::Loggers::reconfigureAllLoggers(el::ConfigurationType::LogFlushThreshold, tempStr);
	}

	value = GetMemberValueString(logConf, _STR_MAX_LOG_SIZE);
	if (!value.empty()) {
		tempStr.assign(value.begin(), value.end());
		el::Loggers::reconfigureAllLoggers(el::ConfigurationType::MaxLogFileSize, tempStr);
	}

	LOG(INFO) << L"Done configuration for log.";

	return;
}

tstring AppConfig::GetMemberValueString(const WValue& obj, const tstring& name) {
	WValue::ConstMemberIterator itr = obj.FindMember(name.c_str());
	tstring value;

	if (itr == obj.MemberEnd()) {
		return value;
	}

	value = itr->value.GetString();

	return value;
}
