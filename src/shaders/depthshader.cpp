#include "depthshader.h"

#include "../core/utils.h"

DepthShader::DepthShader() :
    color(Vector3D(1, 0, 0))
{ }

DepthShader::DepthShader(Vector3D hitColor_, double maxDist_, Vector3D bgColor_) :
    Shader(bgColor_), maxDist(maxDist_), color(hitColor_)
{ }

Vector3D DepthShader::computeColor(const Ray &r, const std::vector<Shape*> &objList, const std::vector<LightSource*> &lsList) const
{
    //(FILL..)

    //if..

    //else...

    Intersection its;
    if (Utils::getClosestIntersection(r, objList, its)) {
        Vector3D B = its.itsPoint;
        Vector3D A = r.o;

        float distance = (A-B).length();
        return Vector3D(0, 1.0 - distance/maxDist, 0);
    }

    return bgColor;
}
