#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <cppunit/CompilerOutputter.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/extensions/HelperMacros.h>

#include "tweak/tweak.h"
#include "ipc.h"
#include "worker.h"
#include "utils/sha1.h"
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <unistd.h>
#include <fcntl.h>

static struct worker worker;

static void output(const char* str){
	fputs(str, stderr);
}

class Test: public CppUnit::TestFixture {
	CPPUNIT_TEST_SUITE(Test);
	CPPUNIT_TEST(test_ipc_basic);
	CPPUNIT_TEST(test_ipc_payload);
	CPPUNIT_TEST(test_ipc_reset);
	CPPUNIT_TEST(test_ipc_flush);
	CPPUNIT_TEST_SUITE_END();
public:

	void test_ipc_basic(){
		ipc_push(&worker, IPC_TESTING, NULL, 0);

		enum IPC command = ipc_fetch(&worker, NULL, 0);
		CPPUNIT_ASSERT_EQUAL(command, IPC_TESTING);

		assert_empty();
	}

	void test_ipc_payload(){
		static const char src[] = "0123456789";
		ipc_push(&worker, IPC_TESTING, src, sizeof(src));

		char* dst;
		size_t size;
		enum IPC command = ipc_fetch(&worker, (void**)&dst, &size);
		CPPUNIT_ASSERT_EQUAL(IPC_TESTING, command);
		CPPUNIT_ASSERT_EQUAL(sizeof(src), size);
		CPPUNIT_ASSERT_EQUAL(std::string(src), std::string(dst));
		free(dst);

		assert_empty();
	}

	void test_ipc_reset(){
		ipc_push(&worker, IPC_TESTING, NULL, 0);

		char* dst = (char*)1;
		size_t size;
		enum IPC command = ipc_fetch(&worker, (void**)&dst, &size);
		CPPUNIT_ASSERT_EQUAL(command, IPC_TESTING);
		CPPUNIT_ASSERT_EQUAL((size_t)0, size);
		CPPUNIT_ASSERT_EQUAL((char*)NULL, dst);

		assert_empty();
	}

	void test_ipc_flush(){
		static const char src[] = "0123456789";
		ipc_push(&worker, IPC_TESTING, src, sizeof(src));

		enum IPC command = ipc_fetch(&worker, NULL, NULL);
		CPPUNIT_ASSERT_EQUAL(IPC_TESTING, command);

		assert_empty();
	}


	void assert_empty(){
		char buf[64];
		ssize_t r = read(worker.pipe[READ_FD], buf, sizeof(buf));
		CPPUNIT_ASSERT_EQUAL_MESSAGE("pipe read didn't fail when it was expected, probably still have data", (ssize_t)-1, r);
		if ( r == -1 ){
			CPPUNIT_ASSERT_EQUAL_MESSAGE("pipe read should have blocked", true, errno == EAGAIN || errno == EWOULDBLOCK);
		}
	}
};

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

int main(int argc, const char* argv[]){
	CppUnit::Test *suite = CppUnit::TestFactoryRegistry::getRegistry().makeTest();
	CppUnit::TextUi::TestRunner runner;

	tweak_output(output);

	if ( pipe2(worker.pipe, O_NONBLOCK) != 0){ /* non-block so it is possible to easily test that the pipe is actually empty */
		abort();
	}

	runner.addTest( suite );
	runner.setOutputter(new CppUnit::CompilerOutputter(&runner.result(), std::cerr));
	return runner.run() ? 0 : 1;
}
