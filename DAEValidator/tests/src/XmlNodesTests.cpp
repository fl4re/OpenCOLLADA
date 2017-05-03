#include "Common.h"
#include "XmlDoc.h"
#include "XmlNodes.h"

using namespace opencollada;
using namespace std;

namespace opencollada_test
{
	TEST_CLASS(XmlNodesTest)
	{
	public:
		TEST_METHOD(Constructor)
		{
			{
				auto b = XmlNodeIteratorByName();
				auto e = XmlNodeIteratorByName();
				XmlNodes<XmlNodeIteratorByName> nodes(b, e);
				Assert::IsFalse(nodes);
			}
			{
				XmlDoc doc;
				doc.readFile(data_path("xml/file.dae"));
				auto b = XmlNodeIteratorByName(doc.root());
				auto e = XmlNodeIteratorByName();
				XmlNodes<XmlNodeIteratorByName> nodes(b, e);
				Assert::IsTrue(nodes);
			}
		}

		TEST_METHOD(Begin)
		{
			{
				auto b = XmlNodeIteratorByName();
				auto e = XmlNodeIteratorByName();
				XmlNodes<XmlNodeIteratorByName> nodes(b, e);
				Assert::IsFalse(*nodes.begin());
			}
			{
				XmlDoc doc;
				doc.readFile(data_path("xml/file.dae"));
				auto b = XmlNodeIteratorByName(doc.root());
				auto e = XmlNodeIteratorByName();
				XmlNodes<XmlNodeIteratorByName> nodes(b, e);
				Assert::IsTrue(*nodes.begin() == doc.root());
			}
		}

		TEST_METHOD(End)
		{
			{
				auto b = XmlNodeIteratorByName();
				auto e = XmlNodeIteratorByName();
				XmlNodes<XmlNodeIteratorByName> nodes(b, e);
				Assert::IsFalse(*nodes.end());
			}
			{
				XmlDoc doc;
				doc.readFile(data_path("xml/file.dae"));
				auto b = XmlNodeIteratorByName(doc.root());
				auto e = XmlNodeIteratorByName();
				XmlNodes<XmlNodeIteratorByName> nodes(b, e);
				Assert::IsFalse(*nodes.end());
			}
		}

		TEST_METHOD(OperatorBool)
		{
			{
				auto b = XmlNodeIteratorByName();
				auto e = XmlNodeIteratorByName();
				XmlNodes<XmlNodeIteratorByName> nodes(b, e);
				Assert::IsFalse(nodes);
			}
			{
				XmlDoc doc;
				doc.readFile(data_path("xml/file.dae"));
				auto b = XmlNodeIteratorByName(doc.root());
				auto e = XmlNodeIteratorByName();
				XmlNodes<XmlNodeIteratorByName> nodes(b, e);
				Assert::IsTrue(nodes);
			}
		}
	};
}