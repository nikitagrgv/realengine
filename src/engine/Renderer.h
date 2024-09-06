#pragma once


class Camera;
class Material;
class Light;

class Renderer
{
public:
    void clearBuffers();
    void renderWorld(Camera *camera, Light *light);

    int getNumRenderedIndices() const { return num_rendered_indices_; }

    void resetStatistics();

private:
    void use_material(Material *material);

private:
    int num_rendered_indices_{0};
};
