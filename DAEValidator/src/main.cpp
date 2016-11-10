#include <iostream>

#include "ArgumentParser.h"
#include "DaeValidator.h"
#include "Dae.h"
#include "Log.h"
#include "PathUtil.h"
#include "StringUtil.h"

using namespace opencollada;
using namespace std;

namespace opencollada
{
	// Validation options
	const char* checkSchemaAuto = "--check-schema-auto";
	const char* checkSchema = "--check-schema";
	const char* checkUniqueIds = "--check-unique-ids";
	const char* checkReferencedJointController = "--check-joint-controller";
	const char* checkCompleteBindPose = "--check-complete-bindpose";
	const char* checkSkeletonRoots = "--check-skeleton-roots";
	const char* checkReferencedJointsBySkinController = "--check-referenced-joints-by-skin-controller";
	const char* checkisSkeletonRootExistToResolveController = "--check-is-skeleton-root-exist-to-resolve-controller";
	const char* checkLOD = "--check-LOD";

	const char* recursive = "--recursive";
	const char* quiet = "--quiet";

	const size_t flag_checkOption = 0x01;

	const char* colladaNamespace141 = "http://www.collada.org/2005/11/COLLADASchema";
	const char* colladaSchemaFileName141 = "collada_schema_1_4_1.xsd";
	XmlSchema colladaSchema141;

	const char* colladaNamespace15 = "http://www.collada.org/2008/03/COLLADASchema";
	const char* colladaSchemaFileName15 = "collada_schema_1_5.xsd";
	XmlSchema colladaSchema15;
}

int main(int argc, char* argv[])
{
	// Parse arguments
	ArgumentParser argparse(argc, argv);
	argparse.addArgument(); // dae or directory
	argparse.addArgument(checkSchemaAuto).flags(flag_checkOption);
	argparse.addArgument(checkSchema).numParameters(1).flags(flag_checkOption);
	argparse.addArgument(checkUniqueIds).flags(flag_checkOption);
	argparse.addArgument(checkReferencedJointController).flags(flag_checkOption);
	argparse.addArgument(checkCompleteBindPose).flags(flag_checkOption);
	argparse.addArgument(checkSkeletonRoots).flags(flag_checkOption);
	argparse.addArgument(checkReferencedJointsBySkinController).flags(flag_checkOption);
	argparse.addArgument(checkisSkeletonRootExistToResolveController).flags(flag_checkOption);
	argparse.addArgument(checkLOD).flags(flag_checkOption);
	argparse.addArgument(recursive);
	argparse.addArgument(quiet);

	if (!argparse.parseArguments())
		return 1;

	bool muted = argparse.findArgument(quiet);
	Log::Setup(muted);

	// Pre-load COLLADA schemas	
	colladaSchema141.readFile(Path::Join(Path::GetExecutableDirectory(), colladaSchemaFileName141));
	if (!colladaSchema141)
	{
		cerr << "Error loading " << Path::Join(Path::GetExecutableDirectory(), colladaSchemaFileName141) << endl;
		return 1;
	}

	// Disabled for the moment. COLLADA schema 1.5 generates errors.
	//colladaSchema15.readFile(Path::Join(GetExecutableDirectory(), colladaSchemaFileName15));
	//if (!colladaSchema15)
	//{
	//	cerr << "Error loading " << Path::Join(GetExecutableDirectory(), colladaSchemaFileName15) << endl;
	//	return 1;
	//}

	string path = argparse.findArgument(0).getValue<string>();

	list<string> daePaths;
	if (Path::IsDirectory(path))
	{
		daePaths = Path::ListDaes(path, argparse.findArgument(recursive));
	}
	else
	{
		daePaths.push_back(path);
	}

	if (daePaths.size() == 0)
	{
		cout << "No DAE found." << endl;
		return 0;
	}

	DaeValidator validator(daePaths);
	int result = 0;

	if (!argparse.hasSetArgument(flag_checkOption))
	{
		result |= validator.checkAll();
	}
	else
	{
		if (argparse.findArgument(checkSchemaAuto))
		{
			result |= validator.checkSchema();
		}

		if (argparse.findArgument(checkUniqueIds))
		{
			result |= validator.checkUniqueIds();
		}

		if (argparse.findArgument(checkReferencedJointController))
		{
			result |= validator.checkReferencedJointController();
		}

		if (argparse.findArgument(checkSkeletonRoots))
		{
			result |= validator.checkSkeletonRoots();
		}

		if (argparse.findArgument(checkReferencedJointsBySkinController))
		{
			result |= validator.checkReferencedJointsBySkinController();
		}

		if (argparse.findArgument(checkCompleteBindPose))
		{
			result |= validator.checkCompleteBindPose();
		}

		if (argparse.findArgument(checkisSkeletonRootExistToResolveController))
		{
			result |= validator.checkisSkeletonRootExistToResolveController();
		}


		if (argparse.findArgument(checkLOD))
		{
			result |= validator.checkLOD();
		}

		if (const auto & arg = argparse.findArgument(checkSchema))
		{
			result |= validator.checkSchema(arg.getValue<string>());
		}
	}

	if (result == 0)
		cout << "Validation SUCCEEDED." << endl;
	else
		cerr << "Validation FAILED." << endl;

	return result;
}