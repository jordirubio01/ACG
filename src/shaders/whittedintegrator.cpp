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
        Vector3D wo = -r.d; // Viewing direction (from its to the cam position)
        Vector3D n = its.normal.normalized(); // Normal at position x
        Vector3D wi; // Incident light direction (depending on each lightsource)
        Vector3D fr; // Reflectance (diffuse + specular)
        Vector3D Lo = Vector3D(0, 0, 0); // Direct illumination
        int V=0; // Visibility term (1 if visible; 0 if occluded)

        // For every light source...
        for (int i = 0; i < lsList.size(); i++) {
            // Incident light position
            Vector3D lightPos = lsList[i]->sampleLightPosition();
            // Incident light direction (from its to lightsource position)
            wi = (lightPos - its.itsPoint).normalized();
            // Direction (negative direction will be black, a value of 0)
            double costheta = std::max(0.0, dot(wi, n));

            // VISIBILITY TERM
            // Ray from position x to the light source
            // (it does not include the extremes, so will not collide with the its itself)
            Ray shadowRay = Ray(its.itsPoint, wi); 
            Intersection shadowIts;
            // Get closest intersection from its to the lightsource direction if exists...
            if (Utils::getClosestIntersection(shadowRay, objList, shadowIts)) {
                double distItsToLight = (lightPos - its.itsPoint).length();
                double distItsToObstacle = (shadowIts.itsPoint - its.itsPoint).length();
                // If there is an obstacle between its and light...
                if (distItsToObstacle < distItsToLight) {
                    V = 0; // Object is not visible
                }
                else V = 1; // Else, object is visible
            }
            else V = 1; // If there is no other object in this direction, its is visible

            // REFLECTANCE OF THE MATERIAL (diffuse + specular)
            const Material& material = its.shape->getMaterial();
            fr = material.getReflectance(n, wo, wi);
            // Incident light intensity
            Vector3D Li = lsList[i]->getIntensity();

            // AMBIENT LIGHT
            Vector3D ambientLight = Vector3D(0.2, 0.2, 0.2);
            // Compute diffuse coefficient from the material
            Vector3D kd = material.getDiffuseReflectance(); // Ambiance constant based on diffuse constant

            // DIRECT ILLUMINATION
            Lo += kd * ambientLight + Li * fr * costheta * V;
        }
        return Lo;
    }
    


    return bgColor;
}
