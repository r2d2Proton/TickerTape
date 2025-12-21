# Ticker Tape
* Quick demonstration of mixing modern C++ elements containers (tuple/map/vector) and struct

* Time permitting will animate stock trades in 3D:
   + requires preprocessing of historical data since the sources this project uses provides downloads of a single, specific, stock symbol
   + the algorithm will basically download stocks based on a time frame and stocks symbols of interest into individual files
      + then aggregate the downloaded stocks into a single file
      + then play back the stocks from the combined file
   + focusing on Single file, Daily, Weekly, Monthly, and Yearly for local storage

* Windows based with a combination of
   + WinRT
   + WinUI 3

   + DirectX
   + Vulkan
   + OpenGL

   + other graphics elements

## Notes (will come back and clean up my comments)
* Alpha Vantage apiKey demo used to work
   - Alpha Vantage download URL:
   - https://www.alphavantage.co/query?function=TIME_SERIES_INTRADAY&symbol=AMZN&interval=1min&outputsize=full&datatype=csv&apikey=demo

// Filename: /AlphaVantage/Downloads/2018-09-07/intraday_1min_AMZN.csv
//
// Format: timestamp, open, high, low, close, volume
//
// 2018-09-07 15:59:00,1953.0500,1953.2800,1952.2200,1953.1000,63837
// 2018-09-07 15:58:00,1953.1700,1953.7800,1952.9009,1952.9100,33356
// :
// 2018-09-07 09:31:00,1944.4000,1949.3550,1944.4000,1949.3550,52008
// 2018-09-07 09:30:00,1937.9301,1944.8199,1937.8101,1944.0699,215066

* Yahoo Finance has other restrictions
   - maxium of 8 days can be downloaded at a time
   - no more than the past 30 days (if I read it correctly)

// Yahoo Finance download URL:
// https://query1.finance.yahoo.com/v8/finance/chart/AMZN?interval=1m&range=7d
// https ://query1.finance.yahoo.com/v8/finance/chart/IBM?interval=1m&range=7d
// https ://query1.finance.yahoo.com/v8/finance/chart/INTC?interval=1m&range=7d
// https ://query1.finance.yahoo.com/v8/finance/chart/MSFT?interval=1m&range=7d
// https ://query1.finance.yahoo.com/v8/finance/chart/NVDA?interval=1m&range=7d

