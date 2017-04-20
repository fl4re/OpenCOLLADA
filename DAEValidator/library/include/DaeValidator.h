#pragma once

#include "Dae.h"

#include "no_warning_functional"
#include "no_warning_list"
#include "no_warning_map"
#include "no_warning_string"

namespace opencollada
{
	class DaeValidator
	{
	public:
		DaeValidator(const std::list<std::string> & daePaths);

		int checkAll() const;
		int checkSchema(const std::string & schema_uri = std::string()) const;
		int checkUniqueIds() const;
		int checkUniqueSids() const;
		int checkLinks() const;

	private:
		int checkSchema(const Dae & dae) const;
		int checkAll(const Dae & dae) const;
		int checkUniqueIds(const Dae & dae) const;
		int checkUniqueSids(const Dae & dae) const;
		int checkLinks(const Dae & dae) const;

		int for_each_dae(const std::function<int(const Dae &)> & task) const;

		static int ValidateAgainstSchema(const Dae & dae, const XmlSchema & schema);
		static int CheckEscapeChar(const std::string & s);

	private:
		std::vector<std::string> mDaePaths;
		// namespace to XmlSchema
		mutable std::map<std::string, XmlSchema> mSchemas;
		// namespace to xsd location
		mutable std::map<std::string, std::string> mSchemaLocations;

		static string colladaNamespace141 = "http://www.collada.org/2005/11/COLLADASchema";
		const char* colladaSchemaFileName141 = "collada_schema_1_4_1.xsd";
		XmlSchema colladaSchema141;

		const char* colladaNamespace15 = "http://www.collada.org/2008/03/COLLADASchema";
		const char* colladaSchemaFileName15 = "collada_schema_1_5.xsd";
		//XmlSchema colladaSchema15;
	};
}