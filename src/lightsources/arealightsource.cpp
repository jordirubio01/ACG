#include "arealightsource.h"

AreaLightSource::AreaLightSource(Square* areaLightsource_) :
    myAreaLightsource(areaLightsource_)
{ }



Vector3D AreaLightSource::getIntensity() const
{
    return myAreaLightsource->getMaterial().getEmissiveRadiance();
}


Vector3D AreaLightSource::sampleLightPosition()   const
{
    //FILL(...)
    // 
    //New Randam Pos inside Area Lightsource
    return Vector3D(0.0);
}

Vector3D AreaLightSource::generateRandomPosition() const
{
    double u = (double)std::rand() / RAND_MAX;
    double v = (double)std::rand() / RAND_MAX;

    Vector3D randpos = myAreaLightsource->corner
        + u * myAreaLightsource->v1
        + v * myAreaLightsource->v2;
    return randpos;
}

