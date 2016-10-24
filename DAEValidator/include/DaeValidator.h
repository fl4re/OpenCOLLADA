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

		/** Check if all joints in the Name_array of each skin controller are referenced in the DAE. */
		int checkReferencedJointController() const;

		/** Check if all skeleton roots in the instance controller are referenced in the DAE. */
		int checkSkeletonRoots() const;

		/** Check if all joints referenced by the skin controller are accessible via the defined skeleton roots. */
		int checkReferencedJointsBySkinController() const;
		
		/** Check if there is at least one skeleton root to resolve the controller. */
		int checkisSkeletonRootExistToResolveController() const;

		/** Check if we have a complete bind pose. */
		int checkCompleteBindPose() const;
		

	private:
		int validateAgainstMemory(const char* xsd, size_t size) const;
		int validateAgainstFile(const std::string & xsdPath) const;

	private:
		const Dae & mDae;
	};
}