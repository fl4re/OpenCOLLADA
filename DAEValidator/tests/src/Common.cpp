#include "Common.h"
#include <CodeCoverage\CodeCoverage.h> 

using namespace std;

/*
// Exclude a particular function:  
ExcludeFromCodeCoverage(Exclusion1, L"MyNamespace::MyClass::MyFunction");

// Exclude all the functions in a particular class:  
ExcludeFromCodeCoverage(Exclusion2, L"MyNamespace::MyClass2::*");

// Exclude all the functions generated from a particular template:   
ExcludeFromCodeCoverage(Exclusion3, L"*::MyFunction<*>");

// Exclude all the code from a particular .cpp file:  
ExcludeSourceFromCodeCoverage(Exclusion4, L"*\\unittest1.cpp");
*/
ExcludeSourceFromCodeCoverage(External, L"*\\Externals\\*");
ExcludeSourceFromCodeCoverage(dirent, L"*\\dirent.h");


namespace opencollada_test
{
	string data_path(const string & relative_path)
	{
		return string("../../data/") + relative_path;
	}
}
