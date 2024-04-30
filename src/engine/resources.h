#pragma once
#include <iostream>
#include <fstream>
#include<filesystem>
#include "dds.h"

namespace dazai_engine
{
	static class resources
	{
	public:
		auto static read_raw_file(const char* filename, uint32_t* length = nullptr)-> char*;
		auto static load_dds_file(const char* filename)->DDSFile*;
	};
}
