#ifndef LOGGER_H_
#define LOGGER_H_

#define RED "\033[0;31m"
#define GREEN "\033[0;32m"
#define ORANGE "\033[0;33m"
#define BLUE "\033[0;34m"
#define PURPLE "\033[0;35m"
#define CYAN "\033[0;36m"
#define YELLOW "\033[1;33m"
#define RESET "\x1B[0m"

#if 1

#define log(stream, file, line, level, ...)                                    \
    do                                                                         \
    {                                                                          \
        fprintf(stream, "%s %s:%d ", level, file, line);                       \
        fprintf(stream, __VA_ARGS__);                                          \
    } while (0)

#define log_debug(...)                                                         \
    //    log(stdout, __FILE__, __LINE__, BLUE "[DEBUG]" RESET, __VA_ARGS__)
#define log_error(...)                                                         \
    log(stdout, __FILE__, __LINE__, RED "[ERROR]" RESET, __VA_ARGS__)
#define log_warning(...)                                                       \
    log(stdout, __FILE__, __LINE__, YELLOW "[WARNING]" RESET, __VA_ARGS__)
#define log_info(...)                                                          \
    log(stdout, __FILE__, __LINE__, GREEN "[INFO]" RESET, __VA_ARGS__)
#define log_trace(...)                                                         \
    log(stdout, __FILE__, __LINE__, CYAN "[TRACE]" RESET, __VA_ARGS__)

#else

#define log_debug(...)
#define log_error(...)
#define log_warning(...)
#define log_info(...)
#define log_trace(...)

#endif
#endif
