# Ticker Tape
* Quick Demonstration of mixing modern C++ elements with struct with containers tuple/map/vector.

* Time permitting will animate stock trades in 3D.
   + requires preprocessing of historical data since source this project uses provides downloads of a single, specific, stock symbol.
   + the algorithm will basically download stocks based on a time frame and stocks symbols of interest into individual files
      + then aggregate the downloaded stocks into a single file
      + then play back the stocks from the combined file
