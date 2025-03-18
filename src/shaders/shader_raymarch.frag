#version 450

layout(location = 0) in vec4 fragColor;

layout(location = 0) out vec4 outColor;



// Example data 

struct Shape
{
	uvec4 shapeInfo;
	//uint shapeType;
	//uint startP;

};

// Get what equation should be used for this shape 
uint GetShapeType(Shape shape)
{
	return shape.shapeInfo.x;
}

// Offset into the parameter array for start 
uint GetShapeOffset(Shape shape)
{
	return shape.shapeInfo.y;
}

layout(std140, binding = 2) readonly buffer storageBufferShapes
{
	Shape shapes[];
} ShapeData;

layout(std140, binding = 3) readonly buffer storageBufferParams
{
	vec4 params[];
} ShapeParamData;

// TODO: Add uniform buffer for lighting 


// SDF functions 
// From: https://iquilezles.org/articles/distfunctions/
float Sphere(vec3 p, vec3 center, float radius);
float Box(vec3 p, vec3 center, vec3 size);
float RoundBox(vec3 p, vec3 center, vec3 size, float rounding);
float BoxFrame(vec3 p, vec3 center, vec3 size, float thickness);
float SampleSDF(vec3 pos, uint type, uint startP);

float MapScene(vec3 pos);
float MapScene(vec3 pos, out uint shape);
vec3 CalcNormal(vec3 p);

#define SPHERE 0
#define BOX 1
#define ROUND_BOX 2
#define FRAME_BOX 3

/// Calculate the normal by taking the central differences on the distance field.
vec3 CalcNormal(vec3 p)
{
    vec2 e = vec2(1.0, -1.0) * 0.0005;
    return normalize(
        e.xyy * MapScene(p + e.xyy) +
        e.yyx * MapScene(p + e.yyx) +
        e.yxy * MapScene(p + e.yxy) +
        e.xxx * MapScene(p + e.xxx));
}

float MapScene(vec3 pos)
{
	float threshold = 0.001f; 
	float internalMap = 99999999.9f;

	// Brute force scene check 
	for(int i = 0; i < ShapeData.shapes.length(); i++)
	{
		internalMap = min(SampleSDF(
						pos, 
						// Type 
						ShapeData.shapes[i].shapeInfo.x, 
						// Parameter offset 
						ShapeData.shapes[i].shapeInfo.y), 
					internalMap);
	}

	return internalMap;
}

float MapScene(vec3 pos, out uint id)
{
	float threshold = 0.001f; 
	float internalMap = 99999999.9f;
	id = -1; 

	// Brute force scene check 
	for(int i = 0; i < ShapeData.shapes.length(); i++)
	{
		// Check for new better shape 
		
		uint currType = ShapeData.shapes[i].shapeInfo.x;

		float currMap = SampleSDF(
						pos, 
						// Type of shape 
						currType, 
						// Parameter offset 
						ShapeData.shapes[i].shapeInfo.y);

		// TODO: Convert to bitwise calculation 

		if (currMap < internalMap)
		{
			// NOTE: We need to get access to the id and not
			//		 just the shape type so we can get it 
			//		 transform data 
			id = i;
			internalMap = currMap;
		}

		// When converting to bitwise this will be more optimal 
		//internalMap = min(currMap, internalMap);
	}

	return internalMap;
}



void main()
{
    //outColor = allCalcs(gl_FragCoord.xy);


    // Default color is screen UV 
	vec2 uv = fragColor.xy;
	
	
	int stepMax = 100;
	float sceneMap = 99999.0f; 
	
	// TODO: Adjust to be current sample distance for
	//		 dynamic adjustments 
	float stepSize = 0.001f;

	vec3 pos = vec3(uv, 0.0f);

	float maxDis = 3.0;
	float dis = 0.0;
	for(int s = 0; s < stepMax; s++)
	{
		float currMap = MapScene(pos); 
		sceneMap = min(currMap, sceneMap);

		if(sceneMap <= 0.01f)
		{
			outColor = vec4(CalcNormal(pos), 1.0);
			return;
		}
		else if(dis >= maxDis)
		{
			break;
		}
		
		dis += currMap;
		pos += normalize(vec3(uv, -1.0)) * currMap;
	}

	outColor = vec4(1.0, 1.0, 1.0, 1.0);
}



float Sphere(vec3 p, vec3 center, float radius)
{
	return distance(p, center) - radius;
}

float Box(vec3 p, vec3 center, vec3 size)
{
	vec3 q = abs(p - center) - size;
	return length(max(q, 0.0)) + min(max(q.x, max(q.y, q.z)), 0.0);
}

float RoundBox(vec3 p, vec3 center, vec3 size, float rounding)
{   
    vec3 q = abs(p - center) - size + rounding;
    return length(max(q, 0.0)) + min(max(q.x,max(q.y,q.z)),0.0) - rounding;
}

float FrameBox(vec3 p, vec3 center, vec3 b, float e )
{
  p = abs(p - center) - b;
  vec3 q = abs(p+e)-e;
  return min(min(
      length(max(vec3(p.x,q.y,q.z),0.0))+min(max(p.x,max(q.y,q.z)),0.0),
      length(max(vec3(q.x,p.y,q.z),0.0))+min(max(q.x,max(p.y,q.z)),0.0)),
      length(max(vec3(q.x,q.y,p.z),0.0))+min(max(q.x,max(q.y,p.z)),0.0));
}

// Returns the dis to the given shape 
float SampleSDF(vec3 p, uint type, uint startP)
{
	switch(type)
	{
		case SPHERE:

			return Sphere(
				// Ray point 
				p, 
				// Sphere Center 
				vec3(ShapeParamData.params[startP + 0].x, ShapeParamData.params[startP + 1].x, ShapeParamData.params[startP + 2].x), 
				// Sphere radius 
				ShapeParamData.params[startP + 3].x);
			break;
		case BOX: 
			return Box(
				p, 
				// Box Center 
				vec3(ShapeParamData.params[startP + 0].x, ShapeParamData.params[startP + 1].x, ShapeParamData.params[startP + 2].x), 
				// Size
				vec3(ShapeParamData.params[startP + 3].x, ShapeParamData.params[startP + 4].x, ShapeParamData.params[startP + 5].x)
			);
		case ROUND_BOX:
			return RoundBox(
				p, 
				// Box Center 
				vec3(ShapeParamData.params[startP + 0].x, ShapeParamData.params[startP + 1].x, ShapeParamData.params[startP + 2].x), 
				// Size
				vec3(ShapeParamData.params[startP + 3].x, ShapeParamData.params[startP + 4].x, ShapeParamData.params[startP + 5].x),
				// Rounding 
				ShapeParamData.params[startP + 6].x
			);
		case FRAME_BOX:
			return FrameBox(
				p, 
				// Box Center 
				vec3(ShapeParamData.params[startP + 0].x, ShapeParamData.params[startP + 1].x, ShapeParamData.params[startP + 2].x), 
				// Size
				vec3(ShapeParamData.params[startP + 3].x, ShapeParamData.params[startP + 4].x, ShapeParamData.params[startP + 5].x),
				// Thickness 
				ShapeParamData.params[startP + 6].x
			);
	}

	return 1.0f;
}









// LIGHTING 
//			vec3 p = ro + rd * t;
//        vec3 normal = calcNormal(p);
//        vec3 light = vec3(0, 2, 0);
//        
//        // Calculate diffuse lighting by taking the dot product of 
//        // the light direction (light-p) and the normal.
//        float dif = clamp(dot(normal, normalize(light - p)), 0., 1.);
//		
//        // Multiply by light intensity (5) and divide by the square
//        // of the distance to the light.
//        dif *= 5. / dot(light - p, light - p);
//        
//        
//        color = vec4(vec3(pow(dif, 0.4545)), 1);     // Gamma correction