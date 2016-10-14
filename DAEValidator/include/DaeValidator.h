#pragma once

#include "Dae.h"

#include <string>

namespace opencollada
{
	class DaeValidator
	{
	public:
		DaeValidator(const Dae & dae);

		int checkAll() const;
		int checkSchema(const std::string & schema_uri = std::string()) const;
		int checkUniqueIds() const;
		int checkReferencedJointController() const;
		int checkSkeletonRoots() const;
		int checkReferencedJointsBySkinController() const ;

	private:
		int validateAgainstMemory(const char* xsd, size_t size) const;
		int validateAgainstFile(const std::string & xsdPath) const;

	private:
		const Dae & mDae;
	};
}