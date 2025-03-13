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


float parameters[] = 
{ 
	// Frame Box 
	 0.5f,  0.0f, -1.0f,	// Center
	 0.1f,  0.1f,  0.1f,	// Size
	 0.025f,				// Thickness 

	// Sphere 
	-0.5f, 0.0f, -1.0f,		// Center 
	 0.1f					// Radius 
};

layout(std140, binding = 2) readonly buffer storageBuffer
{
	Shape shapes[];
} ShapeData;



// SDF functions 
// From: https://iquilezles.org/articles/distfunctions/
float Sphere(vec3 p, vec3 center, float radius);
float Box(vec3 p, vec3 center, vec3 size);
float RoundBox(vec3 p, vec3 center, vec3 size, float rounding);
float BoxFrame(vec3 p, vec3 center, vec3 size, float thickness);
float SampleSDF(vec3 pos, uint type, uint startP);

#define SPHERE 0
#define BOX 1
#define ROUND_BOX 2
#define FRAME_BOX 3

float map(vec3 p)
{
    float d =  RoundBox(p, vec3(-1, 0, -5), vec3(1, 1, 1), 0.1); //distance(p, vec3(-1, 0, -5)) - 1.;     // sphere at (-1,0,5) with radius 1
    //d = min(d, Box(p, vec3(2, 0, -3), vec3(1, 1, 1)));    // second sphere
    // d = min(d, distance(p, vec3(-2, 0, -2)) - 1.);   // and another
    //d = min(d, p.y + 1.);                            // horizontal plane at y = -1
    return d;
}

/// Calculate the normal by taking the central differences on the distance field.
vec3 CalcNormal(vec3 p)
{
    vec2 e = vec2(1.0, -1.0) * 0.0005;
    return normalize(
        e.xyy * map(p + e.xyy) +
        e.yyx * map(p + e.yyx) +
        e.yxy * map(p + e.yxy) +
        e.xxx * map(p + e.xxx));
}



void main()
{
    //outColor = allCalcs(gl_FragCoord.xy);

    // Default color is screen UV 
	outColor = vec4(1.0, 1.0, 1.0, 1.0); //fragColor;
	vec2 uv = fragColor.xy;

	//if (ShapeData.shapes.length() == 3 )
	//{
	//	outColor = vec4(1,0,0,1);
	//}
	//else
	//{
	//	outColor = vec4(0,1,0,1);
	//}
	//
	//return;


	int stepMax = 100;
	// TODO: Adjust to be current sample distance for
	//		 dynamic adjustments 
	float stepSize = 0.01f;
	float threshold = 0.01f; 

	vec3 pos = vec3(uv, 0.0f);

	float sceneMap = 99999.0f; 


	for(int s = 0; s < stepMax; s++)
	{
		// Brute force scene check 
		for(int i = 0; i < ShapeData.shapes.length(); i++)
		{
			//sceneMap = min(SampleSDF(pos, ShapeData.shapes[i].shapeType, ShapeData.shapes[i].startP), 
			//				sceneMap);
			sceneMap = min(SampleSDF(pos, ShapeData.shapes[i].shapeInfo.x, ShapeData.shapes[i].shapeInfo.y), 
							sceneMap);
		}

		if (sceneMap <= threshold)
		{
			outColor = vec4(1.0, 0.0f, 0.0f, 1.0f);
			return;
		}

		pos += normalize(vec3(uv, -1.0)) * stepSize;
	}

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
				vec3(parameters[startP + 0], parameters[startP + 1], parameters[startP + 2]), 
				// Sphere radius 
				parameters[startP + 3]);
			break;
		case BOX: 
			return Box(
				p, 
				// Box Center 
				vec3(parameters[startP + 0], parameters[startP + 1], parameters[startP + 2]), 
				// Size
				vec3(parameters[startP + 3], parameters[startP + 4], parameters[startP + 5])
			);
		case ROUND_BOX:
			return RoundBox(
				p, 
				// Box Center 
				vec3(parameters[startP + 0], parameters[startP + 1], parameters[startP + 2]), 
				// Size
				vec3(parameters[startP + 3], parameters[startP + 4], parameters[startP + 5]),
				// Rounding 
				parameters[startP + 6]
			);
		case FRAME_BOX:
			return FrameBox(
				p, 
				// Box Center 
				vec3(parameters[startP + 0], parameters[startP + 1], parameters[startP + 2]), 
				// Size
				vec3(parameters[startP + 3], parameters[startP + 4], parameters[startP + 5]),
				// Thickness 
				parameters[startP + 6]
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