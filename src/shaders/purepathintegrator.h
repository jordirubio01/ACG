#ifndef PUREPATHINTEGRATOR_H
#define PUREPATHINTEGRATOR_H

#include "shader.h"
#include "materials/phong.h"

class PurePathIntegrator : public Shader
{
public:
    PurePathIntegrator();
    PurePathIntegrator(Vector3D hitColor, Vector3D bgColor_);

    virtual Vector3D computeColor(const Ray &r,
                             const std::vector<Shape*> &objList,
                             const std::vector<LightSource*> &lsList) const;

    Vector3D hitColor;
};

#endif // PUREPATHINTEGRATOR_H
