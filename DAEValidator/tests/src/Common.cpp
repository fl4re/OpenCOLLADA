#include "Common.h"

using namespace std;

string data_path(const string & relative_path)
{
	return string("../../data/") + relative_path;
}
