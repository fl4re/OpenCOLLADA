#include "DaeValidator.h"
#include "COLLADASWConstants.h"
#include <iostream>
#include <set>

using namespace COLLADASW;
using namespace std;

namespace opencollada
{
	class IdLine
	{
	public:
		IdLine(const string & id, size_t line)
			: mId(id)
			, mLine(line)
		{}

		bool operator < (const IdLine & other) const
		{
			return mId < other.mId;
		}

		const string & getId() const
		{
			return mId;
		}

		size_t getLine() const
		{
			return mLine;
		}

	private:
		string mId;
		size_t mLine = string::npos;
	};
}

namespace std
{
    template<>
    struct less<opencollada::IdLine>
    {
        bool operator () (const opencollada::IdLine& a, const opencollada::IdLine& b) const
        {
            return a < b;
        }
    };
}

namespace opencollada
{
	const char* colladaNamespace141 = "http://www.collada.org/2005/11/COLLADASchema";
	const char* colladaSchema141 = "collada_schema_1_4_1.xsd";

	const char* colladaNamespace15 = "http://www.collada.org/2008/03/COLLADASchema";
	const char* colladaSchema15 = "collada_schema_1_5.xsd";

	DaeValidator::DaeValidator(const Dae & dae)
		: mDae(dae)
	{}

	int DaeValidator::checkAll() const
	{
		int result = 0;
		result |= checkSchema();
		result |= checkUniqueIds();
		return result;
	}

	// Split string by whitespace
	vector<string> Split(const string & s)
	{
		vector<string> parts;
		istringstream iss(s);
		while (iss && !iss.eof())
		{
			string sub;
			iss >> sub;
			parts.emplace_back(sub);
		}
		return parts;
	}

	int DaeValidator::checkSchema(const string & schema_uri) const
	{
		int result = 0;
		if (schema_uri.empty())
		{
			// Get root <COLLADA> element
			auto collada = mDae.root();
			if (!collada)
			{
				cerr << "Can't find document root" << endl;
				return 1;
			}

			if (collada.name() != "COLLADA")
			{
				cerr << "Root element is not <COLLADA>" << endl;
				return 1;
			}

			// Get COLLADA namespace
			auto xmlns = collada.ns();
			if (!xmlns)
			{
				cerr << "COLLADA element has no namespace" << endl;
				return 1;
			}

			// Determine COLLADA version used by input dae file
			auto href = xmlns.href();
			if (href == colladaNamespace141)
			{
				result |= validateAgainstFile(colladaSchema141);
			}
			else if (href == colladaNamespace15)
			{
				result |= validateAgainstFile(colladaSchema15);
			}
			else
			{
				cerr << "Can't determine COLLADA version used by input file" << endl;
				return 1;
			}

			set<string> xsdURLs;

			// Find xsi:schemaLocation attributes in dae and try to validate against specified xsd documents
			auto elements = mDae.root().selectNodes("//*[@xsi:schemaLocation]");
			for (const auto & element : elements)
			{
				if (auto schemaLocation = element.attribute("schemaLocation"))
				{
					vector<string> parts = Split(schemaLocation.value());
					// Parse pairs of namespace/xsd and take second element
					for (size_t i = 1; i < parts.size(); i += 2)
					{
						xsdURLs.insert(parts[i]);
					}
				}
			}

			for (const auto & URL : xsdURLs)
			{
				int tmpResult = validateAgainstFile(URL);
				if (tmpResult == 2)
				{
					std::cout
						<< "Warning: can't load \"" << URL << "\"." << endl
						<< "Some parts of the document will not be validated." << endl;
				}
				else
				{
					result |= tmpResult;
				}
			}
		}
		else
		{
			// Validate against specified schema only
			result |= validateAgainstFile(schema_uri);
		}

		return result;
	}

	ostream & operator << (ostream & o, const COLLADABU::URI & uri)
	{
		o << uri.getURIString();
		return o;
	}

	int DaeValidator::checkUniqueIds() const
	{
		int result = 0;
		XmlNodeSet nodes = mDae.root().selectNodes("//*[@id]");
		set<IdLine> ids;
		for (const auto & node : nodes)
		{
			IdLine id_line(
				node.attribute(CSWC::CSW_ATTRIBUTE_ID).value(),
				node.line()
			);
			auto it = ids.find(id_line);
			if (it != ids.end())
			{
				cerr << mDae.getURI() << ":" << node.line() << ": Duplicated id \"" << id_line.getId() << "\". See first declaration at line " << it->getLine() << "." << endl;
				result |= 1;
			}
			else
			{
				ids.insert(id_line);
			}
		}
		return result;
	}


	int DaeValidator::checkReferencedJointsBySkinController() const
	{
		XmlNodeSet instanceControllers = mDae.root().selectNodes("//collada:instance_controller");
		XmlNodeSet controllers = mDae.root().selectNodes("//collada:controller");
		
		string controllerUrl;


		bool found = false;

		for (const auto& instanceController : instanceControllers)
		{
			controllerUrl = instanceController.attribute("url").value();
			size_t pos = controllerUrl.find_last_of('#');

			for (const auto& controller : controllers)
			{
				if (controller.attribute("id").value().compare(controllerUrl.substr(pos + 1)) == 0)
				{
					XmlNodeSet sourceSkinNodes = controller.selectNodes("//collada:Name_array");

					for (const auto& sourceSkinNode : sourceSkinNodes)
					{

						string skin = sourceSkinNode.text();
						std::vector<String> skinNodes;

						Utils::split(skin, " ", skinNodes);
						string visualNodeName;
						string skinNodeName;

						found = false;

						for (const auto& node : skinNodes)
						{
							string research = "//collada:node[@name=" + string("'") + node + "'" + "]";

							XmlNodeSet skeletons = instanceController.selectNodes("//collada:skeleton");
							for (const auto& skeleton : skeletons)
							{
								string skeletonName = skeleton.text();
								size_t pos = skeletonName.find_last_of('#');

								skeletonName = skeletonName.substr(pos + 1);
								string research2 = "//collada:node[@id=" + string("'") + skeletonName + "'" + "]";
								XmlNodeSet rootNodes = mDae.root().selectNodes(research2);

								for (const auto& rootNode : rootNodes)
								{
									string rootNodeName = rootNode.attribute("id").value();
									XmlNodeSet resultnodes = rootNode.selectNodes(research);

									for (const auto& resultnode : resultnodes)
									{
										string name = resultnode.attribute("name").value();
										if (resultnodes.size())
											found = true;
									}
								}
							}
						}
					}
				}
			}
		}

		return 0;
	}

	int DaeValidator::checkSkeletonRoots() const
	{
		
		XmlNodeSet skeletonNodes = mDae.root().selectNodes("//collada:skeleton");
		XmlNodeSet visualSceneNodes = mDae.root().selectNodes("//collada:node[@type='JOINT']");


		for (const auto& skeletonNode : skeletonNodes)
		{
			string visualNodeId;
			string skeletonNodeName = skeletonNode.text();
			
			bool found = false;
			
			for (const auto& visualNode : visualSceneNodes)
			{
				visualNodeId = visualNode.attribute("id").value();

				size_t pos = skeletonNodeName.find_last_of('#');

				if (pos != String::npos)
				if (skeletonNodeName.substr(pos+1).compare(visualNodeId) == 0)
				{
					found = true;
					break;
				}
			}

			if (!found)
			{
				cout << "Error" << skeletonNodeName << "not referenced in visual scene" << endl;
				return 2;
			}

		}

		return 0;

	}



	int DaeValidator::checkReferencedJointController() const
	{

		XmlNodeSet sourceSkinNodes = mDae.root().selectNodes("//collada:library_controllers/collada:controller/collada:skin/collada:source/collada:Name_array");
		XmlNodeSet visualSceneNodes = mDae.root().selectNodes("//collada:node[@type='JOINT']");


		for (const auto& sourceSkinNode : sourceSkinNodes)
		{
			string skin = sourceSkinNode.text();
			std::vector<String> skinNodes;

			Utils::split(skin, " ", skinNodes);
			string visualNodeName;
			string skinNodeName;

			for (const auto& skinNode : skinNodes)
			{
				bool found = false;

				skinNodeName = skinNode;
				
				for (const auto& visualNode : visualSceneNodes)
				{
					visualNodeName = visualNode.attribute("name").value();

					if (skinNode.compare(visualNodeName) == 0)
					{
						found = true;
						break;
					}
				}

				if (!found)
				{
					cout << "Error" << skinNodeName << "not referenced in visual scene" << endl;
					return 2;
				}	
			}
		}

		return 0;
	}

	int DaeValidator::validateAgainstFile(const string & xsdPath) const
	{
		// Open xsd
		cout << "Validating against " << xsdPath << endl;
		XmlSchema xsd;
		xsd.readFile(xsdPath.c_str());
		if (!xsd)
		{
			cerr << "Error loading " << xsdPath << endl;
			return 2;
		}

		// Validate dae against xsd
		if (!xsd.validate(mDae))
		{
			return 1;
		}

		return 0;
	}
}