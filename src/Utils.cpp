#include <iostream>
#include <sstream>
#include <ctime>
#include <algorithm>
#include <string>

#include <shlobj.h>
#pragma comment(lib, "shell32.lib")

#include "Stock.h"

using namespace std;
using namespace std::chrono;


void clearScreen()
{
	system("cls");
}


string trim(const string& s)
{
	auto start = std::find_if_not(s.begin(), s.end(), ::isspace);
	auto end = std::find_if_not(s.rbegin(), s.rend(), ::isspace).base();
	return (start < end) ? std::string(start, end) : "";
}


string getDownloadFolder()
{
	PWSTR path = nullptr;
	HRESULT hr = SHGetKnownFolderPath(FOLDERID_Downloads, 0, NULL, &path);

	if (SUCCEEDED(hr))
	{
		wstring wpath(path);
		int size_needed = WideCharToMultiByte(CP_UTF8, 0, wpath.c_str(), -1, NULL, 0, NULL, NULL);
		string utf8path(size_needed - 1, 0);
		WideCharToMultiByte(CP_UTF8, 0, wpath.c_str(), -1, utf8path.data(), size_needed, NULL, NULL);
		cout << "Download folder: " << utf8path << endl;
		return utf8path;
	}
	else {
		cerr << "Failed to get Downloads folder path. HRESULT: " << hr << endl;
	}

	return string();
}


const string getFullpath(const string& path, const string& filename)
{
	filesystem::path base(path);
	filesystem::path file(filename);

	if (file.has_parent_path())
	{
		return file.string();
	}
	else
	{
		return (base / file).string();
	}
}


time_t parseDateToEpoch(const std::string& mmddyyyy)
{
	std::tm tm = {};
	std::istringstream ss(mmddyyyy);
	ss >> std::get_time(&tm, "%m/%d/%Y");

	if (ss.fail()) throw std::runtime_error("Invalid date format, expected MM/DD/YYYY");
	
	// tm is in local time; convert to time_t (seconds since epoch)
	return mktime(&tm);
}


string epoch_to_utc_string(long epoch)
{
	std::time_t t = static_cast<std::time_t>(epoch);
	std::tm result{};
	gmtime_s(&result, &t);

	char buf[32];
	strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &result);

	return string(buf);
}


bool timePointToLocalTm(const TimePoint& tp, std::tm& outLocalTm)
{
	std::time_t tt = system_clock::to_time_t(tp);
	if (localtime_s(&outLocalTm, &tt) != 0) return false;
	return true;
}


std::pair<long long, long long> computeLocalDayEpochRange(const TimePoint& tp)
{
	std::tm localTm{};
	if (!timePointToLocalTm(tp, localTm)) return { 0, 0 };
	
	// Normalize to local midnight (start of day)
	std::tm startTm = localTm;
	startTm.tm_hour = 0;
	startTm.tm_min = 0;
	startTm.tm_sec = 0;
	startTm.tm_isdst = -1; // let C Runtime (CRT) determine Daylight Savings Time (DST)
	
	// mktime interprets tm as local time and returns time_t (epoch seconds)
	std::time_t startTimeT = std::mktime(&startTm);
	if (startTimeT == (std::time_t)-1) return {0, 0};
	
	// Compute next day start by adding one day (24*3600 seconds)
	// Safer alternative: increment tm_mday and call mktime again to handle month/year boundaries
	std::tm nextTm = startTm;
	nextTm.tm_mday += 1;
	nextTm.tm_isdst = -1;
	std::time_t nextTimeT = std::mktime(&nextTm);
	
	if (nextTimeT == (std::time_t)-1)
	{
		// fallback: add 24h in seconds
		nextTimeT = startTimeT + 24 * 3600;
	}
	
	return { static_cast<long long>(startTimeT), static_cast<long long>(nextTimeT) };
}









