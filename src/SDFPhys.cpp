#include "SDFPhys.h"

SDFPhys::PhysManager::PhysManager(SDFPhys::SDFPhysManagerCreateInfo createInfo)
{
	activeEntitiesShapeID = std::vector<uint32_t>(createInfo.entityMax);
	activeEntitiesParameterData = std::vector<double*>(createInfo.entityMax);

	endCap = 0;
}



/// <summary>
/// Adds a physics entity to be simulated. Ensure that
/// the parameter data matches the expected size of the
/// shape type. To get parameter expectation(s), call 
/// GetShapeParameters 
/// </summary>
void SDFPhys::PhysManager::CreatePhysEntity(SDFPhys::PhysEntityCreateInfo createInfo)
{

}