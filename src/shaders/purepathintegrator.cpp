#include "purepathintegrator.h" // NEEINTTEGRATOR.h
#include "../core/utils.h"
#include "../core/hemisphericalsampler.h"

PurePathIntegrator::PurePathIntegrator() :
    hitColor(Vector3D(1, 0, 0))
{ }

PurePathIntegrator::PurePathIntegrator(Vector3D hitColor_, Vector3D bgColor_) :
    Shader(bgColor_), hitColor(hitColor_)
{ }

Vector3D PurePathIntegrator::computeColor(const Ray &r, const std::vector<Shape*> &objList, const std::vector<LightSource*> &lsList) const
{
    const int MAX_DEPTH = 5;
    Intersection its;

    if (Utils::getClosestIntersection(r, objList, its)) {
        Vector3D wo = -r.d; // Viewing direction (from its to the cam position)
        Vector3D n = its.normal; // Normal at position x
        Vector3D wi; // Incident light direction (depending on each lightsource)
        Vector3D fr; // Reflectance (diffuse + specular)
        //int V=0; // Visibility term (1 if visible; 0 if occluded)
        const Material& material = its.shape->getMaterial();
        Vector3D color = material.getEmissiveRadiance(); // Emitted light (vector 0 if not emissive)

        // If maximum depth is reached...
        if (r.depth >= MAX_DEPTH) {
            return color;
        }

        // 1. MIRROR MATERIAL
        if (material.hasSpecular()) {
            // Perfect reflected direction at its
            Vector3D wr = (2 * dot(wo, n) * n - wo).normalized();
            // Reflected ray
            Ray reflectedRay = Ray(its.itsPoint, wr, r.depth);
			//reflectedRay.depth = r.depth + 1;
            // Reflected color from this direction
            color = computeColor(reflectedRay, objList, lsList);
        }

        // 2. TRANSMISSIVE MATERIAL
        else if (material.hasTransmission()) {
			double n_i = 1.0; // Index of refraction of the medium outside the object (air)
			double n_t = material.getIndexOfRefraction(); // Index of refraction of the medium inside the object
            double mu; // Ratio of refractive indices

			// Check if the ray enters or exits the object
            // If ray exits the object...
            if (dot(wo, n) < 0) {
				n = -n;
                mu = n_i / n_t;
            }
            // If ray enters the object...
			else mu = n_t / n_i;

			// Check if the discriminant is positive (if not, there is total internal reflection)
            double discr = 1.0 - (mu * mu) * (1.0 - dot(n, wo) * dot(n, wo));
            // If discriminant is negative...
            if (discr < 0) {
                //Total internal reflection, it behaves like a mirror
                Vector3D wr = (2 * dot(wo, n) * n - wo).normalized();
                Ray reflectedRay = Ray(its.itsPoint, wr, r.depth+1);
                //reflectedRay.depth = r.depth + 1;
                color = computeColor(reflectedRay, objList, lsList);
            }
            // If discriminant is not negative...
            else {
                // We compute the transmissive refraction
				Vector3D wt = (-mu * wo + n * (mu * dot(n, wo) - sqrt(discr))).normalized();
                //Refracted ray
				Ray refractedRay = Ray(its.itsPoint, wt, r.depth);
                //refractedRay.depth = r.depth + 1;
				//Refracted color from this direction
				color = computeColor(refractedRay, objList, lsList);
            }
        }

		// 3. PURE PATH TRACING FOR DIFFUSE AND GLOSSY MATERIALS
        else if (material.hasDiffuseOrGlossy()) {
            HemisphericalSampler sampler;
            int N = 256;
            Vector3D Lo(0, 0, 0);

			if (r.depth >= 1) {
				N = 1; // Reduce number of samples for deeper bounces
            }
            // For every sample per pixel...
            for (int i = 0; i < N; i++) {
                // Incident light direction (from its to lightsource position)
                wi = sampler.getSample(n);
                // Ray from its towards the direction wi
                Ray NewRay = Ray(its.itsPoint, wi);
                NewRay.depth = r.depth + 1;
                // REFLECTANCE OF THE MATERIAL (diffuse + specular)
                fr = material.getReflectance(n, wo, wi);
				// Reflected color from this direction (recursive call)
                Vector3D Li = computeColor(NewRay, objList, lsList);
                // Direction (negative direction will be black, a value of 0)
                double costheta = std::max(0.0, dot(wi, n));
                // ILLUMINATION (DIFFUSE + SPECULAR)
                Lo += Li * fr * costheta * 2 * M_PI;
            }
			// Average Lo over N samples
			color += Lo / N;
        }

        return color;
    }
    
    return bgColor;
}

