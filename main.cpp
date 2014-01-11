#include "nmea0183.hpp"

#include <string>
#include <conio.h>
#include <windows.h>
#include <iostream>

int main(int, char**)
{
    // open the com port
    HANDLE comHandle = CreateFile(L"COM3", GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (comHandle == INVALID_HANDLE_VALUE)
    {
        std::cerr << "Failed to open the GPS COM Port." << std::endl;
        return EXIT_FAILURE;
    }

    // adjust the settings as needed. These seem to work with my 353
    DCB portSettings;
    memset(&portSettings, 0, sizeof(portSettings));
    portSettings.DCBlength = sizeof(portSettings);
    portSettings.BaudRate = CBR_9600;
    portSettings.ByteSize = 8;
    portSettings.StopBits = ONESTOPBIT;
    portSettings.fParity = false;
    if (!SetCommState(comHandle, &portSettings))
    {
        std::cerr << "Failed adjusting port settings" << std::endl;
        return EXIT_FAILURE;
    }

    // set timeouts for the com read
    COMMTIMEOUTS timeouts;
    memset(&timeouts, 0, sizeof(timeouts));
    timeouts.ReadIntervalTimeout = 1;
    timeouts.ReadTotalTimeoutMultiplier = 1;
    timeouts.ReadTotalTimeoutConstant = 1;
    if (!SetCommTimeouts(comHandle, &timeouts))
    {
        std::cerr << "Failed setting port time-outs" << std::endl;
    }

    DWORD read = 0;
    NMEA0183 gps_parser;
    char buffer[1] = {0};
    while (true)
    {
        ReadFile(comHandle, buffer, sizeof(buffer), &read, NULL);
        if (read > 0)
        {
            gps_parser.update(buffer[0]);
            if (gps_parser.getState() == NMEA0183::NMEA0183_ACCEPT)
            {
                double latitude = 0;
                double longitude = 0;
                if (gps_parser.get_gprmc_lat_long(gps_parser.getSentence(), latitude, longitude))
                {
                    std::cout << latitude << ", " << longitude << std::endl;
                }
            }
        }
    }
    CloseHandle(comHandle);
}
