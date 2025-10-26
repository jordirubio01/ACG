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
        Vector3D wo = -r.d; // direcció del raig que ve de la càmera
        Vector3D color(0, 0, 0);

        // 1. Radiació emesa pel punt
        if (its.shape->getMaterial().isEmissive()) {
            color += its.shape->getMaterial().getEmissiveRadiance();
        }

        // 2. Radiació reflectida (segons NEE)
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

