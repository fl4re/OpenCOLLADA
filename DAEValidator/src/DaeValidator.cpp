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



	int DaeValidator::checkReferencedJointController() const
	{

		XmlNodeSet sourceSkinNodes = mDae.root().selectNodes("//collada:library_controllers/collada:controller/collada:skin/collada:source/collada:Name_array");
		XmlNodeSet visualSceneNodes = mDae.root().selectNodes("//collada:node[@type='JOINT']");

		bool result = true;

		for (const auto& sourceSkinNode : sourceSkinNodes)
		{
			string NameArrayName = sourceSkinNode.attribute("id").value();
			string controllerID = NameArrayName.substr(0, NameArrayName.substr(0, NameArrayName.find_last_of('-')).find_last_of('-'));

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
					cout << "checkReferencedJointController -- Error: " << skinNodeName << "in" << controllerID << " controller is not referenced in the visual scene" << endl;
					result = false;
				}	
			}
		}

		if (result)
			return 0;
		else
			return 2;
	}

	int DaeValidator::checkSkeletonRoots() const
	{

		XmlNodeSet instanceControllers = mDae.root().selectNodes("//collada:instance_controller");
		XmlNodeSet visualSceneNodes = mDae.root().selectNodes("//collada:node[@type='JOINT']");

		bool result = true;

		for (const auto& instanceController : instanceControllers)
		{
			string controllerUrl = instanceController.attribute("url").value();
			size_t posController = controllerUrl.find_last_of('#');

			string researchSkeleton = "//collada:instance_controller[@url=" + string("'") + controllerUrl + "'" + "]//collada:skeleton";
			XmlNodeSet skeletonNodes = mDae.root().selectNodes(researchSkeleton);


			for (const auto& skeletonNode : skeletonNodes)
			{
				string visualNodeId;
				string skeletonNodeName = skeletonNode.text();
				size_t posSkeleton = skeletonNodeName.find_last_of('#');

				bool found = false;

				for (const auto& visualNode : visualSceneNodes)
				{
					visualNodeId = visualNode.attribute("id").value();

					if (posSkeleton != String::npos)
						if (skeletonNodeName.substr(posSkeleton + 1).compare(visualNodeId) == 0)
						{
							found = true;
							break;
						}
				}

				if (!found)
				{

					cout << "checkSkeletonRoots -- Error: " << skeletonNodeName.substr(posSkeleton + 1) << " in " << controllerUrl.substr(posController + 1) << " instance controller is not referenced in the visual scene" << endl;
					result = false;
				}

			}
		}

		if (result)
			return 0;
		else
			return 2;

	}


	int DaeValidator::checkReferencedJointsBySkinController() const
	{
		XmlNodeSet instanceControllers = mDae.root().selectNodes("//collada:instance_controller");
		XmlNodeSet controllers = mDae.root().selectNodes("//collada:controller");

		string controllerUrl;


		bool result = true;

		for (const auto& instanceController : instanceControllers)
		{
			controllerUrl = instanceController.attribute("url").value();
			size_t posController = controllerUrl.find_last_of('#');

			for (const auto& controller : controllers)
			{
				if (controller.attribute("id").value().compare(controllerUrl.substr(posController + 1)) == 0)
				{
					string researchSkinNodes = "//collada:Name_array[@id=" + string("'") + controller.attribute("id").value() + "-joints-array" + "'" + "]";
					XmlNodeSet sourceSkinNodes = controller.selectNodes(researchSkinNodes);

					for (const auto& sourceSkinNode : sourceSkinNodes)
					{
						string skin = sourceSkinNode.text();
						std::vector<String> skinNodes;

						Utils::split(skin, " ", skinNodes);
						string visualNodeName;
						string skinNodeName;

						for (const auto& node : skinNodes)
						{
							string researchSkeleton = "//collada:instance_controller[@url=" + string("'") + controllerUrl + "'" + "]//collada:skeleton";
							XmlNodeSet skeletons = mDae.root().selectNodes(researchSkeleton);

							for (const auto& skeleton : skeletons)
							{
								string skeletonName = skeleton.text();
								size_t posSkeleton = skeletonName.find_last_of('#');

								skeletonName = skeletonName.substr(posSkeleton + 1);
								string research2 = "//collada:node[@id=" + string("'") + skeletonName + "'" + "]";
								XmlNodeSet rootNodes = mDae.root().selectNodes(research2);

								for (const auto& rootNode : rootNodes)
								{
									string rootNodeId = rootNode.attribute("id").value();
									string rootNodeName = rootNode.attribute("name").value();

									string research = "//collada:node[@id=" + string("'") + skeletonName + "'" + "]" + "//collada:node[@name=" + string("'") + node + "'" + "]";

									XmlNodeSet resultnodes = rootNode.selectNodes(research);

									if (resultnodes.size() > 0)
									{
										for (const auto& resultnode : resultnodes)
										{
											string name = resultnode.attribute("name").value();
										}
									}
									else
									{
										if (rootNodeName.compare(node))
										{
											result = false;
											cerr << "checkReferencedJointsBySkinController -- Error: " + node + " is not accessible from " + skeletonName + " skeleton root in " + controllerUrl.substr(posController + 1) + " instance controller"<< endl;
										}
									}
								}
							}
						}
					}
				}
			}
		}

		if (result)
			return 0;
		else
			return 2;
	}




	int DaeValidator::checkisSkeletonRootExistToResolveController() const
	{
		XmlNodeSet controllers = mDae.root().selectNodes("//collada:controller");

		bool result = true;

		for (const auto& controller : controllers)
		{
			string controllerUrl = "#" + controller.attribute("id").value();

			string researchSkeleton = "//collada:instance_controller[@url=" + string("'") + controllerUrl + "'" + "]//collada:skeleton";
			XmlNodeSet skeletons = mDae.root().selectNodes(researchSkeleton);

			if (!skeletons.size())
			{
				cout << "checkisSkeletonRootExistToResolveController -- Error: " << controller.attribute("id").value() << " controller has no skeleton to resolve it" << endl;
				result = false;
			}
		}

		if (result)
			return 0;
		else
			return 2;
	}



	int DaeValidator::checkCompleteBindPose() const
	{

		XmlNodeSet sourceSkinNodes = mDae.root().selectNodes("//collada:library_controllers/collada:controller/collada:skin/collada:source/collada:Name_array");
		
		bool result = true;

		for (const auto& sourceSkinNode : sourceSkinNodes)
		{
			string skin = sourceSkinNode.text();
			string arrayId = sourceSkinNode.attribute("id").value();

			std::vector<String> skinNodes;

			Utils::split(skin, " ", skinNodes);
			
			for (const auto& skinNode : skinNodes)
			{
				bool found = false;

				// search parent's node
				string research = "//collada:node[@name=" + string("'") + skinNode + "'" + "]" + "/parent::collada:node";
				XmlNodeSet resultnodes = mDae.root().selectNodes(research);
				for (const auto& node : resultnodes)
				{
					string parentName = node.attribute("name").value();

					// search if it is in the skinNodes
					for (const auto& skinNode1 : skinNodes)
					{
						if (!parentName.compare(skinNode1))
							found = true;
					}
				}
				
				if (!found)
				{
					bool found1 = true;

					// check if skinNode is the local root (all other node are children)
					for (const auto& skinNode1 : skinNodes)
					{
						string research = "//collada:node[@name=" + string("'") + skinNode + "'" + "]" + "//collada:node[@name=" + string("'") + skinNode1 + "'" + "]";
						if (skinNode.compare(skinNode1))
						{
							XmlNodeSet resultnodes = mDae.root().selectNodes(research);

							if (!(resultnodes.size() > 0))
							{
								found1 = false;
								result = false;
								break;
							}
						}
					}

					if (!found1)
					{
						arrayId = arrayId.substr(0, arrayId.substr(0, arrayId.find_last_of('-')).find_last_of('-'));
						cout << "checkCompleteBindPose -- Error in " << arrayId << " controller, " << skinNode << " has no parent defined" << endl;
					}
						
				}
			}
		}

		if (result)
			return 0;
		else
			return 2;
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