// This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
// If a copy of the MPL was not distributed with this file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is a modified version of code from:
// https://github.com/AuthorityFX/afx-nuke-plugins
// Originally authored by Ryan P. Wilson, Authority FX, Inc.
//
// Modifications for DespillAP plugin:
// - Namespace renamed to a generic form
// - Removed CUDA-specific logic
// - Adjusted for CPU-only use
// - Integrated threading logic with DespillAP's design

#ifndef IMAGE_H
#define IMAGE_H

// ========================================================================
// INCLUDES FOR CROSS-PLATFORM MEMORY MANAGEMENT
// ========================================================================
#ifndef _WIN32
#include <malloc.h>  // Linux/Unix: aligned memory functions
#else
#include <stdlib.h>  // Windows: standard memory functions
#endif

// ========================================================================
// REQUIRED INCLUDES
// ========================================================================
#include <boost/ptr_container/ptr_list.hpp>  // Smart containers
#include <cstdint>                           // Fixed-size integer types
#include <memory>                            // Smart pointers (shared_ptr)
#include <vector>                            // Standard container

#include "include/Attribute.h"  // Attribute system
#include "include/Bounds.h"     // Rectangular region handling
#include "include/Pixel.h"      // Classes for pixel handling
#include "include/Threading.h"  // Threading utilities

struct IppiSize
{
  int width;
  int height;
};

namespace imgcore
{
  // ========================================================================
  // IMAGEBASE<T> TEMPLATE CLASS - Generic image with data type T
  // ========================================================================
  template <class T>
  class ImageBase : public AttributeBase  // Inherits attribute system
  {
   public:
    // ---- CONSTRUCTORS ----

    // Default constructor: empty image
    ImageBase<T>() : ptr_(nullptr), pitch_(0), region_(imgcore::Bounds()) {}

    // Constructor with specific region
    explicit ImageBase<T>(const imgcore::Bounds& region) : ptr_(nullptr), pitch_(0)
    {
      Allocate(region);  // Allocate memory for the region
    }

    // Constructor with width and height
    ImageBase<T>(unsigned int width, unsigned int height) : ptr_(nullptr), pitch_(0)
    {
      Allocate(width, height);  // Create region from dimensions
    }

    // Copy constructor: creates deep copy
    ImageBase<T>(const ImageBase<T>& other) : ptr_(nullptr), pitch_(0)
    {
      Copy(other);  // Copy all data
    }

    // Move constructor: transfers ownership
    ImageBase<T>(ImageBase<T>&& other)
        : ptr_(other.ptr_), pitch_(other.pitch_), region_(other.region_)
    {
      // Reset source object to avoid double-delete
      other.ptr_ = nullptr;
      other.pitch_ = 0;
      other.region_ = imgcore::Bounds();
    }

    // ---- ASSIGNMENT OPERATORS ----

    // Copy assignment
    ImageBase<T>& operator=(const ImageBase<T>& other)
    {
      Copy(other);
      return *this;
    }

    // Move assignment
    ImageBase<T>& operator=(ImageBase<T>&& other)
    {
      if(this != &other) {  // Avoid self-assignment
        Deallocate();       // Free current memory
        // Transfer ownership
        ptr_ = other.ptr_;
        pitch_ = other.pitch_;
        region_ = other.region_;
        // Reset source
        other.ptr_ = nullptr;
        other.pitch_ = 0;
        other.region_ = imgcore::Bounds();
      }
      return *this;
    }

    // Destructor: automatically frees memory
    ~ImageBase<T>() { Deallocate(); }

    // ---- MEMORY MANAGEMENT ----

    // Allocate memory for width x height image
    void Allocate(unsigned int width, unsigned int height)
    {
      Allocate(imgcore::Bounds(0, 0, width - 1, height - 1));
    }

    // Allocate memory for specific region
    void Allocate(const imgcore::Bounds& region)
    {
      Deallocate();  // Free previous memory
      region_ = region;

      // Calculate pitch aligned to 64 bytes for SIMD optimization
      pitch_ = (((region_.GetWidth() * sizeof(T) + 63) / 64) * 64);

#ifdef _WIN32
      // Windows: use _aligned_malloc for 64-byte alignment
      ptr_ = reinterpret_cast<T*>(_aligned_malloc(64, pitch_ * region_.GetHeight()));
#else
      // Linux/Unix: use standard aligned_alloc
      ptr_ = reinterpret_cast<T*>(aligned_alloc(64, pitch_ * region_.GetHeight()));
#endif
    }

    // Deep copy from another image
    void Copy(const ImageBase<T>& other)
    {
      region_ = other.region_;
      Allocate(region_);  // Allocate same region

      // Copy row by row to handle different pitch
      for(int y = region_.y1(); y <= region_.y2(); ++y) {
        memcpy(GetPtr(region_.x1(), y), other.GetPtr(other.region_.x1(), y),
               region_.GetWidth() * sizeof(T));
      }
    }

    // Free allocated memory
    void Deallocate()
    {
      if(ptr_ != nullptr) {
        free(ptr_);
        ptr_ = nullptr;
      }
    }

    // ---- MEMORY COPY FUNCTIONS ----

    // Copy data TO this image from external pointer
    void MemCpyIn(const T* ptr, std::size_t pitch)
    {
      const T* source_ptr = ptr;
      T* dest_ptr = GetPtr(region_.x1(), region_.y1());
      std::size_t size = region_.GetWidth() * sizeof(T);

      // Copy row by row handling different pitch
      for(int y = region_.y1(); y <= region_.y2(); ++y) {
        memcpy(dest_ptr, source_ptr, size);
        // Advance source_ptr using its pitch
        source_ptr =
            reinterpret_cast<const T*>((reinterpret_cast<const std::uint8_t*>(source_ptr) + pitch));
        // Advance dest_ptr using our pitch
        dest_ptr = this->GetNextRow(dest_ptr);
      }
    }

    // Copy data to specific region
    void MemCpyIn(const T* ptr, std::size_t pitch, imgcore::Bounds region)
    {
      const T* source_ptr = ptr;
      T* dest_ptr = GetPtr(region.x1(), region.y1());
      std::size_t size = region.GetWidth() * sizeof(T);

      for(int y = region.y1(); y <= region.y2(); ++y) {
        memcpy(dest_ptr, source_ptr, size);
        source_ptr =
            reinterpret_cast<const T*>((reinterpret_cast<const std::uint8_t*>(source_ptr) + pitch));
        dest_ptr = this->GetNextRow(dest_ptr);
      }
    }

    // Copy from another image with specific region
    void MemCpyIn(const ImageBase<T>& source_image, imgcore::Bounds region)
    {
      MemCpyIn(source_image.GetPtr(region.x1(), region.y1()), source_image.GetPitch(), region);
    }

    // Copy from another image (full region)
    void MemCpyIn(const ImageBase<T>& source_image)
    {
      // Calculate intersection between images to avoid overflow
      imgcore::Bounds region = source_image.GetBounds().GetIntersection(region_);
      MemCpyIn(source_image.GetPtr(region.x1(), region.y1()), source_image.GetPitch(), region);
    }

    // ---- COPY OUT FUNCTIONS ----

    // Copy FROM this image to external pointer
    void MemCpyOut(T* ptr, std::size_t pitch) const
    {
      T* source_ptr = GetPtr(region_.x1(), region_.y1());
      T* dest_ptr = ptr;
      std::size_t size = region_.GetWidth() * sizeof(T);

      for(int y = region_.y1(); y <= region_.y2(); ++y) {
        memcpy(dest_ptr, source_ptr, size);
        source_ptr = this->GetNextRow(dest_ptr);
        dest_ptr = reinterpret_cast<T*>((reinterpret_cast<std::uint8_t*>(dest_ptr) + pitch));
      }
    }

    // Copy specific region to external pointer
    void MemCpyOut(T* ptr, std::size_t pitch, imgcore::Bounds region) const
    {
      T* source_ptr = GetPtr(region.x1(), region.y1());
      T* dest_ptr = ptr;
      std::size_t size = region.GetWidth() * sizeof(T);

      for(int y = region.y1(); y <= region_.y2(); ++y) {
        memcpy(dest_ptr, source_ptr, size);
        source_ptr = this->GetNextRow(dest_ptr);
        dest_ptr = reinterpret_cast<T*>((reinterpret_cast<std::uint8_t*>(dest_ptr) + pitch));
      }
    }

    // Copy to another image with specific region
    void MemCpyOut(const ImageBase<T>& dest_image, imgcore::Bounds region) const
    {
      MemCpyOut(dest_image.GetPtr(region.x1(), region.y1()), dest_image.GetPitch(), region);
    }

    // Copy to another image (full region)
    void MemCpyOut(const ImageBase<T>& dest_image) const
    {
      imgcore::Bounds region = dest_image.GetBounds();
      MemCpyOut(dest_image.GetPtr(region.x1(), region.y1()), dest_image.GetPitch(), region);
    }

    // ---- POINTER ACCESS AND NAVIGATION ----

    // Get base pointer of the image
    T* GetPtr() const { return ptr_; }

    // Get pointer to specific coordinate (x,y)
    T* GetPtr(int x, int y) const
    {
      return reinterpret_cast<T*>(reinterpret_cast<std::uint8_t*>(ptr_) +
                                  (y - region_.y1()) * pitch_ +     // Row offset
                                  (x - region_.x1()) * sizeof(T));  // Column offset
    }

    // Get pointer with automatic clamping to bounds
    T* GetPtrBnds(int x, int y) const
    {
      return reinterpret_cast<T*>(reinterpret_cast<std::uint8_t*>(ptr_) +
                                  (region_.ClampY(y) - region_.y1()) * pitch_ +
                                  (region_.ClampX(x) - region_.x1()) * sizeof(T));
    }

    // Advance pointer to next row
    T* GetNextRow(T* ptr) const
    {
      return reinterpret_cast<T*>(reinterpret_cast<std::uint8_t*>(ptr) + pitch_);
    }

    // Const version of row advance
    const T* GetNextRow(const T* ptr) const
    {
      return reinterpret_cast<const T*>(reinterpret_cast<const std::uint8_t*>(ptr) + pitch_);
    }

    // Move pointer back to previous row
    T* GetPreviousRow(T* ptr) const
    {
      return reinterpret_cast<T*>(reinterpret_cast<std::uint8_t*>(ptr) - pitch_);
    }

    // ---- PROPERTY GETTERS ----

    // Get the pitch (bytes per row)
    std::size_t GetPitch() const { return pitch_; }

    // Get the region/bounds of the image
    imgcore::Bounds GetBounds() const { return region_; }

    // Check if the image has allocated memory
    bool IsAllocated() const
    {
      if(ptr_ != nullptr) {
        return true;
      }
      else {
        return false;
      }
    }

    // Get size compatible with Intel IPP
    IppiSize GetSize() const
    {
      IppiSize size = {static_cast<int>(region_.GetWidth()), static_cast<int>(region_.GetHeight())};
      return size;
    }

   private:
    T* ptr_;                  // Pointer to image data
    std::size_t pitch_;       // Bytes per row (including padding)
    imgcore::Bounds region_;  // Region/bounds of the image
  };

  // ========================================================================
  // TYPEDEF FOR FLOAT IMAGES
  // ========================================================================
  typedef ImageBase<float> Image;  // Common alias for float images

  // ========================================================================
  // IMAGELAYER CLASS - Handles multi-channel image (RGB, RGBA, etc.)
  // ========================================================================
  class ImageLayer
  {
   public:
    // Add new channel with specific region
    void AddImage(const imgcore::Bounds& region)
    {
      channels_.push_back(std::shared_ptr<Image>(new Image(region)));
    }

    // Add channel from existing shared_ptr
    void AddImage(const std::shared_ptr<Image>& image_ptr) { channels_.push_back(image_ptr); }

    // Move channel (transfer ownership)
    void MoveImage(const std::shared_ptr<Image>& image_ptr)
    {
      channels_.push_back(boost::move(image_ptr));
    }

    // Operator [] for direct channel access
    Image* operator[](int channel) const { return GetChannel(channel); }

    // Get pointer to specific channel with validation
    Image* GetChannel(int channel) const
    {
      if(static_cast<unsigned int>(channel) > channels_.size() - 1) {
        throw std::runtime_error("imgcore::Image - channel does not exist");
      }
      return channels_[channel].get();
    }

    // Get read-only pixel at coordinate (x,y)
    imgcore::Pixel<const float> GetPixel(int x, int y) const
    {
      imgcore::Pixel<const float> pixel(static_cast<unsigned int>(channels_.size()));

      // Set up pointer for each channel
      for(unsigned int i = 0; i < channels_.size(); ++i) {
        const float* ptr = channels_[i].get()->GetPtr(x, y);
        pixel.SetPtr(ptr, i);
      }
      return pixel;
    }

    // Get writable pixel at coordinate (x,y)
    imgcore::Pixel<float> GetWritePixel(int x, int y) const
    {
      imgcore::Pixel<float> pixel(static_cast<unsigned int>(channels_.size()));

      // Set up writable pointer for each channel
      for(unsigned int i = 0; i < channels_.size(); ++i) {
        float* ptr = channels_[i].get()->GetPtr(x, y);
        pixel.SetPtr(ptr, i);
      }
      return pixel;
    }

    // Return number of channels
    void ChannelCount() const { channels_.size(); }

   private:
    std::vector<std::shared_ptr<Image> > channels_;  // Vector of channels
  };

  // ========================================================================
  // IMAGEARRAY CLASS - Array of images with attribute system
  // ========================================================================
  class ImageArray : public imgcore::Array<ImageBase<float> >
  {
   public:
    // Add new image with specific region
    void Add(const imgcore::Bounds& region) { this->array_.push_back(new Image(region)); }
  };
}  // namespace imgcore

#endif  // IMAGE_H