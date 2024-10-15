#include <test_plugin/test_plugin.hpp>

int main() {
	test_plugin* test = new test_plugin();
	test->setTreeWidget();
	return 0;
}