#ifndef DISPATCHER_H
#define DISPATCHER_H

#include "include/Color.h"
#include "include/Constants.h"

void LumaTransform(const float (&rgb)[3], float (&luma)[3], int mode)
{
  switch(mode) {
    case Constants::LUMA_REC709:
      color::luma::ToLumaRec709(rgb, luma);
      break;
    case Constants::LUMA_CCIR601:
      color::luma::ToLumaCcir601(rgb, luma);
      break;
    case Constants::LUMA_REC2020:
      color::luma::ToLumaRec2020(rgb, luma);
      break;
    case Constants::LUMA_AVERAGE:
      color::luma::ToLumaAverage(rgb, luma);
      break;
    case Constants::LUMA_MAX:
      color::luma::ToLumaMax(rgb, luma);
      break;
    default:
      break;
  }
}

#endif  // DISPATCHER_H