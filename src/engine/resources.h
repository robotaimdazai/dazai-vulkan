#pragma once
#include <iostream>
#include <fstream>
#include<filesystem>
#include "logger.h"
namespace dazai_engine
{
	static class resources
	{
	public:
		auto static read_raw_file(const char* filename, uint32_t* length = nullptr)-> char*
		{
			auto resolved_path = RESOURCES + std::string(filename);
			char* result = nullptr;
			std::ifstream file(resolved_path, std::ios::ate | std::ios::binary);
			if (!file.is_open())
			{
				//TODO: ASSERT
				LOG_ERROR("Failed to open file:",resolved_path);
				return result;
			}
			//size in bytes
			std::streamsize file_size = file.tellg();
			//move cursor to start
			file.seekg(0);
			//allocate memory
			result = new char[file_size];
			file.read(result, file_size);
			file.close();
			//set length
			*length = static_cast<int>(file_size);// cast bytes to int
			return result;
		}
	};
}
