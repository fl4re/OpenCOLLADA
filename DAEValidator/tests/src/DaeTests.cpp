#include "Common.h"
#include "Dae.h"
#include "PathUtil.h"
#include "StringUtil.h"

using namespace opencollada;
using namespace std;

namespace opencollada_test
{
	TEST_CLASS(DaeTest)
	{
	public:
		TEST_METHOD(DefaultConstructor)
		{
			Dae dae;
			Assert::IsFalse(dae);
		}

		// TODO check external dae!!!

		TEST_METHOD(MoveConstructor)
		{
			Dae src_dae;
			src_dae.readFile(data_path("dae/MoveConstructorTest.dae"));
			Assert::IsTrue(src_dae);

			Dae dst_dae = move(src_dae);
			Assert::IsFalse(src_dae);
			Assert::IsTrue(dst_dae);
		}

		TEST_METHOD(EqualOperator)
		{
			Dae src_dae;
			src_dae.readFile(data_path("dae/MoveConstructorTest.dae"));
			Assert::IsTrue(src_dae);

			Dae dst_dae;
			dst_dae = move(src_dae);
			Assert::IsFalse(src_dae);
			Assert::IsTrue(dst_dae);
		}

		TEST_METHOD(ReadFile)
		{
			Dae dae;

			dae.readFile(data_path("dae/unknown.dae"));
			Assert::IsFalse(dae);

			dae.readFile(data_path("dae/ReadFileTest_1.4.dae"));
			Assert::IsTrue(dae);

			dae.readFile(data_path("dae/ReadFileTest_1.5.dae"));
			Assert::IsTrue(dae);
		}

		TEST_METHOD(GetVersion)
		{
			Dae dae;
			dae.readFile(data_path("dae/unknown.dae"));
			Assert::IsTrue(dae.getVersion() == Dae::Version::Unknown);

			dae.readFile(data_path("dae/ReadFileTest_1.4.dae"));
			Assert::IsTrue(dae.getVersion() == Dae::Version::COLLADA14);

			dae.readFile(data_path("dae/ReadFileTest_1.5.dae"));
			Assert::IsTrue(dae.getVersion() == Dae::Version::COLLADA15);
		}

		TEST_METHOD(GetIds)
		{
			Dae dae;
			dae.readFile(data_path("dae/ReadFileTest_1.4.dae"));
			const auto & ids = dae.getIds();
			Assert::AreEqual(ids.size(), static_cast<size_t>(25));
		}

		TEST_METHOD(GetURI)
		{
			string dataPath = Path::GetAbsolutePath(data_path());
			Uri dataPathUri = Uri::FromNativePath(dataPath);

			Dae dae;
			dae.readFile(data_path("dae/ReadFileTest_1.4.dae"));
			const Uri & uri = dae.getURI();
			Assert::AreEqual(uri.scheme().c_str(), "file");
			Assert::IsTrue(String::StartsWith(uri.path(), dataPathUri.path()));
			Assert::AreEqual(uri.pathFile().c_str(), "ReadFileTest_1.4.dae");
		}

		TEST_METHOD(GetExternalDAEs)
		{
			Dae dae;
			dae.readFile(data_path("dae/MoveConstructorTest.dae"));
			Assert::AreEqual(dae.getExternalDAEs().size(), static_cast<size_t>(1));
		}

		TEST_METHOD(GetAnyURIs)
		{
			Dae dae;
			dae.readFile(data_path("dae/ReadFileTest_1.4.dae"));
			Assert::AreEqual(dae.getAnyURIs().size(), static_cast<size_t>(25));
		}

		TEST_METHOD(GetIDREFs)
		{
			Dae dae;
			dae.readFile(data_path("dae/ReadFileTest_1.4.dae"));
			Assert::AreEqual(dae.getIDREFs().size(), static_cast<size_t>(1));
		}
	};
}