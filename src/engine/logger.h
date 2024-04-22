#include <iostream>
#include <fstream>
#include <string>

enum class LogLevel { INFO, WARNING, ERROR };
namespace dazai_engine
{
    class logger
    {
    public:
        logger(const std::string& log_file_name = "logs.txt") : logFile(log_file_name) {}

        template<typename... Args>
        void log(LogLevel level, const char* file, int line, Args&&... args)
        {
            std::string levelStr;
            switch (level)
            {
            case LogLevel::INFO:
                levelStr = "INFO";
                break;
            case LogLevel::WARNING:
                levelStr = "WARNING";
                break;
            case LogLevel::ERROR:
                levelStr = "ERROR";
                break;
            }

            logHelper(std::forward<Args>(args)...);
            logFile << "[" << levelStr << "] " << file << ":" << line << std::endl;
            std::cout << "[" << levelStr << "] " << file << ":" << line << std::endl;

        }

    private:
        std::ofstream logFile;

        template<typename T>
        void logHelper(T&& arg)
        {
            logFile << arg;
            std::cout << arg;
        }

        template<typename T, typename... Args>
        void logHelper(T&& arg, Args&&... args)
        {
            logFile << arg << " ";
            std::cout << arg << " ";
            logHelper(std::forward<Args>(args)...);
        }
    };




}
extern dazai_engine::logger g_logger;
#define LOG_INFO(...)    g_logger.log(LogLevel::INFO,    __FILE__, __LINE__, __VA_ARGS__)
#define LOG_WARNING(...) g_logger.log(LogLevel::WARNING, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_ERROR(...)   g_logger.log(LogLevel::ERROR,   __FILE__, __LINE__, __VA_ARGS__)

// Usage example:
// LOG_INFO("Initializing game engine...");
// LOG_WARNING("File not found: ", filename);
// LOG_ERROR("Failed to load texture: ", textureName);
