#pragma once
#include <filesystem>
#include <string>
#include <map>
#include <vector>
#include <chrono>
#include <ctime>

constexpr char PathSeparator = static_cast<char>(std::filesystem::path::preferred_separator);


using TimePoint = std::chrono::system_clock::time_point;

using Trade = std::tuple<std::string, double, int>;
using TradeVector = std::vector<Trade>;
using TradeVectorItr = TradeVector::iterator;

using Stock = std::map<std::string, TradeVector>;
using StockItr = Stock::iterator;

struct _TRADE
{
	std::string stkSym;
	int numShares;
	TimePoint transTime;
};

using TradeStructVector = std::vector<_TRADE>;
using TradeStructVectorItr = TradeStructVector::iterator;

using TradeVolumeTimePair = std::pair<int, TimePoint>;
using TradeVolumeTimePairVector = std::vector<TradeVolumeTimePair>;
using TradeVolumeTimePairVectorItr = TradeVolumeTimePairVector::iterator;

struct _TOP_STOCK
{
	int totalShares;
	TradeVolumeTimePairVector trades;

	void addTrade(TradeVolumeTimePair tradeVolumeTimePair)
	{
		totalShares += tradeVolumeTimePair.first;
		trades.push_back(tradeVolumeTimePair);
	}

	void removeOldTrades(TimePoint cutoffTime)
	{
		for (TradeVolumeTimePairVectorItr itr = trades.begin(); itr != trades.end();)
		{
			if (itr->second < cutoffTime)
			{
				totalShares -= itr->first;
				itr = trades.erase(itr);
			}
			else
			{
				++itr;
			}
		}
	}
};

using TopStockStruct = std::map<std::string, _TOP_STOCK>;
using TopStockStructItr = TopStockStruct::iterator;

enum WindowType { Console, Graphical, ThreeD };

struct _TICKER_TAPE_ARGS
{
	bool bInteractive = true;
	bool bCleanApp = true;
	std::string path;
	std::string start = "11/01/2025";
	std::string end = "12/20/2025";
	std::string symbolsFilename = "Symbols.csv";
	std::string symbolsURLsFilename = "SymbolsURLs.txt";
	std::string stocksURLsFilename = "StocksURLs.txt";
	std::string combinedStocksFilename = "CombinedStocks.csv";
	std::string parseStocksFilename = "CombinedStocks.csv";

	WindowType windowTypes[];

	_TICKER_TAPE_ARGS() { }
};


bool downloadDataset
(
	Stock& stocks,
	std::map<std::string, std::string>& symbols,
	_TICKER_TAPE_ARGS& args
);

// general prototypes
void clearScreen();
std::string trim(const std::string& s);
const std::string getFullpath(const std::string& path, const std::string& filename);
std::string getDownloadFolder();

std::time_t parseDateToEpoch(const std::string& mmddyyyy);

std::string epoch_to_utc_string(long epoch);

bool timePointToLocalTm(const TimePoint& tp, std::tm& outLocalTm);
std::pair<long long, long long> computeLocalDayEpochRange(const TimePoint& tp);
