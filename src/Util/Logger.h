
#ifndef LOGGER_H
#define LOGGER_H

#include <cassert>
#include <format>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>

namespace GraphRunner {
namespace Util {

    using namespace std;

    class Logger {
      private:
        static unique_ptr<Logger> instance;

        ofstream log_file;
        size_t messages_processed;

        // Private constructor for singleton
        Logger( ) : messages_processed(0) {
            log_file.open("log.txt", ios::out | ios::trunc);

            if ( !log_file.is_open( ) ) {
                cerr << "ERROR: Could not open log.txt for writing!" << endl;
            }
        }

        // Delete copy constructor and assignment operator
        Logger(Logger const&) = delete;
        Logger& operator=(Logger const&) = delete;

        template<typename T>
        void format_to_stream(ostringstream& oss, T&& arg) {
            oss << forward<T>(arg);
        }

        template<typename T, typename... Args>
        void format_to_stream(ostringstream& oss, T&& first, Args&&... args) {
            oss << forward<T>(first);
            if ( sizeof...(args) > 0 ) {
                oss << " ";
                format_to_stream(oss, forward<Args>(args)...);
            }
        }

      public:
        ~Logger( ) {
            if ( log_file.is_open( ) ) {
                log_file.flush( );
                log_file.close( );
            }
        }

        static Logger& get_instance( ) {
            if ( !instance ) {
                instance = unique_ptr<Logger>(new Logger( ));
            }
            return *instance;
        }

        static void print_log(string message) {
            auto& logger = get_instance( );

            cout << message << endl;

            if ( logger.log_file.is_open( ) ) {
                logger.log_file << message << endl;
                logger.log_file.flush( ); // Ensure immediate write
                logger.messages_processed++;
            } else {
                cerr << "WARNING: Log file is not open, message lost: "
                     << message << endl;
            }
        }
    };

    template<typename... Args>
    void print_log(std::format_string<Args...> fmt, Args&&... args) {
        string message = std::format(fmt, std::forward<Args>(args)...);
        Logger::print_log(message);
    }

    template<typename... Args>
    void exit_with_message(std::format_string<Args...> fmt, Args&&... args) {
        string message = std::format(fmt, std::forward<Args>(args)...);
        Logger::print_log(message);
        assert(false);
        exit(EXIT_FAILURE);
    }
} // namespace Util
} // namespace GraphRunner

#endif
