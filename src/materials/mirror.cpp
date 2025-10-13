#include "mirror.h"

#include <iostream>

Mirror::Mirror()
{ }

Mirror::Mirror(Vector3D Kd_, Vector3D Ks_, float alpha_):
rho_d(Kd_), Ks(Ks_), alpha(alpha_){}


Vector3D Mirror::getReflectance(const Vector3D& n, const Vector3D& wo,
    const Vector3D& wi) const {

    return Vector3D(0,0,0);

};

double Mirror::getIndexOfRefraction() const
{
    std::cout << "Warning! Calling \"Material::getIndexOfRefraction()\" for a non-transmissive material"
              << std::endl;

    return -1;
}


Vector3D Mirror::getEmissiveRadiance() const
{
    return Vector3D(0.0);
}


Vector3D Mirror::getDiffuseReflectance() const
{
    return rho_d;
}

