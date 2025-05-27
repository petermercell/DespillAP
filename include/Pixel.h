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

#ifndef PIXEL_H
#define PIXEL_H

namespace imgcore
{
  // ========================================================================
  // PackedPixel CLASS - For efficient navigation in contiguous memory
  // ========================================================================
  template <class T>
  class PackedPixel
  {
   public:
    // Basic constructor: only pointer, stride=0 (contiguous memory)
    explicit PackedPixel(T* ptr) : ptr_(ptr), stride_(0) {}

    // Constructor with stride: to skip elements (e.g.: RGB -> only R)
    PackedPixel(T* ptr, size_t stride) : ptr_(ptr), stride_(stride) {}

    // Copy constructor: copies only the pointer (shallow copy)
    PackedPixel(const PackedPixel& other) { ptr_ = other.ptr_; }

    // Assignment operator
    PackedPixel& operator=(const PackedPixel& other)
    {
      ptr_ = other.ptr_;
      return *this;
    }

    // ---- OPERATORS TO BEHAVE LIKE A POINTER ----

    // Dereference: get the pointed value
    T& operator*() { return *ptr_; }

    // Direct pointer assignment
    void operator=(T* ptr) { ptr_ = ptr; }

    // Arrow operator: member access if T is struct/class
    T* operator->() { return ptr_; }

    // ---- INCREMENT/DECREMENT OPERATORS WITH STRIDE ----

    // Pre-increment: advance using stride and return new pointer
    T* operator++()
    {
      ptr_ += stride_;
      return ptr_;
    }

    // Post-increment: advance but return previous pointer
    T* operator++(int)
    {
      T* old = ptr_;
      ptr_ += stride_;
      return old;
    }

    // Add assignment: advance N positions
    T* operator+=(const size_t& stride)
    {
      T* old = ptr_;
      ptr_ += stride;
      return old;
    }

    // Pre-decrement: move back using stride
    T* operator--()
    {
      ptr_ -= stride_;
      return ptr_;
    }

    // Post-decrement: move back but return previous pointer
    T* operator--(int)
    {
      T* old = ptr_;
      ptr_ -= stride_;
      return old;
    }

   private:
    T* ptr_;         // Current pointer to data
    size_t stride_;  // Number of elements to skip on each increment
  };

  // ========================================================================
  // Pixel CLASS - For handling multiple channels of a pixel
  // ========================================================================
  template <class T>
  class Pixel
  {
   public:
    // Default constructor: creates RGB pixel (3 channels)
    Pixel() : pointers_(nullptr) { Allocate_(3); }

    // Constructor with specific size (e.g.: RGBA=4, Grayscale=1)
    explicit Pixel(unsigned int size) : pointers_(nullptr) { Allocate_(size); }

    // Copy constructor
    Pixel(const Pixel& other) { CopyPixel_(other); }

    // Assignment operator
    Pixel& operator=(const Pixel& other)
    {
      CopyPixel_(other);
      return *this;
    }

    // Destructor: frees memory
    ~Pixel() { Dispose_(); }

    // ---- PUBLIC METHODS FOR NAVIGATION AND ACCESS ----

    // Advance all pointers to next pixel
    void NextPixel()
    {
      for(unsigned int i = 0; i < size_; ++i) {
        if(pointers_ != nullptr) {
          pointers_[i]++;  // Increment each channel
        }
      }
    }

    // Post-increment: calls NextPixel()
    void operator++(int) { NextPixel(); }

    // Channel access by index: returns reference to value
    T& operator[](unsigned int index) { return *pointers_[index]; }

    // Set pointer for specific channel
    void SetPtr(T* ptr, unsigned int index) { pointers_[index] = ptr; }

    // Set value for specific channel
    void SetVal(T val, unsigned int index) { *pointers_[index] = val; }

    // Get pointer of a channel
    T* GetPtr(unsigned int index) const { return pointers_[index]; }

    // Get value of a channel
    T GetVal(unsigned int index) const { return *pointers_[index]; }

    // Get number of channels
    unsigned int GetSize() const { return size_; }

   private:
    T** pointers_;       // Array of pointers (one per channel)
    unsigned int size_;  // Number of channels

    // ---- PRIVATE MEMORY MANAGEMENT METHODS ----

    // Allocate memory for pointer array
    void Allocate_(unsigned int size)
    {
      Dispose_();  // Free previous memory
      size_ = size;
      pointers_ = new T*[size_];  // Create pointer array
      for(unsigned int x = 0; x < size_; ++x) {
        pointers_[x] = nullptr;  // Initialize to nullptr
      }
    }

    // Copy from another pixel (shallow copy of pointers)
    void CopyPixel_(const Pixel& other)
    {
      Allocate_(other.size_);
      for(unsigned int i = 0; i < size_; ++i) {
        pointers_[i] = other.pointers_[i];  // Copy pointers, not data
      }
    }

    // Free memory of pointer array
    void Dispose_()
    {
      if(pointers_ != nullptr) {
        delete[] pointers_;
        pointers_ = nullptr;
      }
    }
  };

}  // namespace imgcore

#endif  // PIXEL_H