#include "Common.h"

#define CODE_COVERAGE 0

#if CODE_COVERAGE
#include <CodeCoverage/CodeCoverage.h>
ExcludeSourceFromCodeCoverage(External, L"*\\Externals\\*");
ExcludeSourceFromCodeCoverage(dirent, L"*\\dirent.h");
#endif

using namespace std;

namespace opencollada_test
{
	string data_path(const string & relative_path)
	{
		return string("../../data/") + relative_path;
	}
}
