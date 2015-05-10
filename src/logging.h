// -*- C++ -*-
#pragma once
#include <ostream>

enum class log_level {
    info, warn, error, fatal
};

// TODO: turn into a weak symbol that can be overwritten at link time...
std::ostream& get_logger(log_level, const char* filename, unsigned int line);

[[noreturn]]
void fatal_error();

#define ERR_LOGGER get_logger(log_level::error, __FILE__, __LINE__)
#define WARN_LOGGER get_logger(log_level::warn, __FILE__, __LINE__)
#define INFO_LOGGER get_logger(log_level::info, __FILE__, __LINE__)

#define FAIL(msg)                                                             \
    do {                                                                      \
        get_logger(log_level::fatal, __FILE__, __LINE__) << msg << std::endl; \
        fatal_error();                                                        \
    } while (0)
