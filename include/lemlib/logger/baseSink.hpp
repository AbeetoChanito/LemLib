#pragma once

#include "message.hpp"

#define FMT_HEADER_ONLY
#include "fmt/core.h"
#include "fmt/args.h"

#include "pros/rtos.hpp"

#include <initializer_list>

namespace lemlib {
/**
 * @brief A base for any sink in LemLib to implement
 *
 */
class BaseSink {
    public:
        BaseSink() = default;

        /**
         * @brief Construct a new combined sink
         *
         * @param sinks The sinks to use
         */
        BaseSink(std::initializer_list<std::shared_ptr<BaseSink>> sinks);

        /**
         * @brief Set the lowest level.
         * If this is a combined sink, this operation will
         * apply for all the parent sinks.
         * @param level
         */
        void setLowestLevel(Level level);

        /**
         * @brief Log a message at the given level
         * If this is a combined sink, this operation will
         * apply for all the parent sinks.
         * @tparam T
         * @param level
         * @param format The format of the message
         * @param args
         */
        template <typename... T> void log(Level level, fmt::format_string<T...> format, T&&... args) {
            if (!sinks.empty()) {
                for (std::shared_ptr<BaseSink> sink : sinks) { sink->log(level, format, std::forward<T>(args)...); }
                return;
            }

            if (level < lowestLevel) { return; }

            // format the message first
            std::string messageString = fmt::format(format, std::forward<T>(args)...);

            Message message = Message {.level = level, .time = pros::millis()};

            // get the arguments
            fmt::dynamic_format_arg_store<fmt::format_context> formattingArgs = getExtraFormattingArgs(message);

            formattingArgs.push_back(fmt::arg("time", message.time));
            formattingArgs.push_back(fmt::arg("level", message.level));
            formattingArgs.push_back(fmt::arg("message", messageString));

            std::string formattedString = fmt::vformat(logFormat, std::move(formattingArgs));

            message.message = std::move(formattedString);

            sendMessage(std::move(message));
        }

        /**
         * @brief Log a message at the debug level.
         * If this is a combined sink, this operation will
         * apply for all the parent sinks.
         * @tparam T
         * @param format
         * @param args
         */
        template <typename... T> void debug(fmt::format_string<T...> format, T&&... args) {
            log(Level::DEBUG, format, std::forward<T>(args)...);
        }

        /**
         * @brief Log a message at the info level
         * If this is a combined sink, this operation will
         * apply for all the parent sinks.
         * @tparam T
         * @param format
         * @param args
         */
        template <typename... T> void info(fmt::format_string<T...> format, T&&... args) {
            log(Level::INFO, format, std::forward<T>(args)...);
        }

        /**
         * @brief Log a message at the warn level
         * If this is a combined sink, this operation will
         * apply for all the parent sinks.
         * @tparam T
         * @param format
         * @param args
         */
        template <typename... T> void warn(fmt::format_string<T...> format, T&&... args) {
            log(Level::WARN, format, std::forward<T>(args)...);
        }

        /**
         * @brief Log a message at the error level.
         * If this is a combined sink, this operation will
         * apply for all the parent sinks.
         * @tparam T
         * @param format
         * @param args
         */
        template <typename... T> void error(fmt::format_string<T...> format, T&&... args) {
            log(Level::ERROR, format, std::forward<T>(args)...);
        }

        /**
         * @brief Log a message at the fatal level
         * If this is a combined sink, this operation will
         * apply for all the parent sinks.
         * @tparam T
         * @param format
         * @param args
         */
        template <typename... T> void fatal(fmt::format_string<T...> format, T&&... args) {
            log(Level::FATAL, format, std::forward<T>(args)...);
        }
    protected:
        /**
         * @brief Log the given message
         *
         * @param message
         */
        virtual void sendMessage(const Message& message);

        /**
         * @brief Set the format of the logger
         *
         * @param format
         */
        void setFormat(const std::string& format);

        /**
         * @brief Get the arguments for formatting
         *
         * @return fmt::dynamic_format_arg_store<fmt::format_context>
         */
        virtual fmt::dynamic_format_arg_store<fmt::format_context> getExtraFormattingArgs(const Message& messageInfo);
    private:
        Level lowestLevel = Level::DEBUG;
        std::string logFormat;

        std::vector<std::shared_ptr<BaseSink>> sinks {};
};
} // namespace lemlib