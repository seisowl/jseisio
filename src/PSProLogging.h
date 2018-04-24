#ifndef PSPROLOGGING_H
#define PSPROLOGGING_H

#ifdef HAVE_LOG4CPLUS

#include <log4cplus/logger.h>
#include <log4cplus/consoleappender.h>
#include <stdio.h>

/// Get the default root logger. Use this if you don't have your own logger (created using DECLARE_LOGGER() ):
#define LOGGER (log4cplus::Logger::getRoot())

/// Create your own logger as a file-global variable:
#define DECLARE_LOGGER(_logger)     static log4cplus::Logger _logger __attribute__((unused)) = log4cplus::Logger::getInstance(LOG4CPLUS_TEXT(#_logger))

/// Set the current log level of a logger, messages below the current level will not be logged
#define SET_LOG_LEVEL(_logger, _level)  _logger.setLogLevel(log4cplus::_level##_LOG_LEVEL)

/** Log with arbitrary log messages at the different log levels:
 * TRACE messages are for developers
 * INFO, ERROR and FATAL might stay enabled also in the release version, so users could send us the log file
 * with these messages if something goes wrong.
 */
#define TRACE_PRINTF(_logger, ...)  LOG4CPLUS_TRACE_PRINTF(_logger, __VA_ARGS__)
#define INFO_PRINTF(_logger, ...)   LOG4CPLUS_INFO_PRINTF(_logger, __VA_ARGS__)
#define ERROR_PRINTF(_logger, ...)  LOG4CPLUS_ERROR_PRINTF(_logger, __VA_ARGS__)
#define FATAL_PRINTF(_logger, ...)  LOG4CPLUS_FATAL_PRINTF(_logger, __VA_ARGS__)

/// Print an ENTER and EXIT function log, only for loglevel TRACE:
#define TRACE_METHOD(_logger)       LOG4CPLUS_TRACE_METHOD(_logger, __PRETTY_FUNCTION__)

/// Print an ENTER function log, only for loglevel INFO:
#define INFO_METHOD()               LOG4CPLUS_INFO_STR(LOGGER, __PRETTY_FUNCTION__)

/// Print the current function name and line number:
#define TRACE_LINE(_logger)         TRACE_PRINTF(_logger, "%s %d", __PRETTY_FUNCTION__, __LINE__)

/// Print the value of a variable at TRACE loglevel:
#define TRACE_VAR(_logger, _var)                         LOG4CPLUS_TRACE(_logger, __PRETTY_FUNCTION__ << " " <<__LINE__\
                         << ": " << #_var << "=" << _var )

#define TRACE_VAR2(_logger, _var1, _var2)                LOG4CPLUS_TRACE(_logger, __PRETTY_FUNCTION__ << " " <<__LINE__\
                         << ": " << #_var1 << "=" << _var1 << ", " << #_var2 << "=" << _var2 )

#define TRACE_VAR3(_logger, _var1, _var2, _var3)         LOG4CPLUS_TRACE(_logger, __PRETTY_FUNCTION__ << " " <<__LINE__\
                         << ": " << #_var1 << "=" << _var1 << ", " << #_var2 << "=" << _var2 \
                         << ", " << #_var3 << "=" << _var3 )

#define TRACE_VAR4(_logger, _var1, _var2, _var3, _var4)  LOG4CPLUS_TRACE(_logger, __PRETTY_FUNCTION__ << " " <<__LINE__\
                         << ": " << #_var1 << "=" << _var1 << ", " << #_var2 << "=" << _var2 \
                         << ", " << #_var3 << "=" << _var3 << ", " << #_var4 << "=" << _var4)

/** Print the values contained in a container, works for all STL-compatible single-value containers (i.e. not for maps)
 * float x[128];
 * ...
 * TRACE_ITER(LOGGER, const float*, &x[0], 128);
 * and for a set:
 * std::set<int> s;
 * ...
 * TRACE_ITER(LOGGER, std::set<int>::const_iterator, s.begin(), s.size()); */
#define TRACE_ITER(_logger, _itType, _start, _count)                                             \
  do {                                                                                           \
    if((_logger).isEnabledFor(log4cplus::TRACE_LOG_LEVEL)) {                                     \
      log4cplus::tostringstream _log4cplus_buf;                                                  \
      _log4cplus_buf << __PRETTY_FUNCTION__ << " " <<__LINE__  << ": "                           \
                     << #_start "[" <<_count << "]: ";                                           \
      _itType it = (_start);                                                                     \
      for (int i=0; i<(_count); i++) {                                                           \
        _log4cplus_buf << *it << "; ";                                                           \
        ++it;                                                                                    \
      }                                                                                          \
      (_logger).forcedLog(log4cplus::TRACE_LOG_LEVEL, _log4cplus_buf.str(), __FILE__, __LINE__); \
    }                                                                                            \
  } while(0)

// see http://www.decompile.com/cpp/faq/file_and_line_error_string.htm for an explanation of the following two lines:
#define __TO_STRING2(_x) #_x
#define __TO_STRING(_x) __TO_STRING2(_x)

/// Create a log message (FATAL level) if the expression _exp is no true:
#define LOG_ASSERT(_logger, _exp) _logger.assertion( _exp , "ASSERTION  \'" #_exp "\'  FAILED ! " \
                                                            "(in " __FILE__ ":" __TO_STRING(__LINE__) ")" )

/// Use the following to setup log4cplus logging in unit tests, otherwise the logging output will not appear anywhere:
#define SETUP_TEST_LOGGING()                                                              \
{                                                                                         \
  log4cplus::Logger::getRoot().setLogLevel(log4cplus::TRACE_LOG_LEVEL);                   \
  log4cplus::SharedAppenderPtr append_2(new log4cplus::ConsoleAppender(true, true));      \
  append_2->setName(LOG4CPLUS_TEXT("Stderr"));                                            \
  log4cplus::Logger::getRoot().addAppender(append_2);                                     \
}

#else // implementation of the logging macros without log4cplus, simply using stderr/cerr

#include <stdio.h>
#include <iostream>

#define LOGGER 

/// Create your own logger as a file-global variable:
#define DECLARE_LOGGER(_logger)     static int _logger __attribute__((unused)) = 0

/// Set the current log level of a logger, messages below the current level will not be logged
#define SET_LOG_LEVEL(_logger, _level)  {}

/** Log with arbitrary log messages at the different log levels:
 * TRACE messages are for developers
 * INFO, ERROR and FATAL might stay enabled also in the release version, so users could send us the log file
 * with these messages if something goes wrong.
 */
#define INFO_PRINTF(_logger, ...)   {fprintf(stderr, "INFO: "); fprintf(stderr, __VA_ARGS__); fprintf(stderr, "\n");}
#define ERROR_PRINTF(_logger, ...)  {fprintf(stderr, "ERROR: (%s : %d) ", __PRETTY_FUNCTION__, __LINE__); fprintf(stderr, __VA_ARGS__); fprintf(stderr, "\n");}
#define FATAL_PRINTF(_logger, ...)  {fprintf(stderr, "FATAL: "); fprintf(stderr, __VA_ARGS__); fprintf(stderr, "\n");}

/// Print an ENTER function log, only for loglevel INFO:
#define INFO_METHOD()               fprintf(stderr, "INFO: %s\n", __PRETTY_FUNCTION__)

#if defined(LOG4CPLUS_DISABLE_DEBUG) || defined(LOG4CPLUS_DISABLE_TRACE)

#define TRACE_METHOD(_logger)                           {}
#define TRACE_PRINTF(_logger, ...)                      {}
#define TRACE_LINE(_logger)                             {}
#define TRACE_METHOD(_logger)                           {}
#define TRACE_VAR(_logger, _var)                        {}
#define TRACE_VAR2(_logger, _var1, _var2)               {}
#define TRACE_VAR3(_logger, _var1, _var2, _var3)        {}
#define TRACE_VAR4(_logger, _var1, _var2, _var3, _var4) {}
#define TRACE_ITER(_logger, _itType, _start, _count)    {}

#else

/// Print an ENTER and EXIT function log, only for loglevel TRACE:
#define TRACE_METHOD(_logger)       fprintf(stderr, "TRACE: %s\n", __PRETTY_FUNCTION__)

#define TRACE_PRINTF(_logger, ...)  {fprintf(stderr, "TRACE: "); fprintf(stderr, __VA_ARGS__); fprintf(stderr, "\n");}

/// Print the current function name and line number:
#define TRACE_LINE(_logger)     {fprintf(stderr, "TRACE: "); fprintf(stderr, "%s : line %d\n", __FILE__, __LINE__);}

/// Print an ENTER and EXIT function log, only for loglevel TRACE:
#define TRACE_METHOD(_logger)       fprintf(stderr, "TRACE: %s\n", __PRETTY_FUNCTION__)

/// Print the value of a variable at TRACE loglevel:
#define TRACE_VAR(_logger, _var)                       std::cerr << "TRACE: " << __PRETTY_FUNCTION__ << " " <<__LINE__\
                         << ": " << #_var << "=" << _var << std::endl;

#define TRACE_VAR2(_logger, _var1, _var2)             std::cerr << "TRACE: " << __PRETTY_FUNCTION__ << " " <<__LINE__ \
                         << ": " << #_var1 << "=" << _var1 << ", " << #_var2 << "=" << _var2 << std::endl;

#define TRACE_VAR3(_logger, _var1, _var2, _var3)        std::cerr << "TRACE: " << __PRETTY_FUNCTION__ << " " <<__LINE__\
                         << ": " << #_var1 << "=" << _var1 << ", " << #_var2 << "=" << _var2 \
                         << ", " << #_var3 << "=" << _var3 << std::endl;

#define TRACE_VAR4(_logger, _var1, _var2, _var3, _var4) std::cerr << "TRACE: " << __PRETTY_FUNCTION__ << " " <<__LINE__\
                         << ": " << #_var1 << "=" << _var1 << ", " << #_var2 << "=" << _var2 \
                         << ", " << #_var3 << "=" << _var3 << ", " << #_var4 << "=" << _var4 << std::endl;

#define TRACE_ITER(_logger, _itType, _start, _count)                                             \
  do {                                                                                           \
    std::cerr << "TRACE: " << __PRETTY_FUNCTION__ << " " <<__LINE__  << ": "                     \
              << #_start "[" <<_count << "]: ";                                                  \
    _itType it = (_start);                                                                       \
    for (int i=0; i<(_count); i++) {                                                             \
      std::cerr << *it << "; ";                                                                  \
      ++it;                                                                                      \
    }                                                                                            \
  } while(0)

#endif  // DISABLE

#endif // stderr

#endif // file
