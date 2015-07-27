#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <cppunit/CompilerOutputter.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/extensions/HelperMacros.h>

#include "websocket.h"

class Test: public CppUnit::TestFixture {
	CPPUNIT_TEST_SUITE(Test);
	CPPUNIT_TEST(test_derive_key);
	CPPUNIT_TEST_SUITE_END();
public:

	void test_derive_key(){
		const std::string key = "dGhlIHNhbXBsZSBub25jZQ==";
		const std::string expected = "s3pPLMBiTxaQ9kYGzzhZRbK+xOo=";
		CPPUNIT_ASSERT_EQUAL(expected, std::string(websocket_derive_key(key.c_str())));
	}
};

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

int main(int argc, const char* argv[]){
	CppUnit::Test *suite = CppUnit::TestFactoryRegistry::getRegistry().makeTest();
	CppUnit::TextUi::TestRunner runner;

	runner.addTest( suite );
	runner.setOutputter(new CppUnit::CompilerOutputter(&runner.result(), std::cerr));
	return runner.run() ? 0 : 1;
}
