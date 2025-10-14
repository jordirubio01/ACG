#include "whittedintegrator.h"
#include "../core/utils.h"

WhittedIntegrator::WhittedIntegrator() :
    hitColor(Vector3D(1, 0, 0))
{ }

WhittedIntegrator::WhittedIntegrator(Vector3D hitColor_, Vector3D bgColor_) :
    Shader(bgColor_), hitColor(hitColor_)
{ }

Vector3D WhittedIntegrator::computeColor(const Ray &r, const std::vector<Shape*> &objList, const std::vector<LightSource*> &lsList) const
{
    //(FILL..)
    
    Intersection its;

    if (Utils::getClosestIntersection(r, objList, its)) {
        Vector3D wo = -r.d; // Viewing direction (from its to the cam position)
        Vector3D n = its.normal.normalized(); // Normal at position x
        Vector3D wi; // Incident light direction (depending on each lightsource)
        Vector3D fr; // Reflectance (diffuse + specular)
        Vector3D color = Vector3D(0, 0, 0); // Resulting color
        int V=0; // Visibility term (1 if visible; 0 if occluded)
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

			// Check if the ray enters or exits the object
            if (dot(wo, n) < 0) {
            // Ray exits the object
				n = -n;
				std::swap(n_i, n_t);
            }
			double mu = n_t / n_i;

			// Check if the discriminant is positive (if not, there is total internal reflection)
            double discr = 1.0 - (mu * mu) * (1.0 - dot(n, wo) * dot(n, wo));

            if (discr < 0) {
                //Total internal reflection. It behaves like a mirror
                Vector3D wr = (2 * dot(wo, n) * n - wo).normalized();
                Ray reflectedRay = Ray(its.itsPoint, wr);
                color = computeColor(reflectedRay, objList, lsList);
            }
            else {
				Vector3D wt = (-mu * wo + n * (mu * dot(n, wo) - sqrt(discr))).normalized();
                //Refracted ray
				Ray refractedRay = Ray(its.itsPoint, wt);
				//Refracted color from this direction
				color = computeColor(refractedRay, objList, lsList);
            }
        }

        // 3. PHONG MATERIAL
        else if (material.hasDiffuseOrGlossy()) {
            // For every light source...
            for (int i = 0; i < lsList.size(); i++) {
                // Incident light position
                Vector3D lightPos = lsList[i]->sampleLightPosition();
                // Incident light direction (from its to lightsource position)
                wi = (lightPos - its.itsPoint).normalized();
                // Direction (negative direction will be black, a value of 0)
                double costheta = std::max(0.0, dot(wi, n));

                // VISIBILITY TERM
                // Ray from its to the light source
                // (it does not include the extremes, so will not collide with the its itself)
                Ray shadowRay = Ray(its.itsPoint, wi);
                Intersection shadowIts;
                // Get closest intersection from its to the lightsource direction if exists...
                if (Utils::getClosestIntersection(shadowRay, objList, shadowIts)) {
                    double distItsToLight = (lightPos - its.itsPoint).length();
                    double distItsToObstacle = (shadowIts.itsPoint - its.itsPoint).length();
                    // If there is an obstacle between its and light...
                    if (distItsToObstacle < distItsToLight) {
                        V = 0; // Object is not visible
                    }
                    else V = 1; // Else, object is visible
                }
                else V = 1; // If there is no other object in this direction, its is visible

                // REFLECTANCE OF THE MATERIAL (diffuse + specular)
                fr = material.getReflectance(n, wo, wi);
                // Incident light intensity
                Vector3D Li = lsList[i]->getIntensity();

				// DIRECT ILLUMINATION (DIFFUSE + SPECULAR)
                color += Li * fr * costheta * V;
            }
            // AMBIENT LIGHT
            Vector3D ambientLight = Vector3D(0.2, 0.2, 0.2);
            // Compute diffuse coefficient from the material
            Vector3D kd = material.getDiffuseReflectance();

			// DIRECT ILLUMINATION (AMBIENT + DIFFUSE + SPECULAR)
			color += ambientLight * kd;
        }
        return color;
    }
    


    return bgColor;
}
