#include "neeintegrator.h"
#include "../core/utils.h"
#include "../core/hemisphericalsampler.h"

NEEIntegrator::NEEIntegrator() :
    hitColor(Vector3D(1, 0, 0))
{
}

NEEIntegrator::NEEIntegrator(Vector3D hitColor_, Vector3D bgColor_) :
    Shader(bgColor_), hitColor(hitColor_)
{
}

Vector3D NEEIntegrator::computeColor(const Ray& r,
    const std::vector<Shape*>& objList,
    const std::vector<LightSource*>& lsList) const {

    Intersection its;
    if (Utils::getClosestIntersection(r, objList, its)) {
        Vector3D wo = -r.d; // Viewing direction (from its to the cam position)
        Vector3D n = its.normal.normalized(); // Normal at position x
        Vector3D wi; // Incident light direction (depending on each lightsource)
        Vector3D fr; // Reflectance (diffuse + specular)
        Vector3D color = Vector3D(0, 0, 0); // Resulting color
        //int V=0; // Visibility term (1 if visible; 0 if occluded)
        const Material& material = its.shape->getMaterial();

        // EMISSIVE RADIANCE
        if (material.isEmissive()) {
            color += material.getEmissiveRadiance();
        }

		// REFLECTED RADIANCE
        color += reflectedRadiance(its, wo, r.depth, objList, lsList);

        return color;
    }

    return bgColor;
}

Vector3D NEEIntegrator::reflectedRadiance(const Intersection& its, const Vector3D& wo, int depth,
	const std::vector<Shape*>& objList,
	const std::vector<LightSource*>& lsList) const {

	Vector3D dir = directRadiance(its, wo, objList, lsList);
	Vector3D ind = indirectRadiance(its, wo, depth, objList, lsList);
	return dir + ind;
}

Vector3D NEEIntegrator::directRadiance(const Intersection& its, const Vector3D& wo, const std::vector<Shape*>& objList,
                                        const std::vector<LightSource*>& lsList) const {
    //TO DO
	return bgColor;
}

Vector3D NEEIntegrator::indirectRadiance(const Intersection& its, const Vector3D& wo, int depth,
    const std::vector<Shape*>& objList,
    const std::vector<LightSource*>& lsList) const
{
    //TO DO
    return bgColor;
}

