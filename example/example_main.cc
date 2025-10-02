#include <iostream>
#include <MyProject/example.hpp>
#include <graph_runner.h>

int main() {
	std::cout << "2 + 3 = " << MyProject::add(2, 3) << std::endl;
	GR::test_func();
	return 0;
}
