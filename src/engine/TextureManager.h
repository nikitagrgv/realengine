#pragma once

#include "AbstractManager.h"

#include <Texture.h>

class Texture;

class TextureManager : public AbstractManager<Texture>
{
public:
    TextureManager();
};