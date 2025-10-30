#ifndef AREAINTEGRATOR_H
#define AREAINTEGRATOR_H

#include "shader.h"
#include "materials/phong.h"

class AreaIntegrator : public Shader
{
public:
    AreaIntegrator();
    AreaIntegrator(Vector3D hitColor, Vector3D bgColor_);

    virtual Vector3D computeColor(const Ray &r,
                             const std::vector<Shape*> &objList,
                             const std::vector<LightSource*> &lsList) const;

    Vector3D hitColor;
};

#endif // AREAINTEGRATOR_H
