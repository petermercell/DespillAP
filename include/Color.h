#ifndef COLOR_H
#define COLOR_H

#include "include/Pixel.h"

namespace nuke = DD::Image;

// Código del encabezado

namespace color
{
  template <typename T>
  void ConvertToRec709(imgcore::Pixel<T>& pixel)
  {
    imgcore::Pixel<T> result = pixel;

    const float r = static_cast<float>(pixel.GetVal(0));
    const float g = static_cast<float>(pixel.GetVal(0));
    const float b = static_cast<float>(pixel.GetVal(0));

    float luma = 0.2126f * r + 0.7152f * g + 0.0722f * b;

    for(unsigned int i = 0; i < pixel.GetSize(); ++i) {
      result.SetVal(static_cast<T>(luma), i);
    }

    return result;
  }
}  // namespace color

#endif  // COLOR_H
