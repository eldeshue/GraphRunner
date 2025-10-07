#include <MyProject/example.hpp>

#define VOLK_IMPLEMENTATION
#include <volk.h>

namespace MyProject {
	int add(int a, int b) { return a + b; }
	int vk_init_test() { return volkInitialize(); }
} // namespace MyProject
