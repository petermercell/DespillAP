#ifndef UTILS_H
#define UTILS_H

#include <DDImage/Box.h>
#include <DDImage/ImagePlane.h>
#include <DDImage/Iop.h>

#include "include/Bounds.h"
#include "include/Image.h"

namespace nuke = DD::Image;

namespace imgcore
{
  imgcore::Bounds BoxToBounds(nuke::Box box)
  {
    return imgcore::Bounds(box.x(), box.y(), box.r() - 1, box.t() - 1);
  }

  nuke::Box BoundsToBox(imgcore::Bounds bounds)
  {
    return nuke::Box(bounds.x1(), bounds.y1(), bounds.x2() + 1,
                     bounds.y2() + 1);
  }

  imgcore::Bounds InputBounds(nuke::Iop* input)
  {
    return imgcore::BoxToBounds(input->info().box());
  }

  void FetchImage(imgcore::Image* image, nuke::Iop* input,
                  nuke::Channel channel)
  {
    nuke::ImagePlane channel_plane(imgcore::BoundsToBox(image->GetBounds()),
                                   false, channel);
    input->fetchPlane(channel_plane);
    image->MemCpyIn(channel_plane.readable(),
                    channel_plane.rowStride() * sizeof(float));
  }
  void FetchImage(imgcore::Image* image, nuke::Iop* input,
                  nuke::Channel channel, imgcore::Bounds plane_bounds)
  {
    nuke::ImagePlane channel_plane(imgcore::BoundsToBox(plane_bounds), false,
                                   channel);
    input->fetchPlane(channel_plane);
    image->MemCpyIn(channel_plane.readable(),
                    channel_plane.rowStride() * sizeof(float), plane_bounds);
  }
  const float* GetPlanePtr(const nuke::ImagePlane& plane,
                           nuke::Channel channel = nuke::Chan_Black)
  {
    int channel_offset = plane.chanNo(channel);
    return plane.readable() + channel_offset;
  }
  const float* GetPlanePtr(const nuke::ImagePlane& plane, int x, int y,
                           nuke::Channel channel = nuke::Chan_Black)
  {
    int channel_offset = plane.chanNo(channel);
    return plane.readable() +
           (plane.bounds().clampy(y) - plane.bounds().y()) * plane.rowStride() +
           (plane.bounds().clampx(x) - plane.bounds().x()) * plane.colStride() +
           channel_offset;
  }
  std::size_t GetPlanePitch(const nuke::ImagePlane& plane)
  {
    return static_cast<std::size_t>(plane.rowStride() * sizeof(float));
  }

}  // namespace imgcore

#endif  // UTILS_H