#ifndef NEEINTEGRATOR_H
#define NEEINTEGRATOR_H

#include "shader.h"
#include "materials/phong.h"

class NEEIntegrator : public Shader
{
public:
    NEEIntegrator();
    NEEIntegrator(Vector3D hitColor_, Vector3D bgColor_);

    // Funció principal per obtenir el color d’un raig
    virtual Vector3D computeColor(const Ray& r,
        const std::vector<Shape*>& objList,
        const std::vector<LightSource*>& lsList) const;

private:
    // Funcions helpers buides segons pseudocodi NEE
    Vector3D reflectedRadiance(const Intersection& its, const Vector3D& wo, int depth,
        const std::vector<Shape*>& objList,
        const std::vector<LightSource*>& lsList) const;

    Vector3D directRadiance(const Intersection& its, const Vector3D& wo,
        const std::vector<Shape*>& objList,
        const std::vector<LightSource*>& lsList) const;

    Vector3D indirectRadiance(const Intersection& its, const Vector3D& wo, int depth,
        const std::vector<Shape*>& objList,
        const std::vector<LightSource*>& lsList) const;

    Vector3D hitColor;
};

#endif // NEEINTEGRATOR_H
