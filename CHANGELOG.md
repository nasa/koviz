# Koviz 0.1.1 (2026-07-15)

This patch is to incorporate EGs fork 
of koviz to support syncing koviz 
to EG's Virgo tool.

## Enhancements
* Added EG Virgo support

## Fixes
* Smoother video/data sync

# Koviz 0.1.0 (2026-07-08)

## Enhancements
* Added built-in version information displayed in the About menu and via `koviz -version`
* Added Python-like hash comments for DP files
* Tab detach with synchronized koviz windows
* Added support for Trick Hdf5, XLS and Parquet
* Support for Monte Carlo in single data file
* Revamped menus with About and Documentation
* Better video/data window/time synchronization
* Speedup large dataset loading and interaction
* Added 50+ unit tests
* Added "HLines" to DP for horizontal limit lines on plots
* Reduced pdf sizes by using pixmaps
* Better error handling of DP syntax errors
* Greatly sped up CSV loading using memory maps
* Enhanced plot layouts
* Introduced exporting plots to JPG via commandline option
* Dump a set of values for set of vars at a given time (-vars2valsAtTime)
* Create csv from a list of variables (-vars2csv)
* Implemented "debounce" technique for smoother variable selection

## Fixes
* Fixed issues with Time <-> Frequency domain toggling
* Fixed DPs not loading on Mac Tahoe (and other Mac issues)
* Fixed intermittent odd crashes due to improper model signal handling
* Fixed mouse hover not always picking up time under cursor
* Cleaned many new compiler warnings
