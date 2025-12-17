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

bool downloadDataset
(
	Stock& stocks,
	std::map<std::string, std::string>& symbols,
	const std::string& path,
	const std::string& symbolsFilename = "Symbols.csv",
	const std::string& symbolsURLsFilename = "SymbolsURLs.txt",
	const std::string& stocksURLsFilename = "StocksURLs.txt",
	const std::string& combinedStocksFilename = "CombinedStocks.json",
	const std::string& parseStocksFilename = "CombinedStocks.csv"
);

// general prototypes
void clearScreen();
std::string trim(const std::string& s);
const std::string getFullpath(const std::string& path, const std::string& filename);
std::string getDownloadFolder();


