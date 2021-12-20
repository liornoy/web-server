#define _CRT_SECURE_NO_WARNINGS
#pragma once
#include <iostream>
#include <string>
#include <stdio.h>
#include <time.h>
using namespace std;
namespace web_server {
	enum Level { Info, Err, Dbug, Warn };

	inline const char* ToString(Level level)
	{
		switch (level)
		{
		case Info:   return "[INFO]";
		case Err:   return "[ERROR]";
		case Dbug: return "[Debug]";
		case Warn: return "[Warning]";
		default:      return "[Unknown Level_type]";
		}
	}

	class Logger {
	public:
		static Logger& instance() {
			static Logger instance; // Guaranteed to be destroyed.
								  // Instantiated on first use.
			return instance;
		}

		void log(Level level, string msg) {
			cout << currentDateTime() << " " << ToString(level) << ":\t" << msg << endl;
		}

		Logger() {}

	private:
		Logger(Logger const&) = delete;
		void operator=(Logger const&) = delete;
		const std::string currentDateTime();
	};
}
