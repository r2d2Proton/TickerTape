# Ticker Tape
* Quick demonstration of mixing modern C++ elements containers (tuple/map/vector) and struct.

* Time permitting will animate stock trades in 3D.
   + requires preprocessing of historical data since the sources this project uses provides downloads of a single, specific, stock symbol.
   + the algorithm will basically download stocks based on a time frame and stocks symbols of interest into individual files
      + then aggregate the downloaded stocks into a single file
      + then play back the stocks from the combined file

* Windows based with a combination of
   + WinRT
   + WinUI 3

   + DirectX
   + Vulkan
   + OpenGL

   + other graphics elements
