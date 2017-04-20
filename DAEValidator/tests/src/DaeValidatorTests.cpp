#include "Common.h"
#include "DaeValidator.h"
#include "PathUtil.h"

using namespace opencollada;
using namespace std;

namespace opencollada_test
{
	TEST_CLASS(DaeValidatorTest)
	{
	public:
		TEST_METHOD(Constructor)
		{
			list<string> daes{Path::GetAbsolutePath(data_path("daevalidator/file.dae"))};
			DaeValidator v(daes);
			Assert::AreEqual(v.checkAll(), 0);
		}
	};
}