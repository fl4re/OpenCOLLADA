#include "Common.h"
#include "Dae.h"

using namespace opencollada;
using namespace std;

namespace dae_tests
{
	TEST_CLASS(DaeTest)
	{
	public:
		TEST_METHOD(DefaultConstructorTest)
		{
			Dae dae;
		}

		// TODO check external dae!!!

		TEST_METHOD(MoveConstructorTest)
		{
			Dae src_dae;
			src_dae.readFile(data_path("dae/MoveConstructorTest.dae"));
			Assert::IsTrue(src_dae);

			Dae dst_dae = move(src_dae);
			Assert::IsFalse(src_dae);
			Assert::IsTrue(dst_dae);
		}

		TEST_METHOD(EqualOperatorTest)
		{
			Dae src_dae;
			src_dae.readFile(data_path("dae/MoveConstructorTest.dae"));
			Assert::IsTrue(src_dae);

			Dae dst_dae;
			dst_dae = move(src_dae);
			Assert::IsFalse(src_dae);
			Assert::IsTrue(dst_dae);
		}

		TEST_METHOD(ReadFileTest)
		{
			Dae src_dae;
			src_dae.readFile(data_path("dae/MoveConstructorTest.dae"));
			Assert::IsTrue(src_dae);
		}


	};
}