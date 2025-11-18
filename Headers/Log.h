#pragma once

#include <iostream>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

#define NOMINMAX
#include <windows.h>


#define DEBUG

class Log {
private:

    static std::string getCurrentTimestamp() {
        auto now = std::chrono::system_clock::now();
        std::time_t now_c = std::chrono::system_clock::to_time_t(now);

        std::tm tm{};
#ifdef _WIN32
        localtime_s(&tm, &now_c);
#else
        localtime_r(&now_c, &tm);
#endif

        std::ostringstream oss;
        oss << "[" << std::put_time(&tm, "%Y-%m-%d %H:%M:%S") << "] ";
        return oss.str();
    }

public:
    template<typename T>
    static void logToConsole(const T& value) {
#ifndef DEBUG
        return;
#endif
        std::cout << getCurrentTimestamp() << value << std::endl;
    }

    template<typename T, typename... Args>
    static void logToConsole(const T& first, const Args&... rest) {
#ifndef DEBUG
        return;
#endif
        std::cout << getCurrentTimestamp() << first;
        ((std::cout << rest), ...);
        std::cout << std::endl;
    }

    template<typename T>
    static void errorToConsole(const T& value) {
#ifndef DEBUG
        return;
#endif
        // get console handle
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

        // save console attributes
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        GetConsoleScreenBufferInfo(hConsole, &csbi);
        WORD originalAttributes = csbi.wAttributes;

        // set console color to red
        SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_INTENSITY);
        
        // write message
        std::cout << getCurrentTimestamp() << value << std::endl;

        // reset console attributes
        SetConsoleTextAttribute(hConsole, originalAttributes);
    }

    template<typename T, typename... Args>
    static void errorToConsole(const T& first, const Args&... rest) {
#ifndef DEBUG
        return;
#endif

        // get console handle
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

        // save console attributes
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        GetConsoleScreenBufferInfo(hConsole, &csbi);
        WORD originalAttributes = csbi.wAttributes;

        // set console color to red
        SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_INTENSITY);

        // write message
        std::cout << getCurrentTimestamp() << first;
        ((std::cout << rest), ...);
        std::cout << std::endl;

        // reset console attributes
        SetConsoleTextAttribute(hConsole, originalAttributes);
    }

};