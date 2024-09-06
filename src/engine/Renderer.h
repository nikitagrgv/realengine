#pragma once


class Camera;
class Material;
class Light;

class Renderer
{
public:
    void clear();
    void renderWorld(Camera *camera, Light *light);

private:
    void use_material(Material *material);
};
