#include "neeimprovedintegrator.h"
#include "../core/utils.h"
#include "../core/hemisphericalsampler.h"

NEEImprovedIntegrator::NEEImprovedIntegrator() :
    hitColor(Vector3D(1, 0, 0))
{
}

NEEImprovedIntegrator::NEEImprovedIntegrator(Vector3D hitColor_, Vector3D bgColor_) :
    Shader(bgColor_), hitColor(hitColor_)
{
}

Vector3D NEEImprovedIntegrator::computeColor(const Ray& r,
    const std::vector<Shape*>& objList,
    const std::vector<LightSource*>& lsList) const {

    Intersection its;
    if (Utils::getClosestIntersection(r, objList, its)) {
        Vector3D wo = -r.d; // Viewing direction (from its to the cam position)

        // 1. Emitted radiance (returns vector 0 if not emissive)
        Vector3D color = its.shape->getMaterial().getEmissiveRadiance();
        
        // 2. Reflected radiance
        color += reflectedRadiance(its, wo, r.depth, objList, lsList);

        return color;
    }

    return bgColor;
}

Vector3D NEEImprovedIntegrator::reflectedRadiance(const Intersection& its, const Vector3D& wo, int depth,
	const std::vector<Shape*>& objList,
	const std::vector<LightSource*>& lsList) const {

	Vector3D dir = directRadiance(its, wo, objList, lsList);
	Vector3D ind = indirectRadiance(its, wo, depth, objList, lsList);

    return dir + ind;
}

Vector3D NEEImprovedIntegrator::directRadiance(const Intersection& its, const Vector3D& wo, const std::vector<Shape*>& objList,
                                        const std::vector<LightSource*>& lsList) const {
    Vector3D n = its.normal; // Normal at position x
    Vector3D wi; // Incident light direction (depending on each lightsource)
    Vector3D fr; // Reflectance (diffuse + specular)
    Vector3D color = Vector3D(0, 0, 0); // Resulting color
    int V = 0; // Visibility term (1 if visible; 0 if occluded)
    const Material& material = its.shape->getMaterial();

    int N = 5;
    // For every light source...
    for (int i = 0; i < lsList.size(); i++) {
        // For every sample in the area lightsource...
        for (int j = 0; j < N; j++) {
            // Incident light position
            Vector3D lightPos = lsList[i]->sampleLightPosition();
            // Incident light direction (from its to lightsource position)
            wi = (lightPos - its.itsPoint).normalized();
            // Geometric term (negative scalar products will be black, a value of 0)
            double geometricTerm = (std::max(0.0, dot(wi, n))
                * std::max(0.0, dot(-wi, lsList[i]->getNormal())))
                / pow((lightPos - its.itsPoint).length(), 2);

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

            if (V == 1) {
                // REFLECTANCE OF THE MATERIAL (diffuse + specular)
                fr = material.getReflectance(n, wo, wi);
                // Emmited light intensity from the area light source
                Vector3D Le = lsList[i]->getIntensity();

                // DIRECT ILLUMINATION (DIFFUSE + SPECULAR)
                color += 1.0 / N * (Le * fr * geometricTerm) * lsList[i]->getArea();
            }
        }
    }
	return color;
}

Vector3D NEEImprovedIntegrator::indirectRadiance(const Intersection& its, const Vector3D& wo, int depth,
    const std::vector<Shape*>& objList,
    const std::vector<LightSource*>& lsList) const
{
    const int MAX_DEPTH = 3;
    Vector3D Lind(0, 0, 0);
    if (depth >= MAX_DEPTH) {
        return Lind;
    }
    Vector3D n = its.normal.normalized(); // Normal at position x
    Vector3D wi; // Incident light direction (depending on each lightsource)
    const Material& material = its.shape->getMaterial();
    Vector3D fr; // Reflectance (diffuse + specular)

    // 1. MIRROR MATERIAL
    if (material.hasSpecular()) {
        // Perfect reflected direction at its
        Vector3D wr = (2 * dot(wo, n) * n - wo).normalized();
        // Reflected ray
        Ray reflectedRay = Ray(its.itsPoint, wr);
        reflectedRay.depth = depth + 1;
        // Reflected color from this direction
        Lind = computeColor(reflectedRay, objList, lsList);
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
            reflectedRay.depth = depth + 1;
            Lind = computeColor(reflectedRay, objList, lsList);
        }
        // If discriminant is not negative...
        else {
            // We compute the transmissive refraction
            Vector3D wt = (-mu * wo + n * (mu * dot(n, wo) - sqrt(discr))).normalized();
            //Refracted ray
            Ray refractedRay = Ray(its.itsPoint, wt);
            refractedRay.depth = depth + 1;
            //Refracted color from this direction
            Lind = computeColor(refractedRay, objList, lsList);
        }
    }

    // 3. PURE PATH TRACING FOR DIFFUSE AND GLOSSY MATERIALS
    else if (material.hasDiffuseOrGlossy()) {
        HemisphericalSampler sampler;
        int N = 100;

        if (depth >= 1) {
            N = 1; // Reduce number of samples for deeper bounces
        }
        // For every sample per pixel...
        for (int i = 0; i < N; i++) {
            // Incident light direction (from its to lightsource position)
            wi = sampler.getSample(n);
            // Ray from its towards the direction wi
            Ray shadowRay = Ray(its.itsPoint, wi, depth + 1);
            Intersection shadowIts;
            // Get closest intersection from its towards the direction wi...
            Vector3D Li(0, 0, 0);
            if (Utils::getClosestIntersection(shadowRay, objList, shadowIts)) {
                Li = reflectedRadiance(shadowIts, -wi, depth + 1, objList, lsList);
            }
            // Direction (negative direction will be black, a value of 0)
            double costheta = std::max(0.0, dot(wi, n));

            // REFLECTANCE OF THE MATERIAL (diffuse + specular)
            fr = material.getReflectance(n, wo, wi);

            // REFLECTED RADIANCE (DIFFUSE + SPECULAR)
            Lind += 1.0 / N * (Li * fr * costheta) * 2 * M_PI;
        }
    }

    return Lind;
}

