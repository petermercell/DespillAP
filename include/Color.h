#ifndef COLOR_H
#define COLOR_H

#include <algorithm>
#include <cmath>

#include "include/Pixel.h"

namespace nuke = DD::Image;

// Código del encabezado

namespace color
{
  namespace luma
  {
    void ToLumaRec709(const float (&rgb)[3], float (&luma)[3])
    {
      float luminance = rgb[0] * 0.2126f + rgb[1] * 0.7152f + rgb[2] * 0.0722f;
      luma[0] = luminance;
      luma[1] = luminance;
      luma[2] = luminance;
    }

    void ToLumaCcir601(const float (&rgb)[3], float (&luma)[3])
    {
      float luminance = rgb[0] * 0.299f + rgb[1] * 0.587f + rgb[2] * 0.114f;
      luma[0] = luminance;
      luma[1] = luminance;
      luma[2] = luminance;
    }

    void ToLumaRec2020(const float (&rgb)[3], float (&luma)[3])
    {
      float luminance = rgb[0] * 0.2627f + rgb[1] * 0.6780f + rgb[2] * 0.0593f;
      luma[0] = luminance;
      luma[1] = luminance;
      luma[2] = luminance;
    }

    void ToLumaAverage(const float (&rgb)[3], float (&luma)[3])
    {
      float luminance = (rgb[0] + rgb[1] + rgb[2]) / 3.0f;
      luma[0] = luminance;
      luma[1] = luminance;
      luma[2] = luminance;
    }

    void ToLumaMax(const float (&rgb)[3], float (&luma)[3])
    {
      float luminance = std::max({rgb[0], rgb[1], rgb[2]});
      luma[0] = luminance;
      luma[1] = luminance;
      luma[2] = luminance;
    }
  }  // namespace luma

  void HueRotate(const float (&rgb)[3], float (&hue)[3], const float &angle)
  {
    if(angle == 0.0f) {
      return;
    }

    float cosA = std::cos(angle * M_PI / 180.0f);
    float sinA = std::sin(angle * M_PI / 180.0f);
    float sqrt3 = std::sqrt(3.0f);
    float common = (rgb[0] + rgb[1] + rgb[2]) * (1.0f - cosA) / 3.0f;

    hue[0] = common + rgb[0] * cosA + (-rgb[1] / sqrt3 + rgb[2] / sqrt3) * sinA;
    hue[1] = common + rgb[1] * cosA + (rgb[0] / sqrt3 - rgb[2] / sqrt3) * sinA;
    hue[2] = common + rgb[2] * cosA + (-rgb[0] / sqrt3 + rgb[1] / sqrt3) * sinA;
  }

}  // namespace color

#endif  // COLOR_H
