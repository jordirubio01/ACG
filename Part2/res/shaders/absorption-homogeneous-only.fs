#version 330 core

in vec3 v_position;				  // Vertex position (local coords)
in vec3 v_world_position;		  // Vertex position (world coords)

uniform vec3 u_local_camera_pos;  // Camera position (world coords)
uniform mat4 u_model;             // Model matrix (from local to world)
uniform vec3 u_boxMin;            // Box min (local coords)
uniform vec3 u_boxMax;            // Box max (local coords)

uniform vec4 u_color;			  // Volume color
uniform vec4 u_bg_color;		  // Background color
uniform float u_absorption_coeff; // Absorption coefficient

out vec4 FragColor;

/// FUNCTION USED FOR INTERSECTION
// adapted from intersectCube in https://github.com/evanw/webgl-path-tracing/blob/master/webgl-path-tracing.js
// compute the near and far intersections of the cube (stored in the x and y components) using the slab method
// no intersection means vec.x > vec.y (really tNear > tFar)
vec2 intersectAABB(vec3 rayOrigin, vec3 rayDir, vec3 boxMin, vec3 boxMax) {
    vec3 tMin = (boxMin - rayOrigin) / rayDir;
    vec3 tMax = (boxMax - rayOrigin) / rayDir;
    vec3 t1 = min(tMin, tMax);
    vec3 t2 = max(tMin, tMax);
    float tNear = max(max(t1.x, t1.y), t1.z);
    float tFar = min(min(t2.x, t2.y), t2.z);
    return vec2(tNear, tFar);
};

/// MAIN FUNCTION
void main()
{
	// 1. INITIALIZE RAY (POSITION AND DIRECTION)
	vec3 origin_local = u_local_camera_pos;
	vec3 direction_local = normalize(v_position - origin_local);
    
    // 2. COMPUTE INTERSECTIONS WITH THE VOLUME AUXILIARY GEOMETRY
    // We use the implementation suggested in the statement
    //vec3 boxMin = vec3(-1.0);
    //vec3 boxMax = vec3(1.0);
    vec2 t_hit = intersectAABB(origin_local, direction_local, u_boxMin, u_boxMax);
    float tnear = max(t_hit.x, 0.0);
    float tfar = t_hit.y;

    // 3. COMPUTE THE OPTICAL THICKNESS
    float tau = max(0.0, tfar - tnear) * u_absorption_coeff;

    // 4. COMPUTE THE TRANSMITTANCE
    float transmittance = exp(-tau);

    // 5. COMPUTE AND SET FINAL PIXEL COLOR
    vec4 color = vec4(u_bg_color.rgb * transmittance, 1.0);

	FragColor = color;
}