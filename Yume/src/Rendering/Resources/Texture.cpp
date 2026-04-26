#include "Texture.h"
#include "ServiceLocator/ServiceLocator.h"
#include "Services/ResourceManager/ResourceManager.h"

#include <stb_image.h>

namespace Yume {

Texture::~Texture() {
    Unload();                 
}

stbi_uc* Texture::GetPixels() const {
    return m_pixels;
}

bool Texture::DoLoad() {
    return ServiceLocator::GetService<ResourceManager>().ReadImage<Texture>(GetId(), m_width, m_height, m_channels, &m_pixels);
}

bool Texture::DoUnload() {
    if (IsLoaded()) {
        stbi_image_free(m_pixels);
    }
    return true;
}

int Texture::GetWidth() const {
    return m_width;
}

int Texture::GetHeight() const {
    return m_height;
}

int Texture::GetChannels() const {
    return m_channels;
}

}
