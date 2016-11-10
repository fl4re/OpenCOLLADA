#include "DaeValidator.h"
#include "COLLADASWConstants.h"
#include "PathUtil.h"
#include "StringUtil.h"
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
	extern const char* colladaNamespace141;
	extern const char* colladaSchemaFileName141;
	extern XmlSchema colladaSchema141;

	extern const char* colladaNamespace15;
	extern const char* colladaSchemaFileName15;
	extern XmlSchema colladaSchema15;

	DaeValidator::DaeValidator(const list<string> & daePaths)
	{
		mDaePaths.reserve(daePaths.size());
		mDaePaths.insert(mDaePaths.end(), daePaths.begin(), daePaths.end());
	}

	int DaeValidator::for_each_dae(const function<int(const Dae &)> & task) const
	{
		int result = 0;
		for (const auto & daePath : mDaePaths)
		{
			Dae dae;
			dae.readFile(daePath);
			if (dae)
			{
				result |= task(dae);
			}
			else
			{
				result |= 1;
				cerr << "Error loading " << daePath << endl;
			}
		}
		return result;
	}

	int DaeValidator::checkAll() const
	{
		return for_each_dae([&](const Dae & dae) {
			return CheckAll(dae);
		});
	}

	int DaeValidator::CheckAll(const Dae & dae)
	{
		return
			CheckSchema(dae) |
			CheckUniqueIds(dae) |
			CheckLOD(dae);
	}

	int DaeValidator::checkSchema(const string & schema_uri) const
	{
		if (schema_uri.empty())
		{
			return for_each_dae([&](const Dae & dae) {
				return CheckSchema(dae);
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

		return 1;
	}

	int DaeValidator::CheckSchema(const Dae & dae)
	{
		int result = 0;

		// Get root <COLLADA> element
		auto collada = dae.root();
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
			result |= ValidateAgainstSchema(dae, colladaSchema141);
		}
		else if (href == colladaNamespace15)
		{
			result |= ValidateAgainstSchema(dae, colladaSchema15);
		}
		else
		{
			cerr << "Can't determine COLLADA version used by input file" << endl;
			return 1;
		}

		set<string> xsdURLs;

		// Find xsi:schemaLocation attributes in dae and try to validate against specified xsd documents
		auto elements = dae.root().selectNodes("//*[@xsi:schemaLocation]");
		for (const auto & element : elements)
		{
			if (auto schemaLocation = element.attribute("schemaLocation"))
			{
				vector<string> parts = String::Split(schemaLocation.value());
				// Parse pairs of namespace/xsd and take second element
				for (size_t i = 1; i < parts.size(); i += 2)
				{
					xsdURLs.insert(parts[i]);
				}
			}
		}

		for (const auto & URL : xsdURLs)
		{
			int tmpResult = ValidateAgainstFile(dae, URL);
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

		return result;
	}

	ostream & operator << (ostream & o, const COLLADABU::URI & uri)
	{
		o << uri.getURIString();
		return o;
	}

	int DaeValidator::checkUniqueIds() const
	{
		return for_each_dae([&](const Dae & dae) {
			return CheckUniqueIds(dae);
		});
	}

	int DaeValidator::CheckUniqueIds(const Dae & dae)
	{
		int result = 0;
		XmlNodeSet nodes = dae.root().selectNodes("//*[@id]");
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
				cerr << dae.getURI() << ":" << node.line() << ": Duplicated id \"" << id_line.getId() << "\". See first declaration at line " << it->getLine() << "." << endl;
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
		return for_each_dae([&](const Dae & dae) {
			return CheckReferencedJointController(dae);
		});
	}

	int DaeValidator::CheckReferencedJointController(const Dae & dae)
	{
		XmlNodeSet sourceSkinNodes = dae.root().selectNodes("//collada:library_controllers/collada:controller/collada:skin/collada:source/collada:Name_array");
		XmlNodeSet visualSceneNodes = dae.root().selectNodes("//collada:node[@type='JOINT']");

		int result = 0;

		for (const auto& sourceSkinNode : sourceSkinNodes)
		{
			string NameArrayName = sourceSkinNode.attribute("id").value();
			string controllerID = NameArrayName.substr(0, NameArrayName.substr(0, NameArrayName.find_last_of('-')).find_last_of('-'));

			string skin = sourceSkinNode.text();
			std::vector<string> skinNodes = String::Split(skin);

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
					cerr << "checkReferencedJointController -- Error: " << skinNodeName << "in" << controllerID << " controller is not referenced in the visual scene" << endl;
					result |= 1;
				}	
			}
		}

		return result;
	}

	int DaeValidator::checkSkeletonRoots() const
	{
		return for_each_dae([&](const Dae & dae) {
			return CheckSkeletonRoots(dae);
		});
	}

	int DaeValidator::CheckSkeletonRoots(const Dae & dae)
	{
		XmlNodeSet instanceControllers = dae.root().selectNodes("//collada:instance_controller");
		XmlNodeSet visualSceneNodes = dae.root().selectNodes("//collada:node[@type='JOINT']");

		int result = 0;

		for (const auto& instanceController : instanceControllers)
		{
			string controllerUrl = instanceController.attribute("url").value();
			size_t posController = controllerUrl.find_last_of('#');

			string researchSkeleton = "//collada:instance_controller[@url=" + string("'") + controllerUrl + "'" + "]//collada:skeleton";
			XmlNodeSet skeletonNodes = dae.root().selectNodes(researchSkeleton);

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
					cerr << "checkSkeletonRoots -- Error: " << skeletonNodeName.substr(posSkeleton + 1) << " in " << controllerUrl.substr(posController + 1) << " instance controller is not referenced in the visual scene" << endl;
					result |= 1;
				}

			}
		}

		return result;
	}

	int DaeValidator::checkReferencedJointsBySkinController() const
	{
		return for_each_dae([&](const Dae & dae) {
			return CheckReferencedJointsBySkinController(dae);
		});
	}

	int DaeValidator::CheckReferencedJointsBySkinController(const Dae & dae)
	{
		XmlNodeSet instanceControllers = dae.root().selectNodes("//collada:instance_controller");
		XmlNodeSet controllers = dae.root().selectNodes("//collada:controller");

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
					XmlNodeSet sourceSkinNodes = controller.selectNodes(researchSkinNodes);

					for (const auto& sourceSkinNode : sourceSkinNodes)
					{
						string skin = sourceSkinNode.text();
						std::vector<string> skinNodes = String::Split(skin);

						string visualNodeName;
						string skinNodeName;

						for (const auto& node : skinNodes)
						{
							string researchSkeleton = "//collada:instance_controller[@url=" + string("'") + controllerUrl + "'" + "]//collada:skeleton";
							XmlNodeSet skeletons = dae.root().selectNodes(researchSkeleton);

							for (const auto& skeleton : skeletons)
							{
								string skeletonName = skeleton.text();
								size_t posSkeleton = skeletonName.find_last_of('#');

								skeletonName = skeletonName.substr(posSkeleton + 1);
								string research2 = "//collada:node[@id=" + string("'") + skeletonName + "'" + "]";
								XmlNodeSet rootNodes = dae.root().selectNodes(research2);

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
											result |= 1;
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

		return result;
	}

	int DaeValidator::checkisSkeletonRootExistToResolveController() const
	{
		return for_each_dae([&](const Dae & dae) {
			return CheckisSkeletonRootExistToResolveController(dae);
		});
	}

	int DaeValidator::CheckisSkeletonRootExistToResolveController(const Dae & dae)
	{
		XmlNodeSet controllers = dae.root().selectNodes("//collada:controller");

		int result = 0;

		for (const auto& controller : controllers)
		{
			string controllerUrl = "#" + controller.attribute("id").value();

			string researchSkeleton = "//collada:instance_controller[@url=" + string("'") + controllerUrl + "'" + "]//collada:skeleton";
			XmlNodeSet skeletons = dae.root().selectNodes(researchSkeleton);

			if (!skeletons.size())
			{
				cerr << "checkisSkeletonRootExistToResolveController -- Error: " << controller.attribute("id").value() << " controller has no skeleton to resolve it" << endl;
				result |= 1;
			}
		}

		return result;
	}

	/** Check if we have a complete bind pose. */
	int DaeValidator::checkCompleteBindPose() const
	{
		return for_each_dae([&](const Dae & dae) {
			return CheckCompleteBindPose(dae);
		});
	}

	int DaeValidator::CheckCompleteBindPose(const Dae & dae)
	{
		XmlNodeSet sourceSkinNodes = dae.root().selectNodes("//collada:library_controllers/collada:controller/collada:skin/collada:source/collada:Name_array");
		
		int result = 0;

		for (const auto& sourceSkinNode : sourceSkinNodes)
		{
			string skin = sourceSkinNode.text();
			string arrayId = sourceSkinNode.attribute("id").value();

			std::vector<string> skinNodes = String::Split(skin);
			
			for (const auto& skinNode : skinNodes)
			{
				bool found = false;

				// search parent's node
				string research = "//collada:node[@name=" + string("'") + skinNode + "'" + "]" + "/parent::collada:node";
				XmlNodeSet resultnodes = dae.root().selectNodes(research);
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
							XmlNodeSet resultnodes = dae.root().selectNodes(research);

							if (!(resultnodes.size() > 0))
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
						cerr << "checkCompleteBindPose -- Error in " << arrayId << " controller, " << skinNode << " has no parent defined" << endl;
					}
						
				}
			}
		}

		return result;
	}


	int DaeValidator::checkLOD() const
	{
		return for_each_dae([&](const Dae & dae) {
			return CheckLOD(dae);
		});
	}

	static int recursiveSearchLOD(const Dae & dae, string LODUrl)
	{
		int result = 0;

		string research = "//collada:library_nodes//" + string("collada:node[@id=") + string("'") + LODUrl + "'" + "]";
		XmlNodeSet resultnodes = dae.root().selectNodes(research);

		if (!resultnodes.size())
		{
			cerr << "checkLOD -- Error: " << LODUrl << " doesn't exist in library_nodes" << endl;
			result |= 1;
		}

		for (const auto& resultnode : resultnodes)
		{
			string nodeName = resultnode.attribute("name").value();
				
			// search for recursive lod
			string researchproxy = "//collada:node[@id=" + string("'") + LODUrl + "'" + "]" + string("//lod:proxy");

			XmlNodeSet sourceLODs = dae.root().selectNodes(researchproxy);
			for (const auto& sourceLOD : sourceLODs)
			{
				string LODUrl = sourceLOD.attribute("url").value();
				size_t posLODUrln = LODUrl.find_last_of('#');
				LODUrl = LODUrl.substr(posLODUrln + 1);

				result |= recursiveSearchLOD(dae, LODUrl);
			}
				
			// search for instance node
			string researchInstanceNode = "//collada:node[@id=" + string("'") + LODUrl + "'" + "]" + string("//collada:instance_node");
			XmlNodeSet SourceInstanceNode = dae.root().selectNodes(researchInstanceNode);

			// search for instance_node
			/*
			for (const auto& resultInstanceNode : SourceInstanceNode)
			{
				string InstanceNodeUrl = resultInstanceNode.attribute("url").value();

				// search for node referenced by instance node
				InstanceNodeUrl = InstanceNodeUrl.substr(1);
				string researchNode = "//collada:library_nodes//" + string("collada:node[@id=") + string("'") + InstanceNodeUrl + "'" + "]";
				XmlNodeSet SourceNodes = dae.root().selectNodes(researchNode);
				
				if (!SourceNodes.size())
				{
					result |= 1;
					cerr << "checkLOD -- Error: " << InstanceNodeUrl << " doesn't exist in library_nodes" << endl;
				}
					

				for (const auto& result : SourceNodes)
				{
					string nodeName = result.attribute("name").value();
				}
			}
			*/

			// search for instance_geometry
			string researchInstanceGeometry = "//collada:node[@id=" + string("'") + LODUrl + "'" + "]" + string("//collada:instance_geometry");
			XmlNodeSet SourceInstanceGeometry = dae.root().selectNodes(researchInstanceGeometry);

			if (!SourceInstanceGeometry.size() && !SourceInstanceNode.size())
			{
				result |= 1;
				cerr << "checkLOD -- Error: No instance_geometry Or No instance_node in " << LODUrl << " node" << endl;
			}

			/*
			for (const auto& resultInstanceGeometry : SourceInstanceGeometry)
			{
				string InstanceGeometryUrl = resultInstanceGeometry.attribute("url").value();

				// search for node referenced by instance node
				InstanceGeometryUrl = InstanceGeometryUrl.substr(1);
				string researchGeometry = "//collada:library_geometries//" + string("collada:geometry[@id=") + string("'") + InstanceGeometryUrl + "'" + "]";
				XmlNodeSet SourceGeometries = dae.root().selectNodes(researchGeometry);

				if (!SourceGeometries.size())
				{
					result |= 1;
					cerr << "checkLOD -- Error: " << InstanceGeometryUrl << " doesn't exist in library_geometry" << endl;
				}


				for (const auto& result : SourceGeometries)
				{
					string nodeName = result.attribute("name").value();
				}
			}
			*/


		}

		return result;
	}

	int DaeValidator::CheckLOD(const Dae & dae)
	{
		int result = 0;

		XmlNodeSet sourceLOD = dae.root().selectNodes("//collada:library_visual_scenes//lod:proxy");
		for (const auto& source : sourceLOD)
		{
			string LODUrl = source.attribute("url").value();
			size_t posLODUrln = LODUrl.find_last_of('#');
			string LODUrl1 = LODUrl.substr(posLODUrln + 1);

			result |= recursiveSearchLOD(dae, LODUrl1);

			// search for 1st instance node
			string research = "//lod:proxy[@url=" + string("'") + LODUrl + "'" + "]" +"/ancestor::collada:instance_node";
			
			XmlNodeSet resultInstanceNodes = dae.root().selectNodes(research);

			if (!resultInstanceNodes.size())
			{
				result |= 1;

				string research = "//lod:proxy[@url=" + string("'") + LODUrl + "'" + "]" + "/ancestor::collada:node";
				XmlNodeSet resultNodes = dae.root().selectNodes(research);
				for (const auto& source : resultNodes)
				{
					string NodeId = source.attribute("id").value();
					cerr << "checkLOD -- Error: No instance_node in " << NodeId << " node" << endl;
				}
			}

			/*
			for (const auto& instanceNode : resultInstanceNodes)
			{
				string instanceNodeUrl = instanceNode.attribute("url").value();
				
				instanceNodeUrl = instanceNodeUrl.substr(1);
				string researchNode = "//collada:library_nodes//" + string("collada:node[@id=") + string("'") + instanceNodeUrl + "'" + "]";
				XmlNodeSet SourceNodes = dae.root().selectNodes(researchNode);

				if (!SourceNodes.size())
				{
					cerr << "checkLOD -- Error: " << instanceNodeUrl << " doesn't exist in library_nodes" << endl;
					result |= 1;
				}
					

				for (const auto& result : SourceNodes)
				{
					string nodeName = result.attribute("name").value();
				}
			}
			*/
		}

		return result;
	}


	int DaeValidator::ValidateAgainstFile(const Dae & dae, const string & xsdPath)
	{
		// Open xsd
		XmlSchema xsd;
		xsd.readFile(xsdPath.c_str());
		if (!xsd)
		{
			return 2;
		}
		return ValidateAgainstSchema(dae, xsd);
	}

	int DaeValidator::ValidateAgainstSchema(const Dae & dae, const XmlSchema & schema)
	{
		return schema.validate(dae) ? 0 : 1;
	}
}