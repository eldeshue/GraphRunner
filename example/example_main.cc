#include <MyProject/example.hpp>
#include <graph_runner.h>
#include <iostream>

int main() {
	// 테스트 1: C++ 최신 문법(nullptr) 대신 구식 NULL 매크로 사용
	int* pointer = NULL;

	// 테스트 2: C 스타일 캐스팅 사용
	float some_float = 123.45F;
	int some_int = (int)some_float;

	// 변수 사용 (컴파일러 경고 방지)
	if (pointer == nullptr) {
		std::cout << "Value: " << some_int << std::endl;
	}

	std::cout << "2 + 3 = " << MyProject::add(2, 3) << std::endl;
	GR::test_func();

	return 0;
}
