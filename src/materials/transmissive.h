#ifndef TRANSMISSIVEMATERIAL
#define TRANSMISSIVEMATERIAL

#include "material.h"
#define _USE_MATH_DEFINES
#include <math.h>

class Transmissive : public Material
{
public:
    Transmissive();
    Transmissive(double idx_);
    Transmissive(double idx_, Vector3D Kd_, Vector3D Ks_,  float    alpha_);

    Vector3D getReflectance(const Vector3D& n, const Vector3D& wo,
        const Vector3D& wi)const ;

    bool hasSpecular() const { return true; }
    bool hasTransmission() const { return false; }
    bool hasDiffuseOrGlossy() const { return false; }
    bool isEmissive() const { return false; }

    double getIndexOfRefraction() const;
    Vector3D getEmissiveRadiance() const;
    Vector3D getDiffuseReflectance() const;


private:
    double r_idx;
    Vector3D rho_d;
    Vector3D Ks;
    float    alpha;
    

};
#endif // MATERIAL