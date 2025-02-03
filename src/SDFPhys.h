#pragma once

#include <cstdint>
#include <vector>

#include <string> 

namespace SDFPhys
{
    // 3D SDF primitive shapes  
    #define SPHERE 0
    #define BOX_EXACT 1
    #define BOX_FRAME_EXACT 2
    #define TORUS_EXACT 3
    #define TORUS_CAPPED_EXACT 4
    #define LINK_EXACT 5
    #define CYLINDER_EXACT 6
    #define CONE_EXACT 7
    #define CONE_BOUND 8
    #define CONE_INFINETE 9
    #define PLANE_EXACT 10
    #define PRISM_HEXAGONAL 11
    #define PRISM_TRIANGULAR 12
    #define CAPSULE_EXACT 13
    #define CAPSULE_VERTICAL_EXACT 14
    #define CYLINDER_VERTICAL_CAPPED_EXACT 15
    #define CYLINDER_ARBITRARY_CAPPED_EXACT 16
    #define CYLINDER_ROUNDED_EXACT 17
    #define CONE_CAPPED_EXACT 18
    #define CONE_CAPPED_EXACT_ALT 19
    #define ANGLE_SOLID_EXACT 20
    #define SPHERE_CUT_EXACT 21
    #define SPHERE_CUT_HOLLOW_EXACT 22
    #define CONE_ROUND_EXACT 23
    #define CONE_ROUND_EXACT_ALT 24 
    #define ELLIPSOID 25 
    #define VESICA_REVOLVED_EXACT 26
    #define RHOMBUS_EXACT 27
    #define OCTAHEDRON_EXACT 28
    #define OCTAHEDRON 29
    #define PYRAMID_EXACT 30
    #define TRIANGLE_EXACT 31
    #define QUAD_EXACT 32



	struct SDFPhysManagerCreateInfo
	{
		uint32_t entityMax; 
	};

	struct PhysEntityCreateInfo
	{
		uint32_t shapeID;
		double parameterData[];

		// TODO: Layers 
	};


	// Note: The creation of new entities should be done by a sort of 
	//		 flag system where every object that wants to add to the 
	//		 phys manager adds to a set array and all their commands
	//		 are sent to the manager at the end of a frame or when it 
	//		 is finished with previous work 


	class PhysManager
	{
	public:
		PhysManager(SDFPhys::SDFPhysManagerCreateInfo createInfo);
		~PhysManager();


		void CreatePhysEntity(SDFPhys::PhysEntityCreateInfo physCreateInfo);
		bool CheckCollision(uint32_t targetID, uint32_t sourceID, float tolerance);

	private: 

		// Note: Current vectors are to be replaced with PLF Colony

		uint32_t endCap; 
		std::vector<uint32_t> activeEntitiesShapeID; 
		std::vector<double*> activeEntitiesParameterData;


	};



	inline std::string Get3DShapeParameters(uint32_t shapeID)
	{
        switch (shapeID) {
        case SPHERE:
            break;
        case BOX_EXACT:
            break;
        case BOX_FRAME_EXACT:
            break;
        case TORUS_EXACT:
            break;
        case TORUS_CAPPED_EXACT:
            break;
        case LINK_EXACT:
            break;
        case CYLINDER_EXACT:
            break;
        case CONE_EXACT:
            break;
        case CONE_BOUND:
            break;
        case CONE_INFINETE:
            break;
        case PLANE_EXACT:
            break;
        case PRISM_HEXAGONAL:
            break;
        case PRISM_TRIANGULAR:
            break;
        case CAPSULE_EXACT:
            break;
        case CAPSULE_VERTICAL_EXACT:
            break;
        case CYLINDER_VERTICAL_CAPPED_EXACT:
            break;
        case CYLINDER_ARBITRARY_CAPPED_EXACT:
            break;
        case CYLINDER_ROUNDED_EXACT:
            break;
        case CONE_CAPPED_EXACT:
            break;
        case CONE_CAPPED_EXACT_ALT:
            break;
        case ANGLE_SOLID_EXACT:
            break;
        case SPHERE_CUT_EXACT:
            break;
        case SPHERE_CUT_HOLLOW_EXACT:
            break;
        case CONE_ROUND_EXACT:
            break;
        case CONE_ROUND_EXACT_ALT:
            break;
        case ELLIPSOID:
            break;
        case VESICA_REVOLVED_EXACT:
            break;
        case RHOMBUS_EXACT:
            break;
        case OCTAHEDRON_EXACT:
            break;
        case OCTAHEDRON:
            break;
        case PYRAMID_EXACT:
            break;
        case TRIANGLE_EXACT:
            break;
        case QUAD_EXACT:
            break;
        default:
            // Handle unknown shape
            break;
        };



	

}