#version 330 core

in vec3 v_position;				  // Vertex position (local coords)
in vec3 v_world_position;		  // Vertex position (world coords)

uniform vec3 u_local_camera_pos;  // Camera position (world coords)
uniform mat4 u_model;             // Model matrix (from local to world)
uniform vec3 u_boxMin;            // Box min (local coords)
uniform vec3 u_boxMax;            // Box max (local coords)

uniform vec4 u_color;			  // Volume color (emmited light color)
uniform vec4 u_bg_color;		  // Background color
uniform float u_absorption_coeff; // Absorption coefficient
uniform float u_scattering_coeff; // Scattering coefficient

uniform int u_absorption_type;    // 0 is homogeneous; 1 is heterogeneous
uniform float u_step_size;        // Step size for ray marching
uniform float u_noise_freq;       // Frequency for noise sampling
uniform float u_density_scale;    // Scales noise to absorption
uniform sampler3D u_texture;      // Texture of the material

uniform float u_light_intensity;  // Intensity of the light to be scattered
uniform vec4 u_light_color;       // Color
uniform vec3 u_local_light_position; // Light position (local coords)
uniform vec3 u_light_position;    // Light position (world coords) --> no s'utilitza, però el deixo per si de cas perque està a setUniforms de light.cpp
uniform int u_num_light_steps;    // Number of steps for light marching
uniform float u_g;                 // g parameter for Henyey-Greenstein phase function
     

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

/// FUNCTION USED FOR NOISE
// Simplex 3D Noise 
// by Ian McEwan, Stefan Gustavson (https://github.com/stegu/webgl-noise)
vec4 permute(vec4 x){return mod(((x*34.0)+1.0)*x, 289.0);}
vec4 taylorInvSqrt(vec4 r){return 1.79284291400159 - 0.85373472095314 * r;}

float snoise(vec3 v){ 
  const vec2  C = vec2(1.0/6.0, 1.0/3.0) ;
  const vec4  D = vec4(0.0, 0.5, 1.0, 2.0);
  // First corner
  vec3 i  = floor(v + dot(v, C.yyy) );
  vec3 x0 =   v - i + dot(i, C.xxx) ;
  // Other corners
  vec3 g = step(x0.yzx, x0.xyz);
  vec3 l = 1.0 - g;
  vec3 i1 = min( g.xyz, l.zxy );
  vec3 i2 = max( g.xyz, l.zxy );
  //  x0 = x0 - 0. + 0.0 * C 
  vec3 x1 = x0 - i1 + 1.0 * C.xxx;
  vec3 x2 = x0 - i2 + 2.0 * C.xxx;
  vec3 x3 = x0 - 1. + 3.0 * C.xxx;
  // Permutations
  i = mod(i, 289.0 ); 
  vec4 p = permute( permute( permute( 
             i.z + vec4(0.0, i1.z, i2.z, 1.0 ))
           + i.y + vec4(0.0, i1.y, i2.y, 1.0 )) 
           + i.x + vec4(0.0, i1.x, i2.x, 1.0 ));
  // Gradients
  // ( N*N points uniformly over a square, mapped onto an octahedron.)
  float n_ = 1.0/7.0; // N=7
  vec3  ns = n_ * D.wyz - D.xzx;

  vec4 j = p - 49.0 * floor(p * ns.z *ns.z);  //  mod(p,N*N)

  vec4 x_ = floor(j * ns.z);
  vec4 y_ = floor(j - 7.0 * x_ );    // mod(j,N)

  vec4 x = x_ *ns.x + ns.yyyy;
  vec4 y = y_ *ns.x + ns.yyyy;
  vec4 h = 1.0 - abs(x) - abs(y);

  vec4 b0 = vec4( x.xy, y.xy );
  vec4 b1 = vec4( x.zw, y.zw );

  vec4 s0 = floor(b0)*2.0 + 1.0;
  vec4 s1 = floor(b1)*2.0 + 1.0;
  vec4 sh = -step(h, vec4(0.0));

  vec4 a0 = b0.xzyw + s0.xzyw*sh.xxyy ;
  vec4 a1 = b1.xzyw + s1.xzyw*sh.zzww ;

  vec3 p0 = vec3(a0.xy,h.x);
  vec3 p1 = vec3(a0.zw,h.y);
  vec3 p2 = vec3(a1.xy,h.z);
  vec3 p3 = vec3(a1.zw,h.w);
  //Normalise gradients
  vec4 norm = taylorInvSqrt(vec4(dot(p0,p0), dot(p1,p1), dot(p2, p2), dot(p3,p3)));
  p0 *= norm.x;
  p1 *= norm.y;
  p2 *= norm.z;
  p3 *= norm.w;
  // Mix final noise value
  vec4 m = max(0.6 - vec4(dot(x0,x0), dot(x1,x1), dot(x2,x2), dot(x3,x3)), 0.0);
  m = m * m;
  return 42.0 * dot( m*m, vec4( dot(p0,x0), dot(p1,x1), dot(p2,x2), dot(p3,x3) ) );
}


// SECOND RAY MARCHING --> he vist que per en GLSL es millor no repetir noms de variables tot i que estiguin dins la funció 
float computeLightTransmittance(vec3 sample_)
{
    vec3 Ldir_local = normalize(u_local_light_position - sample_);

    vec2 hit_ = intersectAABB(sample_, Ldir_local, u_boxMin, u_boxMax);
    float tmax = hit_.y;

    if (tmax <= 0.0)
        return 0.0;

    // si el l'ultim pas no queda enter no s'està tenint encara en compte
    float step_ = tmax / float(u_num_light_steps);
    float t_ = step_ * 0.5;
    float tau_ = 0.0;
    float transmittance_ = 1.0;

    //potser es millor fer un while pero com que estem iterant per nombre de steps de moment deixo for
    for (int i = 0; i < u_num_light_steps && transmittance_ > 0.0001; i++)
    {
        vec3 p = sample_ + Ldir_local * t_;
        float mu_ = u_absorption_coeff;

        if (u_absorption_type == 1) {
            float density_ = max(0.0, snoise(p * u_noise_freq));
            mu_ = density_ * u_density_scale;
        }
        else if (u_absorption_type == 2) {
            vec3 texture_pos_ = (p + vec3(1.0)) / 2.0;
            float density_ = texture(u_texture, texture_pos_).r;
            mu_ = density_ * u_density_scale;
        }

        float mu_s = u_scattering_coeff;
        float mu_t = mu_ + mu_s;

        tau_ += mu_t * step_;
        transmittance_ = exp(-tau_);
        t_ += step_;
    }

    return transmittance_;
}

float phaseHG(float cosTheta, float g)
{
    float denom = 1.0 + (g * g) - 2.0 * g * cosTheta;
    return (1.0 / (4.0 * 3.14159265)) * ((1.0 - (g * g) / pow(denom, 1.5));
}

/// MAIN FUNCTION
void main()
{
    // 1. INITIALIZE RAY (POSITION AND DIRECTION)
    vec3 origin_local = u_local_camera_pos;
    vec3 direction_local = normalize(v_position - origin_local);

    // 2. COMPUTE INTERSECTIONS WITH THE VOLUME AUXILIARY GEOMETRY
    vec2 t_hit = intersectAABB(origin_local, direction_local, u_boxMin, u_boxMax);
    float tnear = max(t_hit.x, 0.0);
    float tfar  = t_hit.y;

    // if no intersection...
    if (tnear > tfar) {
        FragColor = u_bg_color;
        return;
    }

    // TRAVERSAL LOOP
    // Some variables initialization before entering the conditions
    float transmittance = 1.0;         // Initially there is no absorption
    float tau = 0.0;                   // Optical thickness is initially zero
    vec4 color = vec4(0.0);            // Initialize resulting color
    vec4 emitted_color = u_color;      // Emmited light color
    float mu = u_absorption_coeff;     // Absorption coefficient at each point
    float step_size = u_step_size;     // Traversal loop step size
    float t = tnear + step_size * 0.5; // Start from the closest intersection

    // While t is inside the volume and optical thickness is not too high...
    while (t < tfar && transmittance > 0.0001) {
        // 3. COMPUTE THE OPTICAL THICKNESS
        vec3 sample_pos = origin_local + direction_local * t;
        // If heterogeneous, absorption coefficient changes...
        if (u_absorption_type == 1) { // S'HA DE CANVIAR, NOMÉS ÉS UNA PROVA
            float density = max(0.0, snoise(sample_pos * u_noise_freq));
            mu = density * u_density_scale;
        }
        // Else if we have a 3D texture...
        else if (u_absorption_type == 2) { // S'HA DE CANVIAR, NOMÉS ÉS UNA PROVA
            vec3 texture_pos = (sample_pos + vec3(1.0)) / 2.0;
            float density = texture(u_texture, texture_pos).r;
            mu = density * u_density_scale;
        }

        float mu_s = u_scattering_coeff;
        float mu_t = mu + mu_s;

        tau += mu_t * step_size;
        transmittance = exp(-tau);

        vec3 Le = u_color.rgb;  

        //  IN SCATTERING
        float transmittance_scatt = computeLightTransmittance(sample_pos);
        vec3 Li = u_light_color.rgb * u_light_intensity;
        vec3 Ls = Li * transmittance_scatt;

        // PHASE FUNCTION (HENYEY-GREENSTEIN)
        vec3 Ldir = normalize(u_local_light_position - sample_pos); // direction from sample to light
        float cosTheta = dot(direction_local, Ldir);                // angle between view and light directions
        float phase = phaseHG(cosTheta, u_g);                       // HG phase evaluation
        Ls *= phase;                                                // modulate scattered light by the phase

        // FULL MODEL
        color.rgb += (mu * Le + mu_s * Ls) * transmittance * step_size;
        color.a = 1.0;

        t += step_size;
    }

    color.rgb += u_bg_color.rgb * transmittance;

    FragColor = color;
}
