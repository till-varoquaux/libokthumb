#include <iostream>
#include <cstring>
#include <exception>  // For terminate
#include "src/logging.h"

static const char *shorten_src_path(const char *path) {
    const char *res = strrchr(path, '/');
    if (!res) {
        return path;
    }
    return res + 1;
}

std::ostream &get_logger(log_level lvl, const char *filename,
                         unsigned int line) {
    const char *lvl_name;
    switch (lvl) {
        case log_level::warn:
            lvl_name = "WARNING";
            break;
        case log_level::error:
            lvl_name = "ERROR";
            break;
        case log_level::info:
            lvl_name = "INFO";
            break;
        case log_level::fatal:
            lvl_name = "FATAL";
            break;
    }
    return std::cerr << lvl_name << ":" << shorten_src_path(filename) << "["
                     << line << "]: ";
}

[[noreturn]] void fatal_error() { std::abort(); }
