#include <chrono>
#include "no_warning_iostream"

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
	const char* checkUniqueSids = "--check-unique-sids";
	const char* checkLinks = "--check-links";
	const char* checkReferencedJointController = "--check-joint-controller";
	const char* checkCompleteBindPose = "--check-complete-bindpose";
	const char* checkSkeletonRoots = "--check-skeleton-roots";
	const char* checkReferencedJointsBySkinController = "--check-referenced-joints-by-skin-controller";
	const char* checkSkeletonRootExistsToResolveController = "--check-skeleton-root-exists-to-resolve-controller";
	const char* checkLOD = "--check-lod";
	const char* recursive = "--recursive";
	const char* quiet = "--quiet";
	const char* help = "--help";

	const size_t flag_checkOption = 0x01;

	const char* colladaNamespace141 = "http://www.collada.org/2005/11/COLLADASchema";
	const char* colladaSchemaFileName141 = "collada_schema_1_4_1.xsd";
	XmlSchema colladaSchema141;

	const char* colladaNamespace15 = "http://www.collada.org/2008/03/COLLADASchema";
	const char* colladaSchemaFileName15 = "collada_schema_1_5.xsd";
	//XmlSchema colladaSchema15;
}

int main(int argc, char* argv[])
{
	auto start = chrono::high_resolution_clock::now();

	// Parse arguments
	ArgumentParser argparse(argc, argv);

	argparse.addArgument().hint("path").help("Path to COLLADA document or directory to parse. If 'path' is a directory it is parsed for files with .DAE extension.");
	argparse.addArgument(checkSchemaAuto).flags(flag_checkOption).help("Regular XML schema validation.");
	argparse.addArgument(checkSchema).flags(flag_checkOption).numParameters(1).hint(0, "schema_path").help("Validate against arbitrary XML schema.");
	argparse.addArgument(checkUniqueIds).flags(flag_checkOption).help("Check that ids in documents are unique.");
	argparse.addArgument(checkUniqueSids).flags(flag_checkOption).help("Check that sids in documents are unique in their scope.");
	argparse.addArgument(checkLinks).flags(flag_checkOption).help("Check that URIs refer to valid files and/or elements.");
	argparse.addArgument(checkReferencedJointController).flags(flag_checkOption).help("Check if all joints in the Name_array of each skin controller are referenced in the DAE.");
	argparse.addArgument(checkCompleteBindPose).flags(flag_checkOption).help("Check if we have a complete bind pose.");
	argparse.addArgument(checkSkeletonRoots).flags(flag_checkOption).help("Check if all skeleton roots in the instance controller are referenced in the DAE.");
	argparse.addArgument(checkReferencedJointsBySkinController).flags(flag_checkOption).help("Check if all joints referenced by the skin controller are accessible via the defined skeleton roots.");
	argparse.addArgument(checkSkeletonRootExistsToResolveController).flags(flag_checkOption).help("Check if there is at least one skeleton root to resolve the controller.");
	argparse.addArgument(checkLOD).flags(flag_checkOption).help("Check LOD.");
	argparse.addArgument(recursive).help("Recursively parse directories. Ignored if 'path' is not a directory.");
	argparse.addArgument(quiet).help("If set, no output is sent to standard out/err.");
	argparse.addArgument(help).help("Display help.");

	bool argparse_ok = argparse.parseArguments();

	if (argparse.findArgument(help)) {
		cout << argparse.usage() << endl;
		return 0;
	}

	if (!argparse_ok) {
		cerr << argparse.getParseError() << endl;
		cout << argparse.usage() << endl;
		return 1;
	}

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

	string path = Path::GetAbsolutePath(argparse.findArgument(0).getValue<string>());

	list<string> daePaths;
	if (Path::IsDirectory(path))
	{
		cout << "Listing COLLADA files..." << endl;
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
			result |= validator.checkSchema();

		if (argparse.findArgument(checkUniqueIds))
			result |= validator.checkUniqueIds();

		if (argparse.findArgument(checkUniqueSids))
			result |= validator.checkUniqueSids();

		if (argparse.findArgument(checkLinks))
			result |= validator.checkLinks();

		if (argparse.findArgument(checkReferencedJointController))
			result |= validator.checkReferencedJointController();

		if (argparse.findArgument(checkSkeletonRoots))
			result |= validator.checkSkeletonRoots();

		if (argparse.findArgument(checkReferencedJointsBySkinController))
			result |= validator.checkReferencedJointsBySkinController();

		if (argparse.findArgument(checkCompleteBindPose))
			result |= validator.checkCompleteBindPose();

		if (argparse.findArgument(checkSkeletonRootExistsToResolveController))
			result |= validator.checkSkeletonRootExistsToResolveController();

		if (argparse.findArgument(checkLOD))
			result |= validator.checkLOD();

		if (const auto & arg = argparse.findArgument(checkSchema))
			result |= validator.checkSchema(arg.getValue<string>());
	}

	auto end = chrono::high_resolution_clock::now();
	auto duration_ms = chrono::duration_cast<chrono::milliseconds>(end - start);

	cout << "Processed " << daePaths.size() << " documents in " << duration_ms.count() / 1000.0 << "s" << endl;

	if (result == 0)
		cout << "Validation SUCCEEDED." << endl;
	else
		cerr << "Validation FAILED." << endl;

	return result;
}