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
        return (its.normal+Vector3D(1, 1, 1)) / 2;
    }

    return bgColor;
}
