#pragma once

struct Logger {
    static Logger& GetLoggerInstance() {
        static Logger instance;
        return instance;
    }
};