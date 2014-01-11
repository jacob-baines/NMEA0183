NMEA0183
========

I recently found myself needing to parse NMEA sentences from a GlobalSat BU-353 on Windows. Unfortunately, it was difficult to find a minimal library with a permissive license to do so. Luckily, I found this useful little NMEA0183 parser. I've forked it and modified it for my needs. Specifically, this library will convert the latitude and longitude from GPRMC sentences into doubles.

main.cpp shows how to interface with the BU-353 and writes the doubles to screen

This library is released under the BSD 2-Clause License. See license.txt for
more information.
