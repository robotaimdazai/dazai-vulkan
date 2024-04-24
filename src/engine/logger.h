#include <iostream>
#include <fstream>
#include <string>
#include <vulkan/vulkan.h>

namespace dazai_engine
{

    enum class LogLevel { info, warning, error, validation };
    class logger
    {
    public:
        logger(const std::string& log_file_name = "logs.txt") : logFile(log_file_name) {}

        template<typename... Args>
        auto log(LogLevel level, const char* file, int line, Args&&... args) -> void
        {
            std::string levelStr;
            switch (level)
            {
            case LogLevel::info:
                levelStr = "INFO";
                break;
            case LogLevel::warning:
                levelStr = "WARNING";
                break;
            case LogLevel::error:
                levelStr = "ERROR";
                break;
            case LogLevel::validation:
                levelStr = "VK_VALIDATION";
                break;
            }

            logHelper(std::forward<Args>(args)...);
            logFile << "[" << levelStr << "] " << file << ":" << line << std::endl;
            std::cout << "[" << levelStr << "] " << file << ":" << line << std::endl << std::endl;

        }


    private:
        std::ofstream logFile;

        template<typename T>
        auto logHelper(T&& arg) ->void
        {
            logFile << arg;
            std::cout << arg;
        }

        template<typename T, typename... Args>
        auto logHelper(T&& arg, Args&&... args)->void
        {
            logFile << arg << " ";
            std::cout << arg << " ";
            logHelper(std::forward<Args>(args)...);
        }
    };



}
extern dazai_engine::logger g_logger;
#define LOG_INFO(...)    g_logger.log(dazai_engine::LogLevel::info,    __FILE__, __LINE__, __VA_ARGS__)
#define LOG_WARNING(...) g_logger.log(dazai_engine::LogLevel::warning, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_ERROR(...)   g_logger.log(dazai_engine::LogLevel::error,   __FILE__, __LINE__, __VA_ARGS__)
#define LOG_VK_VALIDATION(...)   g_logger.log(dazai_engine::LogLevel::validation,"",0,__VA_ARGS__)

#define VKCHECK(result)\
if(result!=0)\
{\
    LOG_ERROR("VULKAN CALL UNSUCESSFULL: ", result);\
}\

auto static VK_DEBUG_CALLBACK(VkDebugUtilsMessageSeverityFlagBitsEXT msgSeverity,
    VkDebugUtilsMessageTypeFlagsEXT msgFlags,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) -> VKAPI_ATTR VkBool32 VKAPI_CALL
{
    LOG_VK_VALIDATION(pCallbackData->pMessage);
    return false;
}


