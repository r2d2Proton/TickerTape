# TickerTape  
High‑Fidelity Historical Market Data Downloader  
C++17 • libcurl • nlohmann::json • Windows 11 • VS2022

---

## Overview

TickerTape is a high‑performance, fault‑tolerant market data ingestion pipeline designed for developers who need reproducible, timestamp‑accurate historical stock data from multiple providers. It supports:

- Daily, Weekly, Monthly, Yearly, and Single‑file output modes via a modern C++ bitmask enum  
- Hierarchical directory output (for example, `/Daily/2025/01/01/`)  
- Automatic cleanup and directory recreation  
- Provider‑agnostic ingestion (Yahoo Finance and Alpha Vantage)  
- Exponential backoff for rate limits  
- Robust JSON to CSV conversion  
- Reproducible date‑range iteration  

TickerTape is engineered for automation, maintainability, and developer‑grade clarity, with a focus on explicit behavior and reproducible workflows.

---

## Features

- Bitmask `SaveType` enum enabling combinations such as `Daily | Monthly | Yearly`
- Hierarchical directory structure to avoid clutter and support long‑range datasets
- Automatic cleanup of logs and aggregate files
- Daily epoch‑range computation using local time boundaries
- Provider‑specific download modules with shared retry logic
- CSV header management ensuring consistent schema
- `libcurl` based HTTP pipeline with exponential backoff
- `nlohmann::json` based parsing for Yahoo Finance responses
- VS2022‑ready project structure

---

## Directory structure

TickerTape generates a clean, predictable directory hierarchy under the configured download folder:

```text
<DownloadFolder>/
    Single/
    Daily/
        2025/
            01/
                2025-01-01/
                2025-01-02/
                ...
    Weekly/
        2025/
            week_00/
            week_01/
            ...
    Monthly/
        2025/
            01/
            02/
            ...
    Yearly/
        2025/
```

---

## SaveType bitmask

```cpp
enum class SaveType : uint32_t
{
    None        = 0,
    SingleFile  = 1u << 0,
    DailyFile   = 1u << 1,
    WeeklyFile  = 1u << 2,
    MonthlyFile = 1u << 3,
    YearlyFile  = 1u << 4,
    All         = 0xFFFFFFFFu
};

inline SaveType operator|(SaveType a, SaveType b)
{
    return static_cast<SaveType>(
        static_cast<uint32_t>(a) | static_cast<uint32_t>(b)
    );
}

inline SaveType operator&(SaveType a, SaveType b)
{
    return static_cast<SaveType>(
        static_cast<uint32_t>(a) & static_cast<uint32_t>(b)
    );
}
```

Example:

```cpp
SaveType saveType =
    SaveType::DailyFile |
    SaveType::MonthlyFile |
    SaveType::YearlyFile;
```

---

## Build instructions (VS2022)

### Requirements

- Visual Studio 2022  
- Windows 11 SDK  
- libcurl (x64)  
- nlohmann::json (header‑only)  
- C++17 enabled  

### Steps

1. Clone the repository  
2. Open the solution in VS2022  
3. Set platform to **x64**  
4. Add include paths  
5. Add library paths  
6. Link against `libcurl.lib`  
7. Enable `/std:c++17`  

---

## Runtime configuration

```cpp
struct _TICKER_TAPE_ARGS
{
    bool bInteractive = true;
    bool bCleanApp    = true;

    std::string path;
    std::string start = "11/01/2025";
    std::string end   = "12/20/2025";

    std::string symbolsFilename        = "Symbols.csv";
    std::string symbolsURLsFilename    = "SymbolsURLs.txt";
    std::string stocksURLsFilename     = "StocksURLs.txt";
    std::string combinedStocksFilename = "CombinedStocks.csv";
    std::string parseStocksFilename    = "CombinedStocks.csv";
};
```

---

## Cleanup system

TickerTape deletes:

- booking/log files  
- directories associated with selected SaveType flags  

```cpp
cleanFiles
(
    saveType,
    args.path,
    {
        fullpathSymbolsURLsFilename,
        fullpathStocksURLsFilename,
        fullpathCombinedStocksFilename,
        fullpathParseStocksFilename
    },
    in,
    out
);
```

---

## Hierarchical output generation

TickerTape generates multiple outputs per day depending on SaveType.

```cpp
struct OutputTarget
{
    std::string jsonFile;
    std::string csvFile;
};
```

`makeOutputFilenames()` returns a vector of output targets:

```cpp
auto outputs = makeOutputFilenames(args.path, symbol, day, saveType);

for (const auto& out : outputs)
{
    std::ofstream jf(out.jsonFile);
    jf << downloadBuffer;

    bool exists = std::filesystem::exists(out.csvFile);
    if (!exists)
    {
        std::ofstream hdr(out.csvFile);
        hdr << "timestamp,open,high,low,close,volume\n";
    }

    yahoo_json_to_csv(downloadBuffer, out.csvFile);
}
```

---

## Yahoo Finance download pipeline

- Daily epoch range computation  
- URL construction  
- libcurl download  
- HTTP 429 exponential backoff  
- JSON parsing  
- CSV append  

Example URL:

```
https://query1.finance.yahoo.com/v8/finance/chart/MSFT?period1=1704067200&period2=1704153600&interval=1m&events=history&includeAdjustedClose=true
```

---

## Error handling

TickerTape uses exponential backoff:

```cpp
static long fetch_with_backoff(CURL* curl, const std::string& url, std::string& downloadBuffer, int maxRetries = 6)
{
    int  attempt   = 0;
    long http_code = 0;

    while (attempt <= maxRetries)
    {
        downloadBuffer.clear();
        CURLcode res = downloadURL(curl, url);

        if (res != CURLE_OK)
        {
            std::this_thread::sleep_for(std::chrono::seconds(2 << attempt));
            ++attempt;
            continue;
        }

        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

        if (http_code == 200)
            return http_code;

        if (http_code == 429)
        {
            int wait = (1 << attempt) * 2;
            std::this_thread::sleep_for(std::chrono::seconds(wait));
            ++attempt;
            continue;
        }

        return http_code;
    }

    return http_code;
}
```

---

## Example usage

```cpp
int main()
{
    _TICKER_TAPE_ARGS args;
    args.bInteractive = false;
    args.path = getDownloadFolder();
    args.start = "01/01/2025";
    args.end   = "12/31/2025";

    SaveType saveType =
        SaveType::DailyFile |
        SaveType::MonthlyFile |
        SaveType::YearlyFile;

    Stock stocks;
    createDataset(stocks, args, saveType);
}
```

---

## Design philosophy

TickerTape emphasizes:

- explicit architecture  
- predictable behavior  
- zero hidden state  
- reproducible workflows  
- maintainable helpers  
- documentation as part of the architecture  

---

## License

MIT License.
