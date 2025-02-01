#version 450

layout(location = 0) in vec3 fragColor;

layout(location = 0) out vec4 outColor;

const float WIDTH = 800.0f;
const float HEIGHT = 600.0f;


// SDF functions 
// From: https://iquilezles.org/articles/distfunctions/
float Sphere(vec3 p, vec3 center, float radius);
float Box(vec3 p, vec3 center, vec3 size);
float RoundBox(vec3 p, vec3 center, vec3 size, float rounding);


float map(vec3 p)
{
    float d =  RoundBox(p, vec3(-1, 0, -5), vec3(1, 1, 1), 0.1); //distance(p, vec3(-1, 0, -5)) - 1.;     // sphere at (-1,0,5) with radius 1
    //d = min(d, Box(p, vec3(2, 0, -3), vec3(1, 1, 1)));    // second sphere
   // d = min(d, distance(p, vec3(-2, 0, -2)) - 1.);   // and another
    //d = min(d, p.y + 1.);                            // horizontal plane at y = -1
    return d;
}

//
// Calculate the normal by taking the central differences on the distance field.
//
vec3 calcNormal(vec3 p)
{
    vec2 e = vec2(1.0, -1.0) * 0.0005;
    return normalize(
        e.xyy * map(p + e.xyy) +
        e.yyx * map(p + e.yyx) +
        e.yxy * map(p + e.yxy) +
        e.xxx * map(p + e.xxx));
}

vec4 allCalcs(vec2 fragCoord) {
    vec3 ro = vec3(0, 0, 1);                           // ray origin

    vec2 iResolution = vec2(WIDTH, HEIGHT);
    vec2 q = (fragCoord.xy - .5 * iResolution.xy ) / iResolution.y;
    vec3 rd = normalize(vec3(q, 0.) - ro);             // ray direction for fragCoord.xy

    vec4 color;

    // March the distance field until a surface is hit.
    float h, t = 1.;
    for (int i = 0; i < 256; i++) {
        h = map(ro + rd * t);
        t += h;
        if (h < 0.01) break;
    }

    if (h < 0.01) {
        vec3 p = ro + rd * t;
        vec3 normal = calcNormal(p);
        vec3 light = vec3(0, 2, 0);
        
        // Calculate diffuse lighting by taking the dot product of 
        // the light direction (light-p) and the normal.
        float dif = clamp(dot(normal, normalize(light - p)), 0., 1.);
		
        // Multiply by light intensity (5) and divide by the square
        // of the distance to the light.
        dif *= 5. / dot(light - p, light - p);
        
        
        color = vec4(vec3(pow(dif, 0.4545)), 1);     // Gamma correction
    } else {
        color = vec4(0, 0, 0, 1);
    }

    return color;
}

void main()
{
    outColor = allCalcs(gl_FragCoord.xy);
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