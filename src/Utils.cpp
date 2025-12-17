#include <iostream>
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
