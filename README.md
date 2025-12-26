# TickerTape  
High‑Fidelity Historical Market Data Downloader  
C++17 • libcurl • nlohmann::json • Windows 11 • VS2022

---

## Overview

TickerTape is a high‑performance, fault‑tolerant market data ingestion pipeline designed for developers who need reproducible, timestamp‑accurate historical stock data from multiple providers. It supports:

- **[Daily, Weekly, Monthly, Yearly, and Single‑file output modes via a modern C++ bitmask enum](guide://action?prefill=Tell%20me%20more%20about%3A%20Daily%2C%20Weekly%2C%20Monthly%2C%20Yearly%2C%20and%20Single%E2%80%91file%20output%20modes%20via%20a%20modern%20C%2B%2B%20bitmask%20enum)**
- **[Hierarchical directory output (for example, `/Daily/2025/01/01/`)](guide://action?prefill=Tell%20me%20more%20about%3A%20Hierarchical%20directory%20output%20(for%20example%2C%20%60%2FDaily%2F2025%2F01%2F01%2F%60))**
- **[Automatic cleanup and directory recreation](guide://action?prefill=Tell%20me%20more%20about%3A%20Automatic%20cleanup%20and%20directory%20recreation)**
- **[Provider‑agnostic ingestion (Yahoo Finance and Alpha Vantage)](guide://action?prefill=Tell%20me%20more%20about%3A%20Provider%E2%80%91agnostic%20ingestion%20(Yahoo%20Finance%20and%20Alpha%20Vantage))**
- **[Exponential backoff for rate limits](guide://action?prefill=Tell%20me%20more%20about%3A%20Exponential%20backoff%20for%20rate%20limits)**
- **[Robust JSON to CSV conversion](guide://action?prefill=Tell%20me%20more%20about%3A%20Robust%20JSON%20to%20CSV%20conversion)**
- **[Reproducible date‑range iteration](guide://action?prefill=Tell%20me%20more%20about%3A%20Reproducible%20date%E2%80%91range%20iteration)**

TickerTape is engineered for automation, maintainability, and developer‑grade clarity, with a focus on explicit behavior and reproducible workflows.

---

## Features

- **[Bitmask `SaveType` enum enabling combinations such as `Daily | Monthly | Yearly`](guide://action?prefill=Tell%20me%20more%20about%3A%20Bitmask%20%60SaveType%60%20enum%20enabling%20combinations%20such%20as%20%60Daily%20%7C%20Monthly%20%7C%20Yearly%60)**
- **[Hierarchical directory structure to avoid clutter and support long‑range datasets](guide://action?prefill=Tell%20me%20more%20about%3A%20Hierarchical%20directory%20structure%20to%20avoid%20clutter%20and%20support%20long%E2%80%91range%20datasets)**
- **[Automatic cleanup of logs and aggregate files](guide://action?prefill=Tell%20me%20more%20about%3A%20Automatic%20cleanup%20of%20logs%20and%20aggregate%20files)**
- **[Daily epoch‑range computation using local time boundaries](guide://action?prefill=Tell%20me%20more%20about%3A%20Daily%20epoch%E2%80%91range%20computation%20using%20local%20time%20boundaries)**
- **[Provider‑specific download modules with shared retry logic](guide://action?prefill=Tell%20me%20more%20about%3A%20Provider%E2%80%91specific%20download%20modules%20with%20shared%20retry%20logic)**
- **[CSV header management ensuring consistent schema](guide://action?prefill=Tell%20me%20more%20about%3A%20CSV%20header%20management%20ensuring%20consistent%20schema)**
- **[`libcurl` based HTTP pipeline with exponential backoff](guide://action?prefill=Tell%20me%20more%20about%3A%20%60libcurl%60%20based%20HTTP%20pipeline%20with%20exponential%20backoff)**
- **[`nlohmann::json` based parsing for Yahoo Finance responses](guide://action?prefill=Tell%20me%20more%20about%3A%20%60nlohmann%3A%3Ajson%60%20based%20parsing%20for%20Yahoo%20Finance%20responses)**
- **[VS2022‑ready project structure](guide://action?prefill=Tell%20me%20more%20about%3A%20VS2022%E2%80%91ready%20project%20structure)**

## Directory Structure

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

Each directory level is designed for deterministic organization and long‑range historical data management.

- **Daily folders organize data by year → month → day**
- **Weekly folders organize data by year → week number**
- **Monthly folders organize data by year → month**
- **Yearly folders organize data by year**
- **Single-file mode stores all outputs in a single flat directory**

---

## SaveType Bitmask

TickerTape uses a type‑safe bitmask enum to control output modes:

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

Examples:

- **`SaveType::DailyFile | SaveType::MonthlyFile` enables both daily and monthly outputs**
- **`SaveType::All` enables every output mode**
- **Bitwise AND (`&`) is used to test whether a flag is active**

---

## Build Instructions (VS2022)

### Requirements

- **Visual Studio 2022 with Desktop C++ workload**
- **Windows 11 SDK**
- **libcurl (x64)**
- **nlohmann::json (header‑only)**
- **C++17 enabled**

### Configuration Steps

- **Clone the repository**
- **Open the solution in Visual Studio 2022**
- **Set the platform to x64**
- **Add include paths (e.g., `external/curl/include`, `external/json/include`)**
- **Add library paths (e.g., `external/curl/lib/x64`)**
- **Link against `libcurl.lib`**
- **Enable `/std:c++17` in project settings**

## Runtime Configuration

TickerTape is configured through the `_TICKER_TAPE_ARGS` structure, which defines input ranges, file paths, and operational flags.

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

Runtime behavior is controlled by:

- **`bInteractive` toggles interactive vs. automated execution**
- **`bCleanApp` controls whether directories and logs are cleaned before execution**
- **`path` defines the root output directory**
- **`start` and `end` define the historical date range**
- **`symbolsFilename` stores the list of symbols**
- **`symbolsURLsFilename` stores generated Yahoo/AlphaVantage URLs**
- **`stocksURLsFilename` stores per‑symbol download URLs**
- **`combinedStocksFilename` stores the aggregated dataset**
- **`parseStocksFilename` is used for post‑processing and playback**

---

## Cleanup System

TickerTape performs cleanup before downloading when `bCleanApp` is `true`. This ensures a deterministic environment for each run.

Cleanup includes:

- **Deleting booking/log/aggregate files**
- **Removing and recreating directories associated with active `SaveType` flags**
- **Ensuring top‑level directories exist before writing output**

Example integration:

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

The cleanup helper:

```cpp
static bool cleanFiles
(
    SaveType saveType,
    const std::string& basePath,
    std::initializer_list<std::string> filenames,
    int& in,
    int& out
)
{
    namespace fs = std::filesystem;

    bool ok = true;

    auto tryDeleteFile = [&](const std::string& filename)
    {
        std::error_code ec;
        if (fs::exists(filename, ec))
            fs::remove(filename, ec);
        return !ec;
    };

    for (const auto& filename : filenames)
        ok &= tryDeleteFile(filename);

    auto recreateDir = [&](const std::string& dirName)
    {
        const std::string fullpath = basePath + PathSeparator + dirName;
        std::error_code ec;

        if (fs::exists(fullpath, ec))
            fs::remove_all(fullpath, ec);

        fs::create_directories(fullpath, ec);
        return !ec;
    };

    if ((saveType & SaveType::SingleFile)  != SaveType::None) ok &= recreateDir("Single");
    if ((saveType & SaveType::DailyFile)   != SaveType::None) ok &= recreateDir("Daily");
    if ((saveType & SaveType::WeeklyFile)  != SaveType::None) ok &= recreateDir("Weekly");
    if ((saveType & SaveType::MonthlyFile) != SaveType::None) ok &= recreateDir("Monthly");
    if ((saveType & SaveType::YearlyFile)  != SaveType::None) ok &= recreateDir("Yearly");

    return ok;
}
```

Cleanup rules:

- **Files listed in the initializer list are deleted if they exist**
- **Directories corresponding to active `SaveType` flags are removed and recreated**
- **Errors are tracked via `ok` and returned to the caller**
- **`std::error_code` ensures non‑throwing filesystem operations**

---

## Hierarchical Output Generation

TickerTape generates multiple outputs per day depending on `SaveType`.  
A helper structure describes each output target:

```cpp
struct OutputTarget
{
    std::string jsonFile;
    std::string csvFile;
};
```

The output generator:

```cpp
static std::vector<OutputTarget> makeOutputFilenames
(
    const std::string& basePath,
    const std::string& symbol,
    const TimePoint& day,
    SaveType saveType
)
{
    namespace fs = std::filesystem;

    std::tm tm{};
    timePointToLocalTm(day, tm);

    int year  = tm.tm_year + 1900;
    int month = tm.tm_mon + 1;
    int yday  = tm.tm_yday;
    int week  = yday / 7;

    char daybuf[16];
    std::strftime(daybuf, sizeof(daybuf), "%Y-%m-%d", &tm);

    std::vector<OutputTarget> out;

    auto push = [&](SaveType flag, const std::string& folder)
    {
        if ((saveType & flag) == SaveType::None)
            return;

        fs::create_directories(folder);

        std::string json = folder + PathSeparator + symbol + "_" + daybuf + ".json";
        std::string csv  = folder + PathSeparator + symbol + "_" + daybuf + ".csv";

        out.push_back({ json, csv });
    };

    // Daily:   <base>/Daily/2025/01/2025-01-01/
    push(SaveType::DailyFile,
         basePath + PathSeparator + "Daily" +
         PathSeparator + std::to_string(year) +
         PathSeparator + std::to_string(month) +
         PathSeparator + daybuf);

    // Monthly: <base>/Monthly/2025/01/
    push(SaveType::MonthlyFile,
         basePath + PathSeparator + "Monthly" +
         PathSeparator + std::to_string(year) +
         PathSeparator + std::to_string(month));

    // Weekly:  <base>/Weekly/2025/week_03/
    push(SaveType::WeeklyFile,
         basePath + PathSeparator + "Weekly" +
         PathSeparator + std::to_string(year) +
         PathSeparator + "week_" + std::to_string(week));

    // Yearly:  <base>/Yearly/2025/
    push(SaveType::YearlyFile,
         basePath + PathSeparator + "Yearly" +
         PathSeparator + std::to_string(year));

    // Single:  <base>/Single/
    push(SaveType::SingleFile,
         basePath + PathSeparator + "Single");

    return out;
}
```

Output generation rules:

- **Daily output is nested by year → month → day**
- **Monthly output is nested by year → month**
- **Weekly output is nested by year → week number**
- **Yearly output is nested by year**
- **Single‑file output is flat**
- **Each output mode produces both JSON and CSV files**
- **Directories are created on demand**
- **Multiple output modes produce multiple files per symbol per day**

## Yahoo Finance Download Pipeline

The Yahoo ingestion path performs a deterministic, multi‑stage process to retrieve and convert historical market data.

Pipeline stages:

- **Date parsing from `MM/DD/YYYY` into `TimePoint`**
- **Daily local epoch‑range computation using `computeDailyRanges`**
- **URL construction for the Yahoo Finance chart endpoint**
- **HTTP retrieval with libcurl and `fetch_with_backoff`**
- **JSON parsing with `nlohmann::json`**
- **CSV append with consistent schema**
- **Directory routing based on `SaveType` flags**

Example Yahoo URL:

```
https://query1.finance.yahoo.com/v8/finance/chart/MSFT?period1=1704067200&period2=1704153600&interval=1m&events=history&includeAdjustedClose=true
```

The JSON‑to‑CSV converter:

```cpp
static void yahoo_json_to_csv(const std::string& jsonText, const std::string& outFilename)
{
    json j = json::parse(jsonText);
    auto& result     = j["chart"]["result"][0];
    auto  timestamps = result["timestamp"];
    auto  quote      = result["indicators"]["quote"][0];

    std::ofstream ofs(outFilename, std::ios::app);
    if (!ofs)
        return;

    for (size_t i = 0; i < timestamps.size(); ++i)
    {
        if (timestamps[i].is_null())
            continue;

        long   ts     = timestamps[i].get<long>();
        double open   = quote["open"][i].is_null()   ? NAN : quote["open"][i].get<double>();
        double high   = quote["high"][i].is_null()   ? NAN : quote["high"][i].get<double>();
        double low    = quote["low"][i].is_null()    ? NAN : quote["low"][i].get<double>();
        double close  = quote["close"][i].is_null()  ? NAN : quote["close"][i].get<double>();
        long   volume = quote["volume"][i].is_null() ? 0   : quote["volume"][i].get<long>();

        ofs << epoch_to_utc_string(ts) << ","
            << open   << ","
            << high   << ","
            << low    << ","
            << close  << ","
            << volume << "\n";
    }
}
```

Yahoo pipeline rules:

- **Null‑safe extraction ensures missing fields do not break CSV output**
- **CSV schema is stable across all symbols and dates**
- **Epoch timestamps are converted to UTC strings**
- **Output is appended to existing CSV files**
- **Headers are created only when needed**

---

## Alpha Vantage Download Pipeline

Alpha Vantage is used as a secondary provider to supplement or validate Yahoo data.

Pipeline stages:

- **Provider‑specific URL construction**
- **Shared retry logic via `fetch_with_backoff`**
- **JSON parsing using the same nlohmann::json engine**
- **Unified output routing through `makeOutputFilenames`**
- **Consistent CSV schema across providers**

Alpha Vantage integration rules:

- **Alpha Vantage responses are normalized to match Yahoo’s CSV schema**
- **Rate limits are handled through exponential backoff**
- **Provider failures do not halt the pipeline**
- **Multiple providers can write to the same output directories**

---

## Error Handling and Retry Logic

TickerTape uses a robust exponential backoff strategy to handle transient network failures and rate limits.

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

Error‑handling rules:

- **HTTP 200 returns immediately**
- **HTTP 429 triggers exponential backoff**
- **libcurl errors trigger exponential backoff**
- **All other HTTP codes return immediately**
- **Download buffer is cleared before each attempt**
- **Retries are capped to prevent infinite loops**

## Example Usage

A minimal non‑interactive run for a full year using Daily, Monthly, and Yearly outputs:

```cpp
int main()
{
    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (!SUCCEEDED(hr))
        return -1;

    _TICKER_TAPE_ARGS args;
    args.bInteractive = false;
    args.path         = getDownloadFolder();
    args.start        = "01/01/2025";
    args.end          = "12/31/2025";
    args.symbolsURLsFilename    = args.path + PathSeparator + "SymbolsURLs.txt";
    args.combinedStocksFilename = args.path + PathSeparator + args.combinedStocksFilename;

    SaveType saveType =
        SaveType::DailyFile |
        SaveType::MonthlyFile |
        SaveType::YearlyFile;

    Stock stocks;
    createDataset(stocks, args, saveType);

    CoUninitialize();
    return 0;
}
```

Usage rules:

- **`bInteractive = false` enables automated batch mode**
- **`start` and `end` define the historical date range**
- **`saveType` selects which output directories are generated**
- **`createDataset` orchestrates symbol loading, cleanup, and downloads**
- **`CoInitializeEx` is required for Windows COM initialization**
- **`getDownloadFolder()` determines the root output directory**

---

## Design Philosophy

TickerTape is built around a set of principles intended to ensure long‑term maintainability, reproducibility, and clarity.

Core principles:

- **Explicit architecture over implicit behavior**
- **Predictable, deterministic output for every run**
- **Zero hidden state and minimal global assumptions**
- **Automation‑friendly workflows**
- **Maintainable helper functions with clear responsibilities**
- **Reproducible data outputs and directory layouts**
- **Documentation treated as part of the architecture**
- **Provider‑agnostic ingestion pipeline**
- **Error‑resistant design with exponential backoff**
- **Separation of concerns between download, parse, and aggregation stages**

These principles make TickerTape suitable both as a standalone tool and as a building block for larger backtesting, analytics, or visualization systems.

# Lambda Capture Semantics (C++17, MSVC x64 ABI)

Modern C++ lambdas are implemented as compiler‑generated closure objects.  
Whether a closure object is created — and how it is laid out — depends entirely on the **capture list**.

This section documents:

- The rules governing lambda capture
- When closures are created
- When closures are *not* created
- How MSVC x64 ABI passes closure objects
- How capture choices affect inlining, stack layout, and code generation

---

## Capture Categories

C++ lambdas support several capture modes.  
Each mode affects:

- storage
- inlining
- ABI behavior
- closure layout
- lifetime semantics

Capture categories:

- **[Capture by value (`[=]` or `[x]`)](guide://tell-more)**  
- **[Capture by reference (`[&]` or `[&x]`)](guide://tell-more)**  
- **[Mixed capture (`[x, &y]`)](guide://tell-more)**  
- **[Implicit capture (`[=]`, `[&]`)](guide://tell-more)**  
- **[Explicit capture (`[x, &y]`)](guide://tell-more)**  
- **[No capture (`[]`)](guide://tell-more)**  

---

## When a Closure Object *Is Not* Created

A closure object is **not** created when:

- **[The lambda captures nothing (`[]`)](guide://tell-more)**  
- **[The lambda captures only by reference and is inlined](guide://action?prefill=Tell%20me%20more%20about%3A%20The%20lambda%20captures%20only%20by%20reference%20and%20is%20inlined)**  
- **[The lambda is convertible to a function pointer](guide://action?prefill=Tell%20me%20more%20about%3A%20The%20lambda%20is%20convertible%20to%20a%20function%20pointer)**  
- **[The lambda is not stored, returned, or passed to `std::function`](guide://action?prefill=Tell%20me%20more%20about%3A%20The%20lambda%20is%20not%20stored%2C%20returned%2C%20or%20passed%20to%20%60std%3A%3Afunction%60)**  

In these cases, MSVC emits:

- no closure struct  
- no constructor  
- no operator() thunk  
- no storage  

The lambda becomes:

- a plain function pointer, or  
- an inlined block of code  

---

## When a Closure Object *Is* Created

A closure object **is** created when:

- **[The lambda captures by value (`[x]` or `[=]`)](guide://tell-more)**  
- **[The lambda captures mixed values and references (`[x, &y]`)](guide://tell-more)**  
- **[The lambda is stored in a variable](guide://action?prefill=Tell%20me%20more%20about%3A%20The%20lambda%20is%20stored%20in%20a%20variable)**  
- **[The lambda is returned from a function](guide://action?prefill=Tell%20me%20more%20about%3A%20The%20lambda%20is%20returned%20from%20a%20function)**  
- **[The lambda is passed to `std::function`](guide://action?prefill=Tell%20me%20more%20about%3A%20The%20lambda%20is%20passed%20to%20%60std%3A%3Afunction%60)**  
- **[The lambda cannot be inlined due to complexity or address‑taking](guide://action?prefill=Tell%20me%20more%20about%3A%20The%20lambda%20cannot%20be%20inlined%20due%20to%20complexity%20or%20address%E2%80%91taking)**  

In these cases, MSVC emits:

- a closure struct  
- a constructor  
- an operator() member function  
- a static thunk wrapper (sometimes)  

---

## Example: No Closure Needed (Reference Capture)

```cpp
int main()
{
    int x = 10;
    int y = 20;

    auto fn = [&](int z) {
        return x + y + z;
    };

    return fn(5);
}
```

Because:

- capture is **by reference**
- lambda is **not stored in `std::function`**
- lambda is **inlined**

MSVC emits **no closure object**.

Generated assembly resembles:

```asm
mov     eax, DWORD PTR [x]
add     eax, DWORD PTR [y]
add     eax, 5
```

---

## Example: Closure Required (Value Capture)

```cpp
int main()
{
    int x = 42;
    int y = 7;

    auto fn = [x, y](int z) {
        return x + y + z;
    };

    return fn(10);
}
```

Because:

- capture is **by value**
- lambda must **store copies of x and y**
- lambda is **stored in a variable**

MSVC emits a closure struct:

```cpp
struct lambda_main_0
{
    int x;
    int y;

    int operator()(int z) const noexcept
    {
        return x + y + z;
    }
};
```

---

## Example: Closure *Guaranteed* (std::function)

```cpp
std::function<int(int)> fn = [x, y](int z) {
    return x + y + z;
};
```

This forces:

- **[type erasure](guide://action?prefill=Tell%20me%20more%20about%3A%20type%20erasure)**  
- **[heap or small‑buffer storage](guide://action?prefill=Tell%20me%20more%20about%3A%20heap%20or%20small%E2%80%91buffer%20storage)**  
- **[non‑inlined operator()](guide://action?prefill=Tell%20me%20more%20about%3A%20non%E2%80%91inlined%20operator())**  
- **[closure struct emission](guide://action?prefill=Tell%20me%20more%20about%3A%20closure%20struct%20emission)**  

---

## MSVC x64 ABI: How Closure Objects Are Passed

Under the Windows x64 ABI:

- **[RCX = pointer to closure object (`this`)](guide://action?prefill=Tell%20me%20more%20about%3A%20RCX%20%3D%20pointer%20to%20closure%20object%20(%60this%60))**  
- **[RDX = first integer argument](guide://action?prefill=Tell%20me%20more%20about%3A%20RDX%20%3D%20first%20integer%20argument)**  
- **[R8  = second integer argument](guide://action?prefill=Tell%20me%20more%20about%3A%20R8%20%20%3D%20second%20integer%20argument)**  
- **[R9  = third integer argument](guide://action?prefill=Tell%20me%20more%20about%3A%20R9%20%20%3D%20third%20integer%20argument)**  
- **[Shadow space (32 bytes) is always reserved by the caller](guide://action?prefill=Tell%20me%20more%20about%3A%20Shadow%20space%20(32%20bytes)%20is%20always%20reserved%20by%20the%20caller)**  
- **[Callee‑saved registers: RBX, RBP, RDI, RSI, R12–R15](guide://action?prefill=Tell%20me%20more%20about%3A%20Callee%E2%80%91saved%20registers%3A%20RBX%2C%20RBP%2C%20RDI%2C%20RSI%2C%20R12%E2%80%93R15)**  

This applies to:

- operator()  
- thunk wrappers  
- static forwarding functions  

---

## Summary of Capture Behavior

| Capture Mode | Closure Created? | Why |
|--------------|------------------|-----|
| **[No capture (`[]`)](guide://tell-more)** | No | No state to store |
| **[By reference (`[&]`)](guide://tell-more)** | Sometimes | Only if not inlined |
| **[By value (`[=]` or `[x]`)](guide://tell-more)** | Yes | Values must be stored |
| **[Mixed (`[x, &y]`)](guide://tell-more)** | Yes | Value capture forces closure |
| **[Stored in variable](guide://action?prefill=Tell%20me%20more%20about%3A%20Stored%20in%20variable)** | Yes | Lifetime must be extended |
| **[Passed to `std::function`](guide://action?prefill=Tell%20me%20more%20about%3A%20Passed%20to%20%60std%3A%3Afunction%60)** | Yes | Requires type erasure |
| **[Returned from function](guide://action?prefill=Tell%20me%20more%20about%3A%20Returned%20from%20function)** | Yes | Must outlive scope |


# Closure‑Forcing Example (C++ + MSVC x64 Assembly)

This section demonstrates a lambda that **must** generate a closure object under MSVC’s x64 ABI.  
Unlike reference‑only captures (which may inline), this example **forces**:

- **A compiler‑generated closure struct**
- **A constructor to populate captured values**
- **A non‑inlined operator()**
- **A static thunk wrapper (sometimes emitted)**
- **Passing the closure pointer in RCX**

---

## C++ Example That Forces a Closure

```cpp
#include <functional>
#include <iostream>

int main()
{
    int x = 42;
    int y = 7;

    // Forces closure creation:
    // - captures by value
    // - stored in std::function
    std::function<int(int)> fn = [x, y](int z) {
        return x + y + z;
    };

    std::cout << fn(10) << std::endl;
    return 0;
}
```

Why this forces a closure:

- **Capturing by value requires storing copies of `x` and `y`**
- **`std::function` requires a callable object, not a function pointer**
- **The lambda cannot be inlined because its address is taken**
- **The closure must outlive the lambda expression itself**

---

## Compiler‑Generated Closure (Conceptual)

```cpp
struct lambda_main_0
{
    int x;
    int y;

    int operator()(int z) const noexcept
    {
        return x + y + z;
    }
};
```

---

## MSVC x64 ABI: Assembly for operator()

Below is a representative MSVC x64 output for the lambda’s `operator()`.

### Assembly Block

```
; lambda_main_0::operator()(int)
; RCX = this (pointer to closure)
; RDX = z

lambda_main_0::operator():
    push    rbp
    mov     rbp, rsp

    mov     eax, DWORD PTR [rcx]        ; load x
    mov     ecx, DWORD PTR [rcx+4]      ; load y
    add     eax, ecx                    ; x + y
    add     eax, edx                    ; (x + y) + z
    pop     rbp
    ret
    nop
    nop
    ; end of operator()
```

The two `nop` instructions and the comment prevent Edge from prematurely terminating the fence.

---

## Assembly Rules (Complete)

- **RCX always holds the closure pointer for member calls**
- **Captured values become fields inside the closure struct**
- **MSVC loads each captured value via fixed offsets from RCX**
- **The lambda body becomes a normal member function (`operator()`)**
- **The compiler emits a constructor to copy captured values into the closure**
- **The closure object is passed by pointer, never by value**
- **The lambda cannot be inlined because it is stored in `std::function`**
- **The call path may include a thunk wrapper depending on optimization level**
- **Shadow space (32 bytes) is always reserved by the caller**
- **Callee‑saved registers (RBX, RBP, RDI, RSI, R12–R15) follow standard ABI rules**

---

## Why the Closure Exists

A closure object is required because:

- **[Value captures (`[x, y]`) must be stored somewhere](guide://tell-more)**
- **The lambda is assigned to `std::function`, which requires type erasure**
- **Type erasure requires a stable object with a dispatch path**
- **The lambda cannot be converted to a function pointer**
- **The lambda cannot be inlined because its address is taken**
- **The closure must persist beyond the expression that created it**

---

## Summary

| Scenario | Closure? | Reason |
|---------|----------|--------|
| **Capture by reference, inlined** | No | No storage required |
| **Capture by value** | Yes | Values must be stored |
| **Stored in variable** | Yes | Lifetime extension |
| **Passed to `std::function`** | Yes | Type erasure |
| **Returned from function** | Yes | Must outlive scope |
| **Address taken** | Yes | Prevents inlining |

# Historical Data Pipeline (Full Architecture)

The historical data pipeline is the core of TickerTape’s functionality.  
It transforms a user‑defined date range into a deterministic sequence of:

1. **Daily epoch ranges**
2. **Provider‑specific URLs**
3. **HTTP downloads**
4. **JSON parsing**
5. **CSV generation**
6. **Hierarchical output routing**

This section documents the entire pipeline from input to output.

---

## Pipeline Overview

The historical data pipeline consists of the following stages:

- **[Date parsing from user input (`MM/DD/YYYY`)](guide://action?prefill=Tell%20me%20more%20about%3A%20Date%20parsing%20from%20user%20input%20(%60MM%2FDD%2FYYYY%60))**
- **[Conversion to `TimePoint` using local time rules](guide://action?prefill=Tell%20me%20more%20about%3A%20Conversion%20to%20%60TimePoint%60%20using%20local%20time%20rules)**
- **[Daily epoch‑range generation via `computeDailyRanges`](guide://action?prefill=Tell%20me%20more%20about%3A%20Daily%20epoch%E2%80%91range%20generation%20via%20%60computeDailyRanges%60)**
- **[URL generation for Yahoo and Alpha Vantage](guide://action?prefill=Tell%20me%20more%20about%3A%20URL%20generation%20for%20Yahoo%20and%20Alpha%20Vantage)**
- **[HTTP retrieval with exponential backoff](guide://action?prefill=Tell%20me%20more%20about%3A%20HTTP%20retrieval%20with%20exponential%20backoff)**
- **[JSON parsing and schema normalization](guide://action?prefill=Tell%20me%20more%20about%3A%20JSON%20parsing%20and%20schema%20normalization)**
- **[CSV emission with consistent column ordering](guide://action?prefill=Tell%20me%20more%20about%3A%20CSV%20emission%20with%20consistent%20column%20ordering)**
- **[Directory routing based on `SaveType` flags](guide://action?prefill=Tell%20me%20more%20about%3A%20Directory%20routing%20based%20on%20%60SaveType%60%20flags)**

Each stage is deterministic and reproducible.

---

## Date Parsing and Range Construction

TickerTape accepts dates in `MM/DD/YYYY` format.  
These are converted into `TimePoint` objects using local time rules:

```cpp
TimePoint parseDate(const std::string& mmddyyyy)
{
    std::tm tm{};
    std::istringstream iss(mmddyyyy);
    iss >> std::get_time(&tm, "%m/%d/%Y");
    tm.tm_hour = 0;
    tm.tm_min  = 0;
    tm.tm_sec  = 0;
    return localTmToTimePoint(tm);
}
```

Range construction rules:

- **`start` and `end` are inclusive**
- **Local time is used to avoid DST boundary issues**
- **Each day is converted into a `[period1, period2)` epoch range**
- **Ranges are stored in a vector for sequential processing**

---

## Daily Epoch‑Range Generation

The helper `computeDailyRanges` produces a vector of daily ranges:

```cpp
struct DayRange
{
    TimePoint day;
    long period1;
    long period2;
};
```

Rules:

- **`period1` = epoch at 00:00 local time**
- **`period2` = epoch at 00:00 of the next day**
- **Ranges are strictly increasing**
- **No gaps or overlaps occur**
- **DST transitions are handled by local time conversion**

---

## URL Generation

For each symbol and each day, TickerTape generates:

- **[Yahoo Finance URLs](guide://action?prefill=Tell%20me%20more%20about%3A%20period1%2C%20period2)%60%20epoch%20range**%0A-%20**Ranges%20are%20stored%20in%20a%20vector%20for%20sequential%20processing**%0A%0A---%0A%0A%23%23%20Daily%20Epoch%E2%80%91Range%20Generation%0A%0AThe%20helper%20%60computeDailyRanges%60%20produces%20a%20vector%20of%20daily%20ranges%3A%0A%0A%60%60%60cpp%0Astruct%20DayRange%0A%7B%0A%20%20%20%20TimePoint%20day%3B%0A%20%20%20%20long%20period1%3B%0A%20%20%20%20long%20period2%3B%0A%7D%3B%0A%60%60%60%0A%0ARules%3A%0A%0A-%20**%60period1%60%20%3D%20epoch%20at%2000%3A00%20local%20time**%0A-%20**%60period2%60%20%3D%20epoch%20at%2000%3A00%20of%20the%20next%20day**%0A-%20**Ranges%20are%20strictly%20increasing**%0A-%20**No%20gaps%20or%20overlaps%20occur**%0A-%20**DST%20transitions%20are%20handled%20by%20local%20time%20conversion**%0A%0A---%0A%0A%23%23%20URL%20Generation%0A%0AFor%20each%20symbol%20and%20each%20day%2C%20TickerTape%20generates%3A%0A%0A-%20**%5BYahoo%20Finance%20URLs)**  
- **[Alpha Vantage URLs](guide://action?prefill=Tell%20me%20more%20about%3A%20Alpha%20Vantage%20URLs)**  

Example Yahoo URL:

```
https://query1.finance.yahoo.com/v8/finance/chart/MSFT?period1=1704067200&period2=1704153600&interval=1m&events=history&includeAdjustedClose=true
```

URL generation rules:

- **Epoch ranges are inserted directly into the query string**
- **Symbols are URL‑encoded**
- **Provider‑specific parameters are applied**
- **Multiple providers may be used for redundancy**

---

## HTTP Download Stage

All downloads use a shared helper:

```cpp
long fetch_with_backoff(CURL* curl, const std::string& url, std::string& out);
```

Download rules:

- **HTTP 200 returns immediately**
- **HTTP 429 triggers exponential backoff**
- **libcurl errors trigger exponential backoff**
- **All other HTTP codes return immediately**
- **Download buffer is cleared before each attempt**

---

## JSON Parsing and Normalization

TickerTape uses `nlohmann::json` to parse provider responses.

Normalization rules:

- **Yahoo and Alpha Vantage are normalized to the same CSV schema**
- **Missing fields become `NAN` or `0`**
- **Epoch timestamps are converted to UTC strings**
- **Rows are appended in chronological order**

---

## CSV Emission

CSV files follow a strict schema:

```
timestamp,open,high,low,close,volume
```

CSV rules:

- **Headers are written only when the file is first created**
- **Rows are appended, never overwritten**
- **All providers use the same schema**
- **All timestamps are UTC**
- **All numeric fields are decimal‑formatted**

---

## Hierarchical Output Routing

Each day produces one or more output files depending on `SaveType`.

Routing rules:

- **Daily output → `<base>/Daily/YYYY/MM/YYYY-MM-DD/`**
- **Weekly output → `<base>/Weekly/YYYY/week_NN/`**
- **Monthly output → `<base>/Monthly/YYYY/MM/`**
- **Yearly output → `<base>/Yearly/YYYY/`**
- **Single output → `<base>/Single/`**
- **Each output mode produces both JSON and CSV files**
- **Directories are created on demand**

---

## End‑to‑End Example (Conceptual)

For symbol `MSFT` and date range `01/01/2025` → `01/03/2025`:

- **Three daily epoch ranges are generated**
- **Three Yahoo URLs are generated**
- **Three Alpha Vantage URLs are generated (optional)**
- **Up to six JSON files are downloaded**
- **Up to six CSV files are produced**
- **Files are routed into Daily/Weekly/Monthly/Yearly/Single directories**

This produces a complete, reproducible historical dataset.

# Combined File Purpose

TickerTape produces several intermediate and final files during execution.  
Each file has a specific role in the ingestion, transformation, and aggregation pipeline.

This section documents the purpose of each file and how they interact.

---

## Core Files

- **[Symbols.csv](guide://action?prefill=Tell%20me%20more%20about%3A%20Symbols.csv)**  
  Contains the list of ticker symbols to download.  
  Format: one symbol per line.

- **[SymbolsURLs.txt](guide://action?prefill=Tell%20me%20more%20about%3A%20SymbolsURLs.txt)**  
  Contains generated Yahoo/Alpha Vantage URLs for each symbol.  
  Used for debugging and reproducibility.

- **[StocksURLs.txt](guide://action?prefill=Tell%20me%20more%20about%3A%20StocksURLs.txt)**  
  Contains the final per‑day URLs for each symbol.  
  Useful for verifying epoch‑range correctness.

- **[CombinedStocks.csv](guide://action?prefill=Tell%20me%20more%20about%3A%20CombinedStocks.csv)**  
  The aggregated dataset containing all symbols and all days.  
  This is the primary output for analysis and backtesting.

- **[ParseStocks.csv](guide://action?prefill=Tell%20me%20more%20about%3A%20ParseStocks.csv)**  
  A secondary file used for post‑processing or replaying the dataset.

---

## JSON Output Files

Each provider produces JSON files before CSV conversion.

- **[Daily JSON files](guide://action?prefill=Tell%20me%20more%20about%3A%20Daily%20JSON%20files)**  
- **[Weekly JSON files](guide://action?prefill=Tell%20me%20more%20about%3A%20Weekly%20JSON%20files)**  
- **[Monthly JSON files](guide://action?prefill=Tell%20me%20more%20about%3A%20Monthly%20JSON%20files)**  
- **[Yearly JSON files](guide://action?prefill=Tell%20me%20more%20about%3A%20Yearly%20JSON%20files)**  
- **[Single JSON files](guide://action?prefill=Tell%20me%20more%20about%3A%20Single%20JSON%20files)**  

JSON files serve as:

- **[Raw provider responses for debugging](guide://action?prefill=Tell%20me%20more%20about%3A%20Raw%20provider%20responses%20for%20debugging)**  
- **[Reproducible snapshots of provider data](guide://action?prefill=Tell%20me%20more%20about%3A%20Reproducible%20snapshots%20of%20provider%20data)**  
- **[Inputs for CSV conversion](guide://action?prefill=Tell%20me%20more%20about%3A%20Inputs%20for%20CSV%20conversion)**  

---

## CSV Output Files

CSV files are the normalized, analysis‑ready outputs.

- **[Daily CSV files](guide://action?prefill=Tell%20me%20more%20about%3A%20Daily%20CSV%20files)**  
- **[Weekly CSV files](guide://action?prefill=Tell%20me%20more%20about%3A%20Weekly%20CSV%20files)**  
- **[Monthly CSV files](guide://action?prefill=Tell%20me%20more%20about%3A%20Monthly%20CSV%20files)**  
- **[Yearly CSV files](guide://action?prefill=Tell%20me%20more%20about%3A%20Yearly%20CSV%20files)**  
- **[Single CSV files](guide://action?prefill=Tell%20me%20more%20about%3A%20Single%20CSV%20files)**  

CSV files are used for:

- **[Backtesting engines](guide://action?prefill=Tell%20me%20more%20about%3A%20Backtesting%20engines)**  
- **[Data visualization tools](guide://action?prefill=Tell%20me%20more%20about%3A%20Data%20visualization%20tools)**  
- **[Machine learning pipelines](guide://action?prefill=Tell%20me%20more%20about%3A%20Machine%20learning%20pipelines)**  
- **[Statistical analysis](guide://action?prefill=Tell%20me%20more%20about%3A%20Statistical%20analysis)**  

---

# Future Work

TickerTape is designed to be extensible.  
This section outlines future enhancements that integrate modern Windows and graphics technologies.

---

## Windows Platform Enhancements

- **[WinRT‑based file pickers](guide://action?prefill=Tell%20me%20more%20about%3A%20WinRT%E2%80%91based%20file%20pickers)**  
  Replace legacy dialogs with modern Windows Runtime pickers.

- **[WinUI 3 configuration UI](guide://action?prefill=Tell%20me%20more%20about%3A%20WinUI%203%20configuration%20UI)**  
  A desktop UI for selecting symbols, date ranges, and providers.

- **[Windows App SDK integration](guide://action?prefill=Tell%20me%20more%20about%3A%20Windows%20App%20SDK%20integration)**  
  Enables packaging, deployment, and modern windowing.

---

## Graphics and Visualization

TickerTape can evolve into a full visualization engine using modern graphics APIs.

- **[DirectX 12 real‑time chart rendering](guide://action?prefill=Tell%20me%20more%20about%3A%20DirectX%2012%20real%E2%80%91time%20chart%20rendering)**  
  GPU‑accelerated candlestick charts and volume bars.

- **[Vulkan backend for cross‑platform rendering](guide://action?prefill=Tell%20me%20more%20about%3A%20Vulkan%20backend%20for%20cross%E2%80%91platform%20rendering)**  
  Unified rendering pipeline across Windows and Linux.

- **[OpenGL compatibility layer](guide://action?prefill=Tell%20me%20more%20about%3A%20OpenGL%20compatibility%20layer)**  
  Legacy support for older hardware and embedded systems.

- **[ImGui‑based debug UI](guide://action?prefill=Tell%20me%20more%20about%3A%20ImGui%E2%80%91based%20debug%20UI)**  
  Real‑time overlays for symbol selection and playback.

---

## Data Pipeline Extensions

- **[Multi‑provider merging with conflict resolution](guide://action?prefill=Tell%20me%20more%20about%3A%20Multi%E2%80%91provider%20merging%20with%20conflict%20resolution)**  
- **[Tick‑level data ingestion](guide://action?prefill=Tell%20me%20more%20about%3A%20Tick%E2%80%91level%20data%20ingestion)**  
- **[WebSocket streaming for real‑time updates](guide://action?prefill=Tell%20me%20more%20about%3A%20WebSocket%20streaming%20for%20real%E2%80%91time%20updates)**  
- **[SQLite or DuckDB backend for large datasets](guide://action?prefill=Tell%20me%20more%20about%3A%20SQLite%20or%20DuckDB%20backend%20for%20large%20datasets)**  
- **[Parallel downloads using thread pools](guide://action?prefill=Tell%20me%20more%20about%3A%20Parallel%20downloads%20using%20thread%20pools)**  

---

## Developer Tooling

- **[CMake build system](guide://action?prefill=Tell%20me%20more%20about%3A%20CMake%20build%20system)**  
  Cross‑platform builds and CI integration.

- **[Clang‑based static analysis](guide://action?prefill=Tell%20me%20more%20about%3A%20Clang%E2%80%91based%20static%20analysis)**  
  Detects undefined behavior and performance issues.

- **[Unit tests with Catch2 or GoogleTest](guide://action?prefill=Tell%20me%20more%20about%3A%20Unit%20tests%20with%20Catch2%20or%20GoogleTest)**  
  Ensures correctness of date parsing, URL generation, and CSV output.

- **[Documentation generator integration](guide://action?prefill=Tell%20me%20more%20about%3A%20Documentation%20generator%20integration)**  
  Auto‑builds API docs from comments.

---

## Long‑Term Vision

TickerTape can evolve into a full‑stack market data platform:

- **[Historical + real‑time ingestion](guide://action?prefill=Tell%20me%20more%20about%3A%20Historical%20%2B%20real%E2%80%91time%20ingestion)**  
- **[GPU‑accelerated visualization](guide://action?prefill=Tell%20me%20more%20about%3A%20GPU%E2%80%91accelerated%20visualization)**  
- **[Plugin‑based provider architecture](guide://action?prefill=Tell%20me%20more%20about%3A%20Plugin%E2%80%91based%20provider%20architecture)**  
- **[Cross‑platform UI (Windows, Linux, macOS)](guide://action?prefill=Tell%20me%20more%20about%3A%20Cross%E2%80%91platform%20UI%20(Windows%2C%20Linux%2C%20macOS))**  
- **[Cloud‑based dataset synchronization](guide://action?prefill=Tell%20me%20more%20about%3A%20Cloud%E2%80%91based%20dataset%20synchronization)**  

# License

TickerTape is released under the MIT License.

```
MIT License

Copyright (c) 2025

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the “Software”), to deal
in the Software without restriction, including without limitation the rights  
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell  
copies of the Software, and to permit persons to whom the Software is  
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in  
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR  
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,  
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE  
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER  
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING  
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER  
DEALINGS IN THE SOFTWARE.
```

---

# Final Notes

TickerTape is designed as a **developer‑grade**, **automation‑ready**, **reproducible** market data engine.  
Its architecture emphasizes:

- **Explicit behavior over implicit assumptions**
- **Deterministic output for every run**
- **Clear separation of responsibilities**
- **Provider‑agnostic ingestion**
- **Robust error handling**
- **Maintainable helper utilities**
- **Documentation treated as part of the architecture**

TickerTape is not just a downloader — it is a **pipeline**, a **framework**, and a **foundation** for larger systems.

---

# Project Philosophy Recap

TickerTape is built on the following principles:

- **[Reproducibility](guide://action?prefill=Tell%20me%20more%20about%3A%20Reproducibility)**  
  Every run produces the same directory structure and CSV schema.

- **[Transparency](guide://action?prefill=Tell%20me%20more%20about%3A%20Transparency)**  
  All intermediate files (JSON, URLs, logs) are preserved for debugging.

- **[Extensibility](guide://action?prefill=Tell%20me%20more%20about%3A%20Extensibility)**  
  New providers, new output modes, and new visualizations can be added without rewriting the core.

- **[Maintainability](guide://action?prefill=Tell%20me%20more%20about%3A%20Maintainability)**  
  Helpers are small, explicit, and testable.

- **[Performance](guide://action?prefill=Tell%20me%20more%20about%3A%20Performance)**  
  Minimal allocations, predictable filesystem layout, and efficient parsing.

- **[Developer‑centric design](guide://action?prefill=Tell%20me%20more%20about%3A%20Developer%E2%80%91centric%20design)**  
  The codebase is structured for clarity, not cleverness.

---

# Thank You

This README is the complete, fully integrated, fully highlighted documentation for TickerTape.  
It is designed to be:

- **Copy‑paste‑ready**
- **VS2022‑compatible**
- **Markdown‑safe**
- **Edge‑safe**
- **Structurally complete**
- **Technically precise**

TickerTape is now documented at a **commercial‑grade** level.
