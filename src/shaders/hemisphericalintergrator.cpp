#include "hemisphericalintegrator.h"
#include "../core/utils.h"
#include "../core/hemisphericalsampler.h"

HemisphericalIntegrator::HemisphericalIntegrator() :
    hitColor(Vector3D(1, 0, 0))
{ }

HemisphericalIntegrator::HemisphericalIntegrator(Vector3D hitColor_, Vector3D bgColor_) :
    Shader(bgColor_), hitColor(hitColor_)
{ }

Vector3D HemisphericalIntegrator::computeColor(const Ray &r, const std::vector<Shape*> &objList, const std::vector<LightSource*> &lsList) const
{
    Intersection its;

    if (Utils::getClosestIntersection(r, objList, its)) {
        Vector3D wo = -r.d; // Viewing direction (from its to the cam position)
        Vector3D n = its.normal.normalized(); // Normal at position x
        Vector3D wi; // Incident light direction (depending on each lightsource)
        Vector3D fr; // Reflectance (diffuse + specular)
        Vector3D color = Vector3D(0, 0, 0); // Resulting color
        //int V=0; // Visibility term (1 if visible; 0 if occluded)
        const Material& material = its.shape->getMaterial();

        // 1. MIRROR MATERIAL
        if (material.hasSpecular()) {
            // Perfect reflected direction at its
            Vector3D wr = (2 * dot(wo, n) * n - wo).normalized();
            // Reflected ray
            Ray reflectedRay = Ray(its.itsPoint, wr);
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
                Ray reflectedRay = Ray(its.itsPoint, wr);
                color = computeColor(reflectedRay, objList, lsList);
            }
            // If discriminant is not negative...
            else {
                // We compute the transmissive refraction
				Vector3D wt = (-mu * wo + n * (mu * dot(n, wo) - sqrt(discr))).normalized();
                //Refracted ray
				Ray refractedRay = Ray(its.itsPoint, wt);
				//Refracted color from this direction
				color = computeColor(refractedRay, objList, lsList);
            }
        }

        // 3. PHONG MATERIAL
        else if (material.hasDiffuseOrGlossy()) {
            HemisphericalSampler sampler;
            int N = 200;
            // For every sample per pixel...
            for (int i = 0; i < N; i++) {
                // Incident light direction (from its to lightsource position)
                wi = sampler.getSample(n);
                // Ray from its towards the direction wi
                Ray shadowRay = Ray(its.itsPoint, wi);
                Intersection shadowIts;
                // Get closest intersection from its towards the direction wi...
                Vector3D Li(0, 0, 0);
                if (Utils::getClosestIntersection(shadowRay, objList, shadowIts)) {
                    if (shadowIts.shape->getMaterial().isEmissive()) {
                        Li = shadowIts.shape->getMaterial().getEmissiveRadiance();
                        Vector3D lightPos = shadowIts.itsPoint;
                    }
                }
                // Direction (negative direction will be black, a value of 0)
                double costheta = std::max(0.0, dot(wi, n));

                // REFLECTANCE OF THE MATERIAL (diffuse + specular)
                fr = material.getReflectance(n, wo, wi);

				// DIRECT ILLUMINATION (DIFFUSE + SPECULAR)
                color += 1.0 / N * (Li * fr * costheta) * 2 * M_PI;
            }
            // AMBIENT LIGHT
            Vector3D ambientLight = Vector3D(0.2, 0.2, 0.2);
            // Compute diffuse coefficient from the material
            Vector3D kd = material.getDiffuseReflectance();

			// DIRECT ILLUMINATION (AMBIENT + DIFFUSE + SPECULAR)
			color += ambientLight * kd;
        }

        // 4. EMISSIVE MATERIAL
        if (material.isEmissive()) {
            color += material.getEmissiveRadiance();
        }

        return color;
    }
    
    return bgColor;
}
