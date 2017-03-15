#include "Common.h"
#include "PathUtil.h"

using namespace opencollada;
using namespace std;

namespace path_util_tests
{
	TEST_CLASS(PathUtilTest)
	{
	public:
		TEST_METHOD(SeparatorTest)
		{
#if _WIN32
			Assert::AreEqual(
				Path::Separator(),
				string("\\")
			);
#endif
		}

		TEST_METHOD(JoinTest)
		{
			string a("a");
			string b("b");

			Assert::AreEqual(
				Path::Join(a, b),
				a + Path::Separator() + b
			);

			string a_anti_slash("a\\");

			Assert::AreEqual(
				Path::Join(a_anti_slash, b),
				a_anti_slash + b
			);
		}

		TEST_METHOD(IsDirectoryTest)
		{
			Assert::IsTrue(Path::IsDirectory(data_path("path_util")));
			Assert::IsFalse(Path::IsDirectory(data_path("path_util/dummy")));
			Assert::IsFalse(Path::IsDirectory(data_path("path_util/unknown")));
		}

		TEST_METHOD(ExistsTest)
		{
			Assert::IsTrue(Path::Exists(data_path("path_util")));
			Assert::IsTrue(Path::Exists(data_path("path_util/dummy")));
			Assert::IsFalse(Path::Exists(data_path("path_util/unknown")));
		}

		TEST_METHOD(GetExecutablePathTest)
		{
#if _WIN32
			Assert::AreNotEqual(
				Path::GetExecutablePath().find("\\COMMON7\\IDE\\COMMONEXTENSIONS\\MICROSOFT\\TESTWINDOW\\vstest.executionengine.exe"),
				string::npos
				);
#endif
		}

		TEST_METHOD(GetExecutableDirectoryTest)
		{
			Assert::AreNotEqual(
				Path::GetExecutableDirectory().find("\\COMMON7\\IDE\\COMMONEXTENSIONS\\MICROSOFT\\TESTWINDOW"),
				string::npos
			);
		}

		TEST_METHOD(GetWorkingDirectoryTest)
		{
			Assert::AreNotEqual(
				Path::GetWorkingDirectory().find(
#if defined(_DEBUG)
					"DAEValidator\\tests\\bin\\Debug"
#else
					"DAEValidator\\tests\\bin\\Release"
#endif
					),
				string::npos
			);
		}

		TEST_METHOD(ListDaesTest)
		{
			// non-recursive mode
			auto daes = Path::ListDaes(data_path("path_util"), false);
			Assert::AreEqual(static_cast<size_t>(1), daes.size());

			// recursive mode
			daes = Path::ListDaes(data_path("path_util"), true);
			Assert::AreEqual(static_cast<size_t>(4), daes.size());
		}

		TEST_METHOD(GetAbsolutePathTest)
		{
			Assert::AreEqual(Path::GetWorkingDirectory(), Path::GetAbsolutePath(""));
			Assert::AreEqual(Path::GetWorkingDirectory(), Path::GetAbsolutePath("."));
			Assert::AreEqual(string("/"), Path::GetAbsolutePath("/"));
			Assert::AreEqual(string("/bbb"), Path::GetAbsolutePath("/aaa/../bbb"));
#if _WIN32
			Assert::AreEqual(string("C:\\"), Path::GetAbsolutePath("C:\\"));
			Assert::AreEqual(string("C:\\aaa\\ccc"), Path::GetAbsolutePath("C:\\aaa\\bbb\\..\\ccc"));
#endif
			Assert::AreEqual(Path::Join(Path::GetWorkingDirectory(), string("aaa")), Path::GetAbsolutePath("aaa"));
			Assert::AreEqual(Path::Join(Path::GetWorkingDirectory(), string("bbb")), Path::GetAbsolutePath("aaa/../bbb"));
		}

		TEST_METHOD(RemoveDotSegmentsTest)
		{
			Assert::AreEqual(string(""), Path::RemoveDotSegments(""));
			Assert::AreEqual(string(""), Path::RemoveDotSegments("."));
			Assert::AreEqual(string(""), Path::RemoveDotSegments("./"));
			Assert::AreEqual(string("a"), Path::RemoveDotSegments("./a"));
			Assert::AreEqual(string("a/"), Path::RemoveDotSegments("./a/"));
			Assert::AreEqual(string("a/b"), Path::RemoveDotSegments("./a/b"));
			Assert::AreEqual(string("a/b/"), Path::RemoveDotSegments("./a/b/"));
			Assert::AreEqual(string("a/b/"), Path::RemoveDotSegments("./a/b/."));
			Assert::AreEqual(string("a/b/"), Path::RemoveDotSegments("./a/b/./"));
			Assert::AreEqual(string("a/b/"), Path::RemoveDotSegments("./a/b/./."));
			Assert::AreEqual(string("a/b/c"), Path::RemoveDotSegments("./a/b/././c"));
			Assert::AreEqual(string(""), Path::RemoveDotSegments(".."));
			Assert::AreEqual(string(""), Path::RemoveDotSegments("../"));
			Assert::AreEqual(string("a"), Path::RemoveDotSegments("../a"));
			Assert::AreEqual(string("a/"), Path::RemoveDotSegments("../a/"));
			Assert::AreEqual(string("a/b"), Path::RemoveDotSegments("../a/b"));
			Assert::AreEqual(string("a/b/"), Path::RemoveDotSegments("../a/b/"));
			Assert::AreEqual(string("a/"), Path::RemoveDotSegments("../a/b/.."));
			Assert::AreEqual(string("a/"), Path::RemoveDotSegments("../a/b/../"));
			Assert::AreEqual(string("/"), Path::RemoveDotSegments("../a/b/../.."));
			Assert::AreEqual(string("/c"), Path::RemoveDotSegments("../a/b/../../c"));
			Assert::AreEqual(string("/a/b"), Path::RemoveDotSegments("/a/b"));
			Assert::AreEqual(string("/a/c"), Path::RemoveDotSegments("/a/b/../c"));
			Assert::AreEqual(string("/"), Path::RemoveDotSegments("/a/b/../.."));
			Assert::AreEqual(string("/"), Path::RemoveDotSegments("/a/b/../../"));
			Assert::AreEqual(string("/c/d"), Path::RemoveDotSegments("/a/b/../../c/d"));
			Assert::AreEqual(string("/a/b/e"), Path::RemoveDotSegments("/a/b/./c/../d/./../e"));
#if _WIN32
			Assert::AreEqual(string("\\a\\b\\e"), Path::RemoveDotSegments("\\a\\b\\.\\c\\..\\d\\.\\..\\e"));
#endif
		}

		TEST_METHOD(GetFileBaseTest)
		{
#if _WIN32
			Assert::AreEqual(string("file"), Path::GetFileBase("C:\\dir\\file.ext"));
#endif
			Assert::AreEqual(string("file"), Path::GetFileBase("file"));
			Assert::AreEqual(string("file"), Path::GetFileBase("file.ext"));
		}

		TEST_METHOD(GetFileSizeTest)
		{
			Assert::AreEqual(static_cast<size_t>(4), Path::GetFileSize(data_path("path_util/root.txt")));
			Assert::AreEqual(static_cast<size_t>(0), Path::GetFileSize(data_path("path_util/dummy")));
			Assert::AreEqual(static_cast<size_t>(0), Path::GetFileSize(data_path("path_util/unknown")));
		}
	};
}