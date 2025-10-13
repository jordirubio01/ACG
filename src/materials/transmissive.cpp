#include "transmissive.h"

#include <iostream>

Transmissive::Transmissive()
{ }

Transmissive::Transmissive(double idx_):
r_idx(idx_) {}

Transmissive::Transmissive(double idx_, Vector3D Kd_, Vector3D Ks_, float alpha_):
r_idx(idx_), rho_d(Kd_), Ks(Ks_), alpha(alpha_){}


Vector3D Transmissive::getReflectance(const Vector3D& n, const Vector3D& wo,
    const Vector3D& wi) const {

    return Vector3D(0,0,0);

};

double Transmissive::getIndexOfRefraction() const
{
    return r_idx;
}


Vector3D Transmissive::getEmissiveRadiance() const
{
    return Vector3D(0.0);
}


Vector3D Transmissive::getDiffuseReflectance() const
{
    return rho_d;
}

