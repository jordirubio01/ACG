#ifndef WHITTEDINTEGRATOR_H
#define WHITTEDINTEGRATOR_H

#include "shader.h"
#include "materials/phong.h"

class WhittedIntegrator : public Shader
{
public:
    WhittedIntegrator();
    WhittedIntegrator(Vector3D hitColor, Vector3D bgColor_);

    virtual Vector3D computeColor(const Ray &r,
                             const std::vector<Shape*> &objList,
                             const std::vector<LightSource*> &lsList) const;

    Vector3D hitColor;
};

#endif // WHITTEDINTEGRATOR_H
