#include "whittedintegrator.h"
#include "../core/utils.h"

WhittedIntegrator::WhittedIntegrator() :
    hitColor(Vector3D(1, 0, 0))
{ }

WhittedIntegrator::WhittedIntegrator(Vector3D hitColor_, Vector3D bgColor_) :
    Shader(bgColor_), hitColor(hitColor_)
{ }

Vector3D WhittedIntegrator::computeColor(const Ray &r, const std::vector<Shape*> &objList, const std::vector<LightSource*> &lsList) const
{
    //(FILL..)
    
    Intersection its;

    if (Utils::getClosestIntersection(r, objList, its)) {
        Vector3D wo = -r.d.normalized(); // Viewing direction
        Vector3D n = its.normal.normalized(); // Normal at position x
        Vector3D wi; // Incident light direction depending on each light source
        Vector3D fr; // Reflectance
        Vector3D Lo = Vector3D(0, 0, 0); // Direct illumination
        
        int V=0;

        for (int i = 0; i < lsList.size(); i++) {
            // Incident light direction
            Vector3D lightPos = lsList[i]->sampleLightPosition();
            wi = (lightPos - its.itsPoint).normalized();
            // Direction (negative direction will be black, a value of 0)
            double costheta = std::max(0.0, dot(wi, n));

            // Visibility term
            Ray shadowRay = Ray(its.itsPoint + n * 1e-4, wi); // Ray from position x to the light source
            Intersection shadowIts;

            if (Utils::getClosestIntersection(shadowRay, objList, shadowIts)) {
                double distPointToLight = (lightPos - its.itsPoint).length();
                double distPointToObstacle = (shadowIts.itsPoint - its.itsPoint).length();

                if (distPointToObstacle < distPointToLight) {
                    V = 0; // Object is not visible
                }
                else V = 1;
            }

            // Compute the reflectance
            // REVISAR AQUESTA PART !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
            const Material& material = its.shape->getMaterial();
            fr = material.getReflectance(n, wo, wi);
            Vector3D Li = lsList[i]->getIntensity();


            // Define ambient light intensity
            Vector3D ambientLight = Vector3D(0.2, 0.2, 0.2);
            // Compute diffuse coefficient from the material
            Vector3D kd = material.getDiffuseReflectance(); // suposant que tens aquesta funció

            Lo += kd * ambientLight + Li * fr * costheta * V;
        }
        return Lo;
    }
    


    return bgColor;
}
