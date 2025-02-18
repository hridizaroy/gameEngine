#version 450

layout(location = 0) in vec4 fragColor;

layout(location = 0) out vec4 outColor;


// Example data 

struct Shape
{
	int shapeType;
	int startP;
};

Shape shapes[] = 
{
	{0, 0}
};

float parameters[] = 
{ 
	// Sphere 
	0.0f, 0.0f, -1.0f,	// Center 
	1.0f				// Radius 
};

float Sphere(vec3 p, vec3 center, float radius);
float SampleSDF(vec3 pos, int type, int startP);

#define SPHERE 0


void main()
{
	
	// Default color is screen UV 
	outColor = vec4(1.0, 1.0, 1.0, 1.0); //fragColor;
	vec2 uv = fragColor.xy;


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
		for(int i = 0; i < shapes.length(); i++)
		{
			sceneMap = min(SampleSDF(pos, shapes[i].shapeType, shapes[i].startP), 
							sceneMap);
		}

		//outColor = vec4(1.0f, 0.0f, 0.0f, 0.0f);
		if (sceneMap <= threshold)
		{
			outColor = vec4(1.0, 0.0f, 0.0f, 1.0f);
			return;
		}

		pos += normalize(vec3(uv, -1.0)) * stepSize;
	}


}

// SDF function for a sphere 
float Sphere(vec3 p, vec3 center, float radius)
{
	return distance(p, center) - radius;
}

// Returns the dis to the given shape 
float SampleSDF(vec3 p, int type, int startP)
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
	}

	return 1.0f;
}