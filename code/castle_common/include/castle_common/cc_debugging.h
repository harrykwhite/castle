#pragma once

#include <iostream>

#define CC_CHECK(X) do { if (!(X)) { std::cout << "CHECK FAILED: " << #X << " in file " << __FILE__ << " at line " << __LINE__ << "." << std::endl; } } while (0)

namespace cc
{

}
