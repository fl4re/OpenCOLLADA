#include "Macros.h"
#include "DaeValidator.h"
#include "PathUtil.h"
#include "Strings.h"
#include "StringUtil.h"
#include <cmath>
#include <iomanip>
#include "no_warning_iostream"
#include <set>
#include <sstream>

using namespace std;

namespace std
{
	template<>
	struct less<tuple<size_t, string>>
	{
		bool operator () (const tuple<size_t, string>& a, const tuple<size_t, string>& b) const
		{
			return get<1>(a) < get<1>(b);
		}
	};
}

namespace opencollada
{
	extern const char* colladaNamespace141;
	extern const char* colladaSchemaFileName141;
	extern XmlSchema colladaSchema141;

	extern const char* colladaNamespace15;
	extern const char* colladaSchemaFileName15;
	//extern XmlSchema colladaSchema15;

	DaeValidator::DaeValidator(const list<string> & daePaths)
	{
		mDaePaths.reserve(daePaths.size());
		mDaePaths.insert(mDaePaths.end(), daePaths.begin(), daePaths.end());
	}

#if IS_MSVC_AND_MSVC_VERSION_LT(1900)
	typedef unsigned long long uint64_t;
#endif
	static const vector<tuple<uint64_t, string>> table =
	{
		make_tuple(1, "B"),
		make_tuple(1024, "kB"),
		make_tuple(1048576, "MB"),
		make_tuple(1073741824, "GB"),
		make_tuple(1099511627776, "TB")
	};

	class Size
	{
	public:
		Size(size_t size)
			: mSize(size)
		{}

		string str() const
		{
			stringstream s;
			for (const auto & entry : table)
			{
				if (mSize < (get<0>(entry) * 1024))
				{
					s << round(mSize / static_cast<double>(get<0>(entry))) << get<1>(entry);
					break;
				}
			}
			return s.str();
		}

	private:
		size_t mSize = 0;
	};

	int DaeValidator::for_each_dae(const function<int(const Dae &)> & task) const
	{
		int result = 0;
		size_t count = 1;
		for (const auto & daePath : mDaePaths)
		{
			if (mDaePaths.size() > 1)
			{
				cout << "[" << count << "/" << mDaePaths.size() << " " << static_cast<size_t>(static_cast<float>(count) / static_cast<float>(mDaePaths.size()) * 100.0f) << "%]" << endl;
				++count;
			}

			cout << "Processing " << daePath << " (" << Size(Path::GetFileSize(daePath)).str() << ")" << endl;

			Dae dae;
			dae.readFile(daePath);
			if (dae)
			{
				result |= task(dae);
			}
			else
			{
				result |= 1;
				cout << "Error loading " << daePath << endl;
			}
		}
		return result;
	}

	int DaeValidator::checkAll() const
	{
		return for_each_dae([&](const Dae & dae) {
			return checkAll(dae);
		});
	}

	int DaeValidator::checkAll(const Dae & dae) const
	{
		return
			checkSchema(dae) |
			checkUniqueIds(dae) |
			checkUniqueSids(dae) |
			checkLinks(dae) |
			checkSkinController(dae) |
			checkLOD(dae);
	}

	int DaeValidator::checkSchema(const string & schema_uri) const
	{
		if (schema_uri.empty())
		{
			return for_each_dae([&](const Dae & dae) {
				return checkSchema(dae);
			});
		}

		XmlSchema schema;
		schema.readFile(schema_uri);
		if (schema)
		{
			return for_each_dae([&](const Dae & dae) {
				return ValidateAgainstSchema(dae, schema);
			});
		}

		cout << "Error loading " << schema_uri << endl;
		return 1;
	}

	int DaeValidator::checkSchema(const Dae & dae) const
	{
		cout << "Checking schema..." << endl;

		int result = 0;

		// Get root <COLLADA> element
		auto collada = dae.root();
		if (!collada)
		{
			cout << "Can't find document root" << endl;
			return 1;
		}

		if (collada.name() != Strings::COLLADA)
		{
			cout << "Root element is not <" << Strings::COLLADA << ">" << endl;
			return 1;
		}

		// Get COLLADA namespace
		auto xmlns = collada.ns();
		if (!xmlns)
		{
			cout << "COLLADA element has no namespace" << endl;
			return 1;
		}

		// Determine COLLADA version used by input dae file
		auto href = xmlns.href();
		if (href == colladaNamespace141)
		{
			result |= ValidateAgainstSchema(dae, colladaSchema141);
		}
		else if (href == colladaNamespace15)
		{
			//result |= ValidateAgainstSchema(dae, colladaSchema15);
			cout << "COLLADA 1.5 not supported yet." << endl;
			return 1;
		}
		else
		{
			cout << "Can't determine COLLADA version used by input file" << endl;
			return 1;
		}

		vector<XmlNode> subDocs;

		// Find xsi:schemaLocation attributes in dae and try to validate against specified xsd documents
		const auto & elements = dae.root().selectNodes("//*[@xsi:schemaLocation]");
		for (const auto & element : elements)
		{
			if (auto schemaLocation = element.attribute("schemaLocation"))
			{
				vector<string> parts = String::Split(schemaLocation.value());
				// Parse pairs of namespace/xsd
				for (size_t i = 1; i < parts.size(); i += 2)
				{
					const string & ns = parts[i - 1];
					const string & xsdUri = parts[i];

					if (ns != colladaNamespace141 && ns != colladaNamespace15)
					{
						// "insert" does nothing if element already exists.
						mSchemas.insert(pair<string, XmlSchema>(ns, XmlSchema()));
						mSchemaLocations.insert(pair<string, string>(ns, xsdUri));
					}
				}
			}
		}

		// Preload uninitialized .xsd files
		auto itSchema = mSchemas.begin();
		auto itSchemaLocation = mSchemaLocations.begin();
		for (; itSchema != mSchemas.end(); ++itSchema, ++itSchemaLocation)
		{
			const auto & schemaUri = itSchemaLocation->second;
			auto & schema = itSchema->second;

			// Don't try to load schemas that already failed in a previous run
			if (schema.failedToLoad())
				continue;

			string uri = schemaUri;

			if (!schema)
			{
				schema.readFile(uri);
			}

			if (!schema)
			{
				// Try to find schema document in executable directory
				Uri xsdUri(schemaUri);
				if (xsdUri.isValid())
				{
					uri = Path::Join(Path::GetExecutableDirectory(), xsdUri.pathFile());
					schema.readFile(uri);
				}
			}

			if (!schema)
			{
				// Try to find schema document in COLLADA document directory
				Uri xsdUri(schemaUri);
				string xsdFile = xsdUri.pathFile();
				xsdUri = dae.getURI();
				xsdUri.setPathFile(xsdFile);
				uri = xsdUri.str();
				schema.readFile(uri);
			}

			if (schema && uri != schemaUri)
			{
				cout << "Using " << uri << endl;
			}
			else if (!schema)
			{
				cout << "Error loading " << schemaUri << endl;
				result |= 1;
			}
		}

		// Validate "sub documents"
		for (const auto & schema : mSchemas)
		{
			// Ignore schemas that failed to load
			if (!schema.second)
				continue;

			const auto & ns = schema.first;
			stringstream xpath;
			xpath << "//*[namespace-uri()='" << ns << "' and not(namespace-uri(./..)='" << ns << "')]";
			const auto & nodes = dae.root().selectNodes(xpath.str());
			for (auto node : nodes)
			{
				auto old = dae.setRoot(node);
				result |= ValidateAgainstSchema(dae, schema.second);
				dae.setRoot(old);
			}
		}

		return result;
	}

	int DaeValidator::checkUniqueIds() const
	{
		return for_each_dae([&](const Dae & dae) {
			return checkUniqueIds(dae);
		});
	}

	int DaeValidator::checkUniqueIds(const Dae & dae) const
	{
		cout << "Checking unique ids..." << endl;

		int result = 0;
		map<string, size_t> ids;
		const auto & nodes = dae.root().selectNodes("//*[@id]");
		for (const auto & node : nodes)
		{
			string id = node.attribute("id").value();
			size_t line = node.line();

			int checkEscapeCharResult = CheckEscapeChar(id);
			if (checkEscapeCharResult != 0)
			{
				cout << dae.getURI() << ":" << line << ": \"" << id << "\" contains non-escaped characters." << endl;
				result |= checkEscapeCharResult;
			}

			auto it = ids.find(id);
			if (it != ids.end())
			{
				cout << dae.getURI() << ":" << line << ": Duplicated id \"" << id << "\". See first declaration at line " << it->second << "." << endl;
				result |= 1;
			}
			else
			{
				ids[id] = line;
			}
		}
		return result;
	}

	int DaeValidator::checkUniqueSids() const
	{
		return for_each_dae([&](const Dae & dae) {
			return checkUniqueSids(dae);
		});
	}

	int DaeValidator::checkUniqueSids(const Dae & dae) const
	{
		cout << "Checking unique sids..." << endl;

		int result = 0;
		const auto & parents = dae.root().selectNodes("//*[@sid]/..");
		for (auto parent : parents)
		{
			const auto & children = parent.selectNodes("/*[@sid]");
			map<string, size_t> sids;
			for (auto child : children)
			{
				string sid = child.attribute("sid").value();
				size_t line = child.line();

				auto it = sids.find(sid);
				if (it != sids.end())
				{
					cout << dae.getURI() << ":" << line << ": Duplicated sid \"" << sid << "\". See first declaration at line " << it->second << "." << endl;
					result |= 1;
				}
				else
				{
					sids[sid] = line;
				}
			}
		}
		return result;
	}

	int DaeValidator::checkReferencedJointController() const
	{
		return for_each_dae([&](const Dae & dae) {
			return checkReferencedJointController(dae);
		});
	}

	int DaeValidator::checkReferencedJointController(const Dae & dae) const
	{
		cout << "Checking controller joints..." << endl;

		const auto & sourceSkinNodes = dae.root().selectNodes("//collada:library_controllers/collada:controller/collada:skin/collada:source/collada:Name_array");
		const auto & visualSceneNodes = dae.root().selectNodes("//collada:node[@type='JOINT']");

		int result = 0;

		for (const auto& sourceSkinNode : sourceSkinNodes)
		{
			string NameArrayName = sourceSkinNode.attribute("id").value();
			string controllerID = NameArrayName.substr(0, NameArrayName.substr(0, NameArrayName.find_last_of('-')).find_last_of('-'));

			string skin = sourceSkinNode.text();
			vector<string> skinNodes = String::Split(skin);

			string visualNodeName;
			string skinNodeName;

			for (const auto& skinNode : skinNodes)
			{
				bool found = false;

				skinNodeName = skinNode;
				
				for (const auto& visualNode : visualSceneNodes)
				{
					visualNodeName = visualNode.attribute("sid").value();

					if (skinNode.compare(visualNodeName) == 0)
					{
						found = true;
						break;
					}
				}

				if (!found)
				{
					cout << "checkReferencedJointController -- Error: " << skinNodeName << " in " << controllerID << " controller is not referenced in the visual scene" << endl;
					result |= 1;
				}	
			}
		}

		return result;
	}

	int DaeValidator::checkSkeletonRoots() const
	{
		return for_each_dae([&](const Dae & dae) {
			return checkSkeletonRoots(dae);
		});
	}

	int DaeValidator::checkSkeletonRoots(const Dae & dae) const
	{
		cout << "Checking skeleton roots..." << endl;

		const auto & instanceControllers = dae.root().selectNodes("//collada:instance_controller");
		const auto & visualSceneNodes = dae.root().selectNodes("//collada:node[@type='JOINT']");

		int result = 0;

		for (const auto& instanceController : instanceControllers)
		{
			string controllerUrl = instanceController.attribute("url").value();
			size_t posController = controllerUrl.find_last_of('#');

			string researchSkeleton = "//collada:instance_controller[@url=" + string("'") + controllerUrl + "'" + "]//collada:skeleton";
			const auto & skeletonNodes = dae.root().selectNodes(researchSkeleton);

			for (const auto& skeletonNode : skeletonNodes)
			{
				string visualNodeId;
				string skeletonNodeName = skeletonNode.text();
				size_t posSkeleton = skeletonNodeName.find_last_of('#');

				bool found = false;

				for (const auto& visualNode : visualSceneNodes)
				{
					visualNodeId = visualNode.attribute("id").value();

					if (posSkeleton != string::npos)
						if (skeletonNodeName.substr(posSkeleton + 1).compare(visualNodeId) == 0)
						{
							found = true;
							break;
						}
				}

				if (!found)
				{
					cout << "checkSkeletonRoots -- Error: " << skeletonNodeName.substr(posSkeleton + 1) << " in " << controllerUrl.substr(posController + 1) << " instance controller is not referenced in the visual scene" << endl;
					result |= 1;
				}

			}
		}

		return result;
	}

	int DaeValidator::checkReferencedJointsBySkinController() const
	{
		return for_each_dae([&](const Dae & dae) {
			return checkReferencedJointsBySkinController(dae);
		});
	}

	int DaeValidator::checkReferencedJointsBySkinController(const Dae & dae) const
	{
		cout << "Checking controller skin joints..." << endl;

		const auto & instanceControllers = dae.root().selectNodes("//collada:instance_controller");
		const auto & controllers = dae.root().selectNodes("//collada:controller");

		string controllerUrl;

		int result = 0;

		for (const auto& instanceController : instanceControllers)
		{
			controllerUrl = instanceController.attribute("url").value();
			size_t posController = controllerUrl.find_last_of('#');

			for (const auto& controller : controllers)
			{
				if (controller.attribute("id").value().compare(controllerUrl.substr(posController + 1)) == 0)
				{
					string researchSkinNodes = "//collada:Name_array[@id=" + string("'") + controller.attribute("id").value() + "-joints-array" + "'" + "]";
					const auto & sourceSkinNodes = controller.selectNodes(researchSkinNodes);

					for (const auto& sourceSkinNode : sourceSkinNodes)
					{
						string skin = sourceSkinNode.text();
						vector<string> skinNodes = String::Split(skin);

						string visualNodeName;
						string skinNodeName;

						for (const auto& node : skinNodes)
						{
							string researchSkeleton = "//collada:instance_controller[@url=" + string("'") + controllerUrl + "'" + "]//collada:skeleton";
							const auto & skeletons = dae.root().selectNodes(researchSkeleton);

							for (const auto& skeleton : skeletons)
							{
								string skeletonName = skeleton.text();
								size_t posSkeleton = skeletonName.find_last_of('#');

								skeletonName = skeletonName.substr(posSkeleton + 1);
								string research2 = "//collada:node[@id=" + string("'") + skeletonName + "'" + "]";
								const auto & rootNodes = dae.root().selectNodes(research2);

								for (const auto& rootNode : rootNodes)
								{
									string rootNodeId = rootNode.attribute("id").value();
									string rootNodeName = rootNode.attribute("name").value();

									string research = "//collada:node[@id=" + string("'") + skeletonName + "'" + "]" + "//collada:node[@name=" + string("'") + node + "'" + "]";

									const auto & resultnodes = rootNode.selectNodes(research);

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
											result |= 1;
											cout << "checkReferencedJointsBySkinController -- Error: " + node + " is not accessible from " + skeletonName + " skeleton root in " + controllerUrl.substr(posController + 1) + " instance controller"<< endl;
										}
									}
								}
							}
						}
					}
				}
			}
		}

		return result;
	}

	int DaeValidator::checkSkeletonRootExistsToResolveController() const
	{
		return for_each_dae([&](const Dae & dae) {
			return checkSkeletonRootExistsToResolveController(dae);
		});
	}

	int DaeValidator::checkSkeletonRootExistsToResolveController(const Dae & dae) const
	{
		cout << "Checking skeleton root exists..." << endl;

		const auto & controllers = dae.root().selectNodes("//collada:controller");

		int result = 0;

		for (const auto& controller : controllers)
		{
			string controllerUrl = "#" + controller.attribute("id").value();

			string researchSkeleton = "//collada:instance_controller[@url=" + string("'") + controllerUrl + "'" + "]//collada:skeleton";
			const auto & skeletons = dae.root().selectNodes(researchSkeleton);

			if (!skeletons.size())
			{
				cout << "checkisSkeletonRootExistToResolveController -- Error: " << controller.attribute("id").value() << " controller has no skeleton to resolve it" << endl;
				result |= 1;
			}
		}

		return result;
	}

	/** Check if we have a complete bind pose. */
	int DaeValidator::checkCompleteBindPose() const
	{
		return for_each_dae([&](const Dae & dae) {
			return checkCompleteBindPose(dae);
		});
	}

	int DaeValidator::checkCompleteBindPose(const Dae & dae) const
	{
		cout << "Checking complete bind pose..." << endl;

		const auto & sourceSkinNodes = dae.root().selectNodes("//collada:library_controllers/collada:controller/collada:skin/collada:source/collada:Name_array");
		
		int result = 0;

		for (const auto& sourceSkinNode : sourceSkinNodes)
		{
			string skin = sourceSkinNode.text();
			string arrayId = sourceSkinNode.attribute("id").value();

			vector<string> skinNodes = String::Split(skin);
			
			for (const auto& skinNode : skinNodes)
			{
				bool found = false;

				// search parent's node
				string research = "//collada:node[@name=" + string("'") + skinNode + "'" + "]" + "/parent::collada:node";
				const auto & nodes1 = dae.root().selectNodes(research);
				for (const auto& node : nodes1)
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
						if (skinNode.compare(skinNode1))
						{
							research = "//collada:node[@name=" + string("'") + skinNode + "'" + "]" + "//collada:node[@name=" + string("'") + skinNode1 + "'" + "]";
							const auto & nodes2 = dae.root().selectNodes(research);

							if (nodes2.empty())
							{
								found1 = false;
								result |= 1;
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

		return result;
	}

	int DaeValidator::checkSkinController() const
	{
		return for_each_dae([&](const Dae & dae) {
			return checkSkinController(dae);
		});
	}


	int DaeValidator::checkSkinController(const Dae & dae) const
	{
		int result = 0;

		result |= DaeValidator::checkReferencedJointController(dae);
		result |= DaeValidator::checkSkeletonRoots(dae);
		result |= DaeValidator::checkReferencedJointsBySkinController(dae);
		result |= DaeValidator::checkSkeletonRootExistsToResolveController(dae);
		result |= DaeValidator::checkCompleteBindPose(dae);

		return result;
	}

	int DaeValidator::checkLOD() const
	{
		return for_each_dae([&](const Dae & dae) {
			return checkLOD(dae);
		});
	}

	static int recursiveSearchLOD(const Dae & dae, string LODUrl)
	{
		int result = 0;

		string research = "//collada:library_nodes//" + string("collada:node[@id=") + string("'") + LODUrl + "'" + "]";
		const auto & resultnodes = dae.root().selectNodes(research);

		if (!resultnodes.size())
		{
			cout << "checkLOD -- Error: " << LODUrl << " doesn't exist in library_nodes" << endl;
			result |= 1;
		}

		for (const auto& resultnode : resultnodes)
		{
			string nodeName = resultnode.attribute("name").value();
				
			// search for recursive LOD
			string researchproxy = "//collada:node[@id=" + string("'") + LODUrl + "'" + "]" + string("//lod:proxy");

			const auto & sourceLODs = dae.root().selectNodes(researchproxy);
			for (const auto& sourceLOD : sourceLODs)
			{
				string url = sourceLOD.attribute("url").value();
				size_t posLODUrln = url.find_last_of('#');
				url = url.substr(posLODUrln + 1);

				result |= recursiveSearchLOD(dae, url);
			}
				
			// search for instance node
			string researchInstanceNode = "//collada:node[@id=" + string("'") + LODUrl + "'" + "]" + string("//collada:instance_node");
			const auto & SourceInstanceNode = dae.root().selectNodes(researchInstanceNode);
			
			// search for instance_geometry
			string researchInstanceGeometry = "//collada:node[@id=" + string("'") + LODUrl + "'" + "]" + string("//collada:instance_geometry");
			const auto & SourceInstanceGeometry = dae.root().selectNodes(researchInstanceGeometry);

			if (!SourceInstanceGeometry.size() && !SourceInstanceNode.size())
			{
				result |= 1;
				cout << "checkLOD -- Error: No instance_geometry Or No instance_node in " << LODUrl << " node" << endl;
			}
		}

		return result;
	}

	int DaeValidator::checkLOD(const Dae & dae) const
	{
		int result = 0;

		const auto & sourceLOD = dae.root().selectNodes("//collada:library_visual_scenes//lod:proxy");
		for (const auto& source : sourceLOD)
		{
			string LODUrl = source.attribute("url").value();
			size_t posLODUrln = LODUrl.find_last_of('#');
			string LODUrl1 = LODUrl.substr(posLODUrln + 1);

			result |= recursiveSearchLOD(dae, LODUrl1);

			// search for instance node
			string research = "//lod:proxy[@url=" + string("'") + LODUrl + "'" + "]" +"/ancestor::collada:instance_node";
			
			const auto & resultInstanceNodes = dae.root().selectNodes(research);

			if (!resultInstanceNodes.size())
			{
				result |= 1;

				research = "//lod:proxy[@url=" + string("'") + LODUrl + "'" + "]" + "/ancestor::collada:node";
				const auto & resultNodes = dae.root().selectNodes(research);
				for (const auto& node : resultNodes)
				{
					string NodeId = node.attribute("id").value();
					cout << "checkLOD -- Error: No instance_node in " << NodeId << " node" << endl;
				}
			}
		}

		return result;
	}

	int DaeValidator::checkLinks() const
	{
		return for_each_dae([&](const Dae & dae) {
			return checkLinks(dae);
		});
	}

	int DaeValidator::checkLinks(const Dae & dae) const
	{
		cout << "Checking links..." << endl;

		const auto & ids = dae.getIds();

		int result = 0;
		for (const auto & t : dae.getAnyURIs())
		{
			const auto & line = get<0>(t);
			const auto & uri = get<1>(t);
			if (!Path::Exists(uri.nativePath()))
			{
				cout << dae.getURI() << ":" << line << ": Can't resolve " << uri << endl;
				result |= 1;
			}
			else if (!uri.fragment().empty())
			{
				Uri no_fragment_uri(uri);
				no_fragment_uri.setFragment("");
				if (no_fragment_uri == dae.getURI())
				{
					auto id = ids.find(uri.fragment());
					if (id == ids.end())
					{
						cout << dae.getURI() << ":" << line << ": Can't resolve #" << uri.fragment() << endl;
						result |= 1;
					}
				}
				else
				{
					auto it = dae.getExternalDAEs().find(no_fragment_uri);
					if (it != dae.getExternalDAEs().end())
					{
						auto ext_ids = it->second.getIds();
						auto id = ext_ids.find(uri.fragment());
						if (id == ext_ids.end())
						{
							cout << dae.getURI() << ":" << line << ": Can't resolve " << uri << endl;
							result |= 1;
						}
					}
					else
					{
						cout << dae.getURI() << ":" << line << ": " << uri << ": referenced file exists but has not been successfully loaded." << endl;
						result |= 1;
					}
				}
			}
		}

		// IDREF
		for (const auto & IDREF : dae.getIDREFs())
		{
			const auto & line = get<0>(IDREF);
			const auto & idref = get<1>(IDREF);

			auto id = ids.find(idref);
			if (id == ids.end())
			{
				cout << dae.getURI() << ":" << line << ": Can't resolve #" << idref << endl;
				result |= 1;
			}
		}

		return result;
	}

	int DaeValidator::ValidateAgainstSchema(const Dae & dae, const XmlSchema & schema)
	{
		return schema.validate(dae) ? 0 : 1;
	}

	int DaeValidator::CheckEscapeChar(const string & s)
	{
		if (s.find_first_of(" #$%&/:;<=>?@[\\:]^`{|}~") != string::npos)
			return 1;
		return 0;
	}
}
