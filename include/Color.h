#ifndef COLOR_H
#define COLOR_H

#include <algorithm>
#include <cmath>

#include "DDImage/DDMath.h"
#include "DDImage/Vector3.h"
#include "include/Pixel.h"

namespace nuke = DD::Image;
namespace color
{
  enum ScreenColor {
    kScreenColorRed = 0,
    kScreenColorGreen = 1,
    kScreenColorBlue = 2,
  };
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

  Vector3 HueRotate(const float (&rgb)[3], const float &angle)
  {
    Vector3 vec(rgb);
    Vector3 hue;

    if(angle == 0.0f) {
      return;
    }

    float cosA = std::cosf(angle * M_PI_F / 180.0f);
    float sinA = std::sinf(angle * M_PI_F / 180.0f);
    float sqrt3 = std::sqrtf(3.0f);
    float common = (vec.x + vec.y + vec.z) * (1.0f - cosA) / 3.0f;

    hue[0] = common + vec.x * cosA + (-vec.y / sqrt3 + vec.z / sqrt3) * sinA;
    hue[1] = common + vec.y * cosA + (vec.x / sqrt3 - vec.z / sqrt3) * sinA;
    hue[2] = common + vec.z * cosA + (-vec.x / sqrt3 + vec.y / sqrt3) * sinA;

    return hue;
  }

  Vector3 VectorToPlane(const float (&vec1)[3], const float (&vec2)[3])
  {
    Vector3 ret;
    Vector3 proj;
    Vector3 v1(vec1);
    Vector3 v2(vec2);

    float scale = v1.dot(v2) / v2.dot(v1);

    proj[0] = v2.x * scale;
    proj[1] = v2.y * scale;
    proj[2] = v2.z * scale;

    ret[0] = v2.x - proj.x;
    ret[1] = v2.y - proj.y;
    ret[2] = v2.z - proj.z;

    return ret;
  }

  float ColorAngle(const float (&vec1)[3], const float (&vec2)[3])
  {
    Vector3 normal(1.0f, 1.0f, 1.0f);
    Vector3 v1(vec1);
    Vector3 v2(vec2);

    float mag1 = std::sqrtf(v1.dot(v1));
    float mag2 = std::sqrtf(v2.dot(v2));

    float cosTheta = clamp(v1.dot(v2) / (mag1 * mag2), -1.0f, 1.0f);
    float angle = std::acosf(cosTheta);

    if(normal.dot(v1.cross(v2)) > 0.0f) {
      angle = -angle;
    }

    return angle;
  }

}  // namespace color

#endif  // COLOR_H
