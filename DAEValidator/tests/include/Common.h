#pragma once

#include <SDKDDKVer.h>
#include <CppUnitTest.h>
#include <string>

//C4628: digraphs not supported with -Ze. Character sequence '<:' not interpreted as alternate token for '['
#pragma warning(disable:4628)

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace opencollada_test
{
	std::string data_path(const std::string & relative_path = std::string());
}