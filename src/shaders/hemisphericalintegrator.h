#ifndef HEMISPHERICALINTEGRATOR_H
#define HEMISPHERICALINTEGRATOR_H

#include "shader.h"
#include "materials/phong.h"

class HemisphericalIntegrator : public Shader
{
public:
    HemisphericalIntegrator();
    HemisphericalIntegrator(Vector3D hitColor, Vector3D bgColor_);

    virtual Vector3D computeColor(const Ray &r,
                             const std::vector<Shape*> &objList,
                             const std::vector<LightSource*> &lsList) const;

    Vector3D hitColor;
};

#endif // HEMISPHERICALINTEGRATOR_H
