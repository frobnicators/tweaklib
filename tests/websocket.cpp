#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <cppunit/CompilerOutputter.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/extensions/HelperMacros.h>

#include "websocket.h"
#include "utils/sha1.h"
#include <cstring>

class Test: public CppUnit::TestFixture {
	CPPUNIT_TEST_SUITE(Test);
	CPPUNIT_TEST(test_derive_key);
	CPPUNIT_TEST(test_sha1);
	CPPUNIT_TEST_SUITE_END();
public:

	void test_derive_key(){
		const std::string key = "dGhlIHNhbXBsZSBub25jZQ==";
		const std::string expected = "s3pPLMBiTxaQ9kYGzzhZRbK+xOo=";
		CPPUNIT_ASSERT_EQUAL(expected, std::string(websocket_derive_key(key.c_str())));
	}

	void test_sha1(){
		struct { const char* string; const char* hash; } tests[] = {
			{"abc", "a9993e364706816aba3e25717850c26c9cd0d89d"},
			{"tweaklib", "018b014fc94cb4ff8fb24fe26f59679e4b591b46"},
		};

		sha1_t s = sha1_new();
		sha1_hash buffer;

		size_t n = sizeof(tests) / sizeof(tests[0]);
		for ( unsigned int i = 0; i < n; i++ ){
			sha1_reset(s);
			sha1_update(s, tests[i].string, strlen(tests[i].string));
			CPPUNIT_ASSERT_EQUAL(std::string(tests[i].hash), std::string(sha1_hash_hex(s, buffer)));
		}

		sha1_free(s);
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
