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

enum SaveType { SingleFile, DailyFile, WeeklyFile, MonthlyFile, YearlyFile };

// ------------------------------------------------------------------------------------------------------------------------------------
// Yahoo Finance download URL:
// https://query1.finance.yahoo.com/v8/finance/chart/AMZN?interval=1m&range=7d
// https ://query1.finance.yahoo.com/v8/finance/chart/IBM?interval=1m&range=7d
// https ://query1.finance.yahoo.com/v8/finance/chart/INTC?interval=1m&range=7d
// https ://query1.finance.yahoo.com/v8/finance/chart/MSFT?interval=1m&range=7d
// https ://query1.finance.yahoo.com/v8/finance/chart/NVDA?interval=1m&range=7d
// 
// ------------------------------------------------------------------------------------------------------------------------------------
// Alpha Vantage download URL:
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
// 
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
// ------------------------------------------------------------------------------------------------------------------------------------
static string apiKey("TP0YZA44F9L6E10L");
static string ListingStatusPrefix = "https://www.alphavantage.co/query?function=LISTING_STATUS";
static string ListingStatusSuffix = "&apikey=" + apiKey;

static string GlobalQuotePrefix = "https://www.alphavantage.co/query?function=GLOBAL_QUOTE&symbol=";
static string GlobalQuoteSuffix = "&interval=1min&outputsize=full&datatype=json&apikey=" + apiKey;

static string TimeSeriesDailyPrefix = "https://www.alphavantage.co/query?function=TIME_SERIES_DAILY&symbol=";
static string TimeSeriesDailySuffix = "&interval=1min&datatype=json&apikey=" + apiKey;

static string TimeSeriesIntradayPrefix = "https://www.alphavantage.co/query?function=TIME_SERIES_INTRADAY&symbol=";
static string TimeSeriesIntradaySuffix = "&interval=1min&datatype=json&apikey=" + apiKey;


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


static long fetch_with_backoff(CURL* curl, const string& url, string& downloadBuffer, int maxRetries = 6)
{
	int attempt = 0;
	long http_code = 0;
	
	while (attempt <= maxRetries)
	{
		downloadBuffer.clear();
		CURLcode res = downloadURL(curl, url);

		if (res != CURLE_OK)
		{
			std::cerr << "curl error: " << curl_easy_strerror(res) << endl;
			std::this_thread::sleep_for(std::chrono::seconds(2 << attempt));
			++attempt;
			continue;
		}
		
		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

		if (http_code == 200) return http_code;
		
		if (http_code == 429)
		{
			// exponential backoff
			int wait = (1 << attempt) * 2; // 2,4,8...
			std::cerr << "429 received, backing off " << wait << "s" << endl;
			std::this_thread::sleep_for(std::chrono::seconds(wait));
			++attempt;
			continue;
		}
		
		// other non-200 codes: break and return
		std::cerr << "HTTP code: " << http_code << endl;
		return http_code;
	}
	
	return http_code;
}


static void yahoo_json_to_csv(const string& jsonText, const string& outFilename)
{
	json j = json::parse(jsonText);
	auto& result = j["chart"]["result"][0];
	auto timestamps = result["timestamp"];
	auto quote = result["indicators"]["quote"][0];

	ofstream ofs(outFilename, std::ios::app);
	if (!ofs)
	{
		std::cerr << "Cannot open " << outFilename << endl;
		return;
	}
	
	for (size_t i = 0; i < timestamps.size(); ++i)
	{
		if (timestamps[i].is_null()) continue;

		long ts = timestamps[i].get<long>();
		double open = quote["open"][i].is_null() ? NAN : quote["open"][i].get<double>();
		double high = quote["high"][i].is_null() ? NAN : quote["high"][i].get<double>();
		double low = quote["low"][i].is_null() ? NAN : quote["low"][i].get<double>();
		double close = quote["close"][i].is_null() ? NAN : quote["close"][i].get<double>();
		long volume = quote["volume"][i].is_null() ? 0 : quote["volume"][i].get<long>();
		
		ofs << epoch_to_utc_string(ts) << "," << open << "," << high << "," << low << "," << close << "," << volume << endl;
	}
	
	ofs.close();
}


static TimePoint parse_mmddyyyy(const string& s)
{
	std::tm tm{};
	std::istringstream iss(s);
	iss >> std::get_time(&tm, "%m/%d/%Y");
	tm.tm_isdst = -1;
	std::time_t tt = std::mktime(&tm);
	return std::chrono::system_clock::from_time_t(tt);
}


static vector<std::pair<long long, long long>> computeDailyRanges(const TimePoint& start, const TimePoint& end)
{
	vector<std::pair<long long, long long>> ranges;
	TimePoint cur = start;
	while (cur <= end)
	{
		auto [p1, p2] = computeLocalDayEpochRange(cur);
		ranges.emplace_back(p1, p2);
		cur += std::chrono::hours(24);
	}
	return ranges;
}


static string buildYahooURL
(
	const string& symbol,
	long long p1,
	long long p2,
	const string& interval = "1m"
)
{
	return "https://query1.finance.yahoo.com/v8/finance/chart/"
		+ symbol
		+ "?period1=" + std::to_string(p1)
		+ "&period2=" + std::to_string(p2)
		+ "&interval=" + interval
		+ "&events=history&includeAdjustedClose=true";
}


static std::pair<string, string> makeOutputFilenames
(
	const string& basePath,
	const string& symbol,
	const TimePoint& day,
	SaveType saveType
)
{
	std::tm tm{};
	timePointToLocalTm(day, tm);

	char buf[32];
	std::strftime(buf, sizeof(buf), "%Y-%m-%d", &tm);

	string csvFilename;

	switch (saveType)
	{
	case SingleFile:
		csvFilename = basePath + PathSeparator + symbol + ".csv";
		break;

	case DailyFile:
		csvFilename = basePath + PathSeparator + symbol + "_" + buf + ".csv";
		break;

	case WeeklyFile:
	{
		int week = tm.tm_yday / 7;
		csvFilename = basePath + PathSeparator + symbol + "_week" + std::to_string(week) + ".csv";
		break;
	}

	case MonthlyFile:
		csvFilename = basePath + PathSeparator + symbol + "_" +
			std::to_string(tm.tm_year + 1900) + "-" +
			std::to_string(tm.tm_mon + 1) + ".csv";
		break;

	case YearlyFile:
		csvFilename = basePath + PathSeparator + symbol + "_" +
			std::to_string(tm.tm_year + 1900) + ".csv";
		break;
	}

	string jsonFilename = csvFilename.substr(0, csvFilename.find_last_of('.')) + ".json";
	return { jsonFilename, csvFilename };
}


static void downloadYahoo
(
	CURL* curl,
	string& downloadBuffer,
	Stock& stocks,
	map<string, string>& symbols,
	_TICKER_TAPE_ARGS& args,
	SaveType saveType
)
{
	auto now = system_clock::now();
	auto range = computeLocalDayEpochRange(now);
	long period1 = range.first;
	long period2 = range.second;

	time_t t1 = parseDateToEpoch(args.start);
	time_t t2 = parseDateToEpoch(args.end);

	// add one day to make sure the end date is included
	t2 += 24 * 60 * 60;

	string tmpSymbol("NVDA");

	string tmpURL = "https://query1.finance.yahoo.com/v8/finance/chart/"
		+ tmpSymbol
		+ "?period1=" + to_string(static_cast<long long>(t1))
		+ "&period2=" + to_string(static_cast<long long>(t2))
		+ "&interval=1m&events=history&includeAdjustedClose=true";

	TimePoint start = parse_mmddyyyy(args.start);
	TimePoint end = parse_mmddyyyy(args.end);

	auto ranges = computeDailyRanges(start, end);

	for (auto& sym : symbols)
	{
		const string& symbol = sym.first;

		for (size_t i = 0; i < ranges.size(); ++i)
		{
			auto [p1, p2] = ranges[i];

			string url = buildYahooURL(symbol, p1, p2, "1m");
			cout << url << endl;

			long code = fetch_with_backoff(curl, url, downloadBuffer);
			cout << downloadBuffer << endl;

			if (code != 200 || downloadBuffer.empty())
			{
				std::cerr << "Failed: " << symbol << " day " << i << std::endl;
				continue;
			}

			TimePoint day = start + std::chrono::hours(24 * i);

			auto [jsonFilename, csvFilename] = makeOutputFilenames(args.path, symbol, day, saveType);

			ofstream jsonFile(jsonFilename);
			jsonFile << downloadBuffer;
			jsonFile.close();
			cout << "Saved JSON: " << jsonFilename << endl;

			bool exists = std::filesystem::exists(csvFilename);
			if (!exists)
			{
				std::ofstream hdr(csvFilename);
				hdr << "timestamp,open,high,low,close,volume\n";
			}

			yahoo_json_to_csv(downloadBuffer, csvFilename);
		}
	}
}


static void downloadYahooOld
(
	CURL* curl,
	string& downloadBuffer,
	Stock& stocks,
	map<string, string>& symbols,
	_TICKER_TAPE_ARGS& args
)
{
	for (auto& symbol : symbols)
	{
		auto now = system_clock::now();
		auto range = computeLocalDayEpochRange(now);
		long period1 = range.first;
		long period2 = range.second;

		time_t t1 = parseDateToEpoch(args.start);
		time_t t2 = parseDateToEpoch(args.end);

		// add one day to make sure the end date is included
		t2 += 24 * 60 * 60;

		string url = "https://query1.finance.yahoo.com/v8/finance/chart/"
			       + symbol.first
			       + "?period1=" + to_string(static_cast<long long>(t1)) //std::to_string(period1)
			       + "&period2=" + to_string(static_cast<long long>(t2)) //std::to_string(period2)
			       + "&interval=1m&events=history&includeAdjustedClose=true";

		cout << url << endl;

		long code = fetch_with_backoff(curl, url, downloadBuffer);

		cout << downloadBuffer << endl;

		if (code == 200 && !downloadBuffer.empty())
		{
			// Save JavaScript Object Notation (JSON)
			string jsonFilename(args.path + PathSeparator + symbol.first + "_7d.json");
			ofstream jsonFile(jsonFilename);
			jsonFile << downloadBuffer;
			jsonFile.close();
			cout << "Saved JSON: " << jsonFilename << endl;

			// Convert to Comma Separator Values (CSV)
			string csvFilename(args.path + PathSeparator + symbol.first + "_1m_7d.csv");
			
			// write header if file doesn't exist
			ifstream check(csvFilename);
			bool exists = check.good();
			check.close();
			
			if (!exists)
			{
				ofstream hdr(csvFilename);
				hdr << "timestamp,open,high,low,close,volume\n";
				hdr.close();
			}
			
			yahoo_json_to_csv(downloadBuffer, csvFilename);
			
			cout << "Saved CSV: " << csvFilename << endl;
		}
		else
		{
			std::cerr << "Failed to fetch data, HTTP code: " << code << endl;
		}
	}
}


static void downloadListingStatus
(
	CURL* curl,
	string& downloadBuffer,
	_TICKER_TAPE_ARGS& args
)
{
	// LISTING_STATUS
	string url = ListingStatusPrefix + ListingStatusSuffix;
	downloadBuffer.clear();
	cout << url << endl;

	CURLcode res = downloadURL(curl, url);
	cout << url << ": " << endl << downloadBuffer << endl;

	this_thread::sleep_until(chrono::system_clock::now() + chrono::seconds(10));
}


static void downloadGlobalQuote
(
	CURL* curl,
	string& downloadBuffer,
	const string& symbol,
	_TICKER_TAPE_ARGS& args
)
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


static void downloadTimeSeriesDaily
(
	CURL* curl,
	string& downloadBuffer,
	const string& symbol,
	_TICKER_TAPE_ARGS& args
)
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


static void downloadTimeSeriesIntraday
(
	CURL* curl,
	string& downloadBuffer,
	const string& symbol,
	_TICKER_TAPE_ARGS& args
)
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


static void downloadAlphaVantage
(
	CURL* curl,
	string& downloadBuffer,
	Stock& stocks,
	map<string, string>& symbols,
	_TICKER_TAPE_ARGS& args
)
{
	// LISTING_STATUS
	downloadListingStatus(curl, downloadBuffer, args);

	for (auto& symbol : symbols)
	{
		// GLOBAL_QUOTE
		downloadGlobalQuote(curl, downloadBuffer, symbol.first, args);

		// TIME_SERIES_DAILY
		downloadTimeSeriesDaily(curl, downloadBuffer, symbol.first, args);

		// TIME_SERIES_INTRADAY
		downloadTimeSeriesIntraday(curl, downloadBuffer, symbol.first, args);
	}
}


static bool downloadStocks
(
	Stock& stocks,
	map<string, string>& symbols,
	_TICKER_TAPE_ARGS& args
)
{
	curl_global_init(CURL_GLOBAL_DEFAULT);
	CURL* curl = curl_easy_init();
	if (!curl) return false;

	CURLcode res = CURLE_OK;
	string url;
	string downloadBuffer;
	
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &downloadBuffer);
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0 (compatible)");

	 // Yahoo Finance
	cout << "Downloading Yahoo Finance..." << endl;
	downloadYahoo(curl, downloadBuffer, stocks, symbols, args, SaveType::DailyFile);
	cout << "Yahoo Finance completed." << endl;
	std::this_thread::sleep_for(std::chrono::seconds(9));

	// Alpha Vantage
	cout << "Downloading Alpha Vantage..." << endl;
	downloadAlphaVantage(curl, downloadBuffer, stocks, symbols, args);
	cout << "Alpha Vantage completed." << endl;
	std::this_thread::sleep_for(std::chrono::seconds(9));

	curl_easy_cleanup(curl);
	curl_global_cleanup();

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


static bool parseStocks(Stock& stocks, const string& parseStocksFilename)
{
	ifstream inFile(parseStocksFilename);
	if (inFile.is_open())
	{
		try
		{
			while (!inFile.eof())
			{
				string date;
				string timestamp;
				string symbol;
				double price = 0.0;
				int volume = 0;

				inFile >> date;
				inFile >> timestamp;
				inFile >> symbol;
				inFile >> price;
				inFile >> volume;

				if (inFile.good())
					stocks[date + " " + timestamp].push_back(make_tuple(symbol, price, volume));
			}
		}
		catch (...)
		{
		}

		inFile.close();

		return true;
	}

	return false;
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


static bool cleanFiles(std::initializer_list<string> filenames, int& in, int& out)
{
	namespace fs = std::filesystem;

	auto tryDelete = [&in, &out](const string& filename) -> bool
	{
		in++;
		--out;
		cout << "in=" << in << " out=" << out << " filename=" << filename << endl;

		std::error_code ec;
		if (fs::exists(filename, ec))
		{
			fs::remove(filename, ec);
			return !ec;
		}
			
		return true;
	};

	bool lOK = true;
	for (const auto& filename : filenames)
		lOK &= tryDelete(filename);

	return lOK;
}


bool downloadDataset
(
	Stock& stocks,
	map<string, string>& symbols,
	_TICKER_TAPE_ARGS& args
)
{
	const string date = "2018-09-07";
	const string fullpathSymbolsFilename = getFullpath(args.path, args.symbolsFilename);
	const string fullpathSymbolsURLsFilename = getFullpath(args.path, args.symbolsURLsFilename);
	const string fullpathStocksURLsFilename = getFullpath(args.path, args.stocksURLsFilename);
	const string fullpathCombinedStocksFilename = getFullpath(args.path, args.combinedStocksFilename);
	const string fullpathParseStocksFilename = getFullpath(args.path, args.parseStocksFilename);

	int in = 100;
	int out = -100;

	if (args.bCleanApp)
	{
		cout << "Deleting old files...";
		cleanFiles
		(	{
				fullpathSymbolsURLsFilename,
				fullpathStocksURLsFilename,
				fullpathCombinedStocksFilename,
				fullpathParseStocksFilename
			},
			in,
			out
		);
		cout << "done." << endl;
	}

	cout << "Adding symbols from " << fullpathSymbolsFilename << endl;
	addSymbols(symbols, fullpathSymbolsFilename);

	downloadStocks(stocks, symbols, args);

	cout << "Writing Symbols URL's " << fullpathSymbolsURLsFilename  << endl;
	writeSymbolsDownloadURLs(symbols, fullpathSymbolsURLsFilename);

	cout << "Writing Stocks downloaded URL's " << fullpathStocksURLsFilename << endl;
	writeStocksDownloadURLs(stocks, fullpathStocksURLsFilename);

	cout << "Writing combined data to " << fullpathCombinedStocksFilename << endl;
	writeCombinedData(stocks, fullpathCombinedStocksFilename);

	cout << "Testing parsing algorithm for " << fullpathCombinedStocksFilename << endl;
	parseStocks(stocks, fullpathCombinedStocksFilename);

	cout << "Parsing combined stocks from " << fullpathParseStocksFilename << endl;
	if (!parseCSVStocks(stocks, fullpathParseStocksFilename, args.path, date))
	{
		cout << "Error reading " << fullpathParseStocksFilename << endl;
		cout << "Press any key to continue. . .";
		cin.get();
		return false;
	}
	
	return true;
}
