// Dazai Vulkan.cpp : Defines the entry point for the application.
#include <iostream>
#include "engine/engine.h"
#include "engine/logger.h"

using namespace std;
using namespace dazai_engine;
logger g_logger;
int main()
{
	engine d_engine;
	d_engine.update();
	return 0;
}
