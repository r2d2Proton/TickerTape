#include <thread>
#include <iostream>
#include <ctime>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <tuple>
#include <algorithm>
#include <sstream>

#include <curl/curl.h>
#include <nlohmann/json.hpp>

#include "Stock.h"

using namespace std;
using namespace std::chrono;
using json = nlohmann::json;


// ---------------------------------------------------------------------------------------
// Download URL:
// https://www.alphavantage.co/query?function=TIME_SERIES_INTRADAY&symbol=AMZN&interval=1min&outputsize=full&datatype=csv&apikey=demo
//
// Filename: /AlphaVantage/Downloads/2018-09-07/intraday_1min_AMZN.csv
//
// Format: timestamp, open, high, low, close, volume
//
// 2018-09-07 15:59:00,1953.0500,1953.2800,1952.2200,1953.1000,63837
// 2018-09-07 15:58:00,1953.1700,1953.7800,1952.9009,1952.9100,33356
// :
// 2018-09-07 09:31:00,1944.4000,1949.3550,1944.4000,1949.3550,52008
// 2018-09-07 09:30:00,1937.9301,1944.8199,1937.8101,1944.0699,215066
// :
// convert dataset to map<timestamp, vector<symbol, open, volume>>
// ---------------------------------------------------------------------------------------
//static string prefix = "https://www.alphavantage.co/query?function=TIME_SERIES_INTRADAY&symbol=";
//static string suffix = "&interval=1min&outputsize=full&datatype=csv&apikey=TP0YZA44F9L6E10L";
// 
// datatype=csv
// datatype=json
// 
// LISTING_STATUS
// 
// GLOBAL_QUOTE
// TIME_SERIES_DAILY
// TIME_SERIES_INTRADAY
// 
static string ListingStatusPrefix = "https://www.alphavantage.co/query?function=LISTING_STATUS";
static string ListingStatusSuffix = "&apikey=TP0YZA44F9L6E10L";

static string GlobalQuotePrefix = "https://www.alphavantage.co/query?function=GLOBAL_QUOTE&symbol=";
static string GlobalQuoteSuffix = "&interval=1min&outputsize=full&datatype=json&apikey=TP0YZA44F9L6E10L";

static string TimeSeriesDailyPrefix = "https://www.alphavantage.co/query?function=TIME_SERIES_DAILY&symbol=";
static string TimeSeriesDailySuffix = "&interval=1min&datatype=json&apikey=TP0YZA44F9L6E10L";

static string TimeSeriesIntradayPrefix = "https://www.alphavantage.co/query?function=TIME_SERIES_INTRADAY&symbol=";
static string TimeSeriesIntradaySuffix = "&interval=1min&datatype=json&apikey=TP0YZA44F9L6E10L";


static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp)
{
	size_t totalSize = size * nmemb;
	std::string* str = static_cast<std::string*>(userp);
	str->append(static_cast<char*>(contents), totalSize);
	return totalSize;
}


static CURLcode downloadURL(CURL* curl, string url)
{
	curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	CURLcode res = curl_easy_perform(curl);
	return res;
}


static void downloadListingStatus(CURL* curl, string& downloadBuffer)
{
	// LISTING_STATUS
	string url = ListingStatusPrefix + ListingStatusSuffix;
	downloadBuffer.clear();
	cout << url << endl;

	CURLcode res = downloadURL(curl, url);
	cout << url << ": " << endl << downloadBuffer << endl;

	this_thread::sleep_until(chrono::system_clock::now() + chrono::seconds(10));
}


static void downloadGlobalQuote(CURL* curl, string& downloadBuffer, const string& symbol)
{
	// GLOBAL_QUOTE
	string url = GlobalQuotePrefix + symbol + GlobalQuoteSuffix;
	downloadBuffer.clear();
	cout << url << endl;

	CURLcode res = downloadURL(curl, url);

	if (res == CURLE_OK)
		cout << symbol << ": " << endl << downloadBuffer << endl;
	else
		cerr << symbol << "error" << endl;

	this_thread::sleep_until(chrono::system_clock::now() + chrono::seconds(4));
}


static void downloadTimeSeriesDaily(CURL* curl, string& downloadBuffer, const string& symbol)
{
	// TIME_SERIES_DAILY
	string url = TimeSeriesDailyPrefix + symbol + TimeSeriesDailySuffix;
	downloadBuffer.clear();
	cout << url << endl;

	CURLcode res = downloadURL(curl, url);

	if (res == CURLE_OK)
		cout << symbol << ": " << endl << downloadBuffer << endl;
	else
		cerr << symbol << "error" << endl;

	const string TimeSeriesDaily("Time Series (Daily)");
	json j = json::parse(downloadBuffer);

	if (j.contains(TimeSeriesDaily))
	{
		auto& timeSeries = j[TimeSeriesDaily];
		auto lastEntry = timeSeries.begin();
		string latestDate = lastEntry.key();
		auto& dailyData = lastEntry.value();

		// Output stock data
		std::cout << "Latest date: " << latestDate << "\n";
		std::cout << "Open:  " << dailyData["1. open"] << "\n";
		std::cout << "High:  " << dailyData["2. high"] << "\n";
		std::cout << "Low:   " << dailyData["3. low"] << "\n";
		std::cout << "Close: " << dailyData["4. close"] << "\n";
	}

	this_thread::sleep_until(chrono::system_clock::now() + chrono::seconds(4));
}


static void downloadTimeSeriesIntraday(CURL* curl, string& downloadBuffer, const string& symbol)
{
	// TIME_SERIES_INTRADAY
	string url = TimeSeriesIntradayPrefix + symbol + TimeSeriesIntradaySuffix;
	downloadBuffer.clear();
	cout << url << endl;

	CURLcode res = downloadURL(curl, url);

	if (res == CURLE_OK)
		cout << symbol << ": " << endl << downloadBuffer << endl;
	else
		cerr << symbol << "error" << endl;

	cout << endl;

	this_thread::sleep_until(chrono::system_clock::now() + chrono::seconds(4));
}


static bool downloadStocks(Stock& stocks, map<string, string>& symbols)
{
	CURL* curl = curl_easy_init();
	if (!curl)
		return false;

	CURLcode res;
	string url;
	string downloadBuffer;
	
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &downloadBuffer);

	// LISTING_STATUS
	downloadListingStatus(curl, downloadBuffer);
	clearScreen();

	for (auto& symbol : symbols)
	{
		// GLOBAL_QUOTE
		downloadGlobalQuote(curl, downloadBuffer, symbol.first);

		// TIME_SERIES_DAILY
		downloadTimeSeriesDaily(curl, downloadBuffer, symbol.first);

		// TIME_SERIES_INTRADAY
		downloadTimeSeriesIntraday(curl, downloadBuffer, symbol.first);

		//clearScreen();
	}

	curl_easy_cleanup(curl);

	return true;
}


static void processStocks(Stock& stocks)
{
	for (auto& stock : stocks)
	{
		string symbol = stock.first;
	}
}


static bool writeSymbolsDownloadURLs(map<string, string>& symbols, const string& filename)
{
	ofstream outFile(filename);

	// LISTING_STATUS
	outFile << ListingStatusPrefix + ListingStatusSuffix << endl;

	for (auto& symbol : symbols)
	{
		// GLOBAL_QUOTE
		outFile << GlobalQuotePrefix << symbol.first << GlobalQuoteSuffix << endl;

		// TIME_SERIES_DAILY
		outFile << TimeSeriesDailyPrefix << symbol.first << TimeSeriesDailySuffix << endl;

		// TIME_SERIES_INTRADAY
		outFile << TimeSeriesIntradayPrefix << symbol.first << TimeSeriesIntradaySuffix << endl;
	}

	outFile.close();

	return true;
}


static bool writeStocksDownloadURLs(Stock& stocks, const string& filename)
{
	ofstream outFile(filename);

	// LISTING_STATUS
	outFile << ListingStatusPrefix + ListingStatusSuffix << endl;

	for (auto& symbol : stocks)
	{
		// GLOBAL_QUOTE
		outFile << GlobalQuotePrefix << symbol.first << GlobalQuoteSuffix << endl;

		// TIME_SERIES_DAILY
		outFile << TimeSeriesDailyPrefix << symbol.first << TimeSeriesDailySuffix << endl;

		// TIME_SERIES_INTRADAY
		outFile << TimeSeriesIntradayPrefix << symbol.first << TimeSeriesIntradaySuffix << endl;
	}

	outFile.close();

	return true;
}


static bool parseCSVStocks(Stock& stocks, const string& filename, const string& path, const string& date)
{
	string txtLine;

	ifstream inFile(filename);
	if (!inFile.is_open())
		return false;

	while (!inFile.eof())
	{
		string symbol;
		inFile >> symbol;

		if (!inFile.fail())
		{
			StockItr p = stocks.find(symbol);

			// check if already in our map
			if (p == stocks.end())
			{
				string dsFilename = path + "intraday_1min_" + symbol + ".csv";
				ifstream srcDataset(dsFilename);
				if (srcDataset.is_open())
				{
					// read past the header line
					getline(srcDataset, txtLine);

					while (!srcDataset.eof())
					{
						getline(srcDataset, txtLine);

						if (!srcDataset.fail())
						{
							//timestamp,open,high,low,close,volume
							//2018-09-07 15:59:00,38.4300,38.4350,38.4000,38.4300,63289
							string timestamp;
							double open;
							double high;
							double low;
							double close;
							int volume;

							stringstream ss(txtLine);
							
							if (!getline(ss, timestamp, ','))
								continue;

							if (timestamp.compare(0, date.size(), date) != 0)
								continue;

							ss >> open;
							ss.ignore(1);

							ss >> high;
							ss.ignore(1);

							ss >> low;
							ss.ignore(1);

							ss >> close;
							ss.ignore(1);

							ss >> volume;

							stocks[timestamp].push_back(make_tuple(symbol, open, volume));
						}
					}

					srcDataset.close();
				}
			}
		}
	}

	inFile.close();

	return true;
}


static void writeCombinedData(Stock& stocks, const string& filename)
{
	ofstream outCombinedFile(filename);

	// use first map entry vector size as the number of data points to write
	for (StockItr stockItr = stocks.begin(); stockItr != stocks.end(); ++stockItr)
	{
		for (TradeVectorItr tradeItr = stockItr->second.begin(); tradeItr != stockItr->second.end(); ++tradeItr)
		{
			Trade& trade = *tradeItr;
			outCombinedFile << stockItr->first << " " << get<0>(trade) << " " << get<1>(trade) << " " << get<2>(trade);
			
			// write  CR/LF if more data is available (either another timestamp or another trade for the current day
			if (next(stockItr, 1) != stocks.end() || next(tradeItr, 1) != stockItr->second.end())
				outCombinedFile << endl;
		}
	}
}


static bool addSymbols(map<string, string>& symbols, const string& symbolsFilename)
{
	ifstream inFile(symbolsFilename);
	if (!inFile.is_open())
		return false;

	while (!inFile.eof())
	{
		string txtLine;
		getline(inFile, txtLine);

		if (!inFile.fail())
		{
			string symbol;
			stringstream ss(txtLine);

			if (!getline(ss, symbol, ','))
				continue;

			symbol = trim(symbol);

			map<string, string>::iterator itr = symbols.find(symbol);

			// add to symbols map if needed
			if (itr == symbols.end())
			{
				symbols[symbol] = symbol;
			}
		}
	}

	inFile.close();

	return true;
}


bool downloadDataset
(
	Stock& stocks,
	map<string, string>& symbols,
	const string& path,
	const string& symbolsFilename,
	const string& symbolsURLsFilename,
	const string& stocksURLsFilename,
	const string& combinedStocksFilename,
	const string& parseStocksFilename
)
{
	const string date = "2018-09-07";
	const string fullpathSymbolsFilename = getFullpath(path, symbolsFilename);
	const string fullpathSymbolsURLsFilename = getFullpath(path, symbolsURLsFilename);
	const string fullpathstocksURLsFilename = getFullpath(path, stocksURLsFilename);
	const string fullpathCombinedStocksFilename = getFullpath(path, combinedStocksFilename);
	const string fullpathParseStocksFilename = getFullpath(path, parseStocksFilename);

	cout << "Adding symbols from " << fullpathSymbolsFilename << endl;
	addSymbols(symbols, fullpathSymbolsFilename);

	downloadStocks(stocks, symbols);

	cout << "Writing Symbols URL's " << fullpathSymbolsURLsFilename  << endl;
	writeSymbolsDownloadURLs(symbols, fullpathSymbolsURLsFilename);

	cout << "Writing Stocks downloaded URL's " << fullpathstocksURLsFilename << endl;
	writeStocksDownloadURLs(stocks, fullpathstocksURLsFilename);

	cout << "Parsing combined stocks from " << fullpathParseStocksFilename << endl;
	if (!parseCSVStocks(stocks, fullpathParseStocksFilename, path, date))
	{
		cout << "Error reading " << fullpathParseStocksFilename << endl;
		cout << "Press any key to continue. . .";
		cin.get();
		return false;
	}
	
	cout << "Writing combined data to " << fullpathCombinedStocksFilename << endl;
	writeCombinedData(stocks, fullpathCombinedStocksFilename);

	return true;
}
