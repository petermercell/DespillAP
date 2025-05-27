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

#ifndef BOUNDS_H
#define BOUNDS_H

#include <algorithm>

namespace imgcore
{
  namespace math
  {
    // Template function to limit a value between a minimum and maximum
    template <typename T>
    T Clamp(T value, T min_v, T max_v)
    {
      return value >= min_v ? value <= max_v ? value : max_v : min_v;
    }
  }  // namespace math

  class Bounds
  {
   private:
    int x1_;
    int y1_;
    int x2_;
    int y2_;

   public:
    // Creates an empty rectangle
    Bounds() : x1_(0), y1_(0), x2_(0), y2_(0) {}
    // Creates a rectangle with specific coordinates
    Bounds(int x1, int y1, int x2, int y2) : x1_(x1), y1_(y1), x2_(x2), y2_(y2) {}
    // Creates rectangle coordinates from width and height
    Bounds(unsigned int width, unsigned int height)
    {
      x1_ = 0;
      y1_ = 0;
      x2_ = width > 0 ? width - 1 : 0;
      y2_ = height > 0 ? height - 1 : 0;
    }

    // Compares if two rectangles are equal
    bool operator==(const Bounds& other)
    {
      if(other.x1_ == x1_ && other.x2_ == x2_ && other.y1_ == y1_ && other.y2_ == y2_) {
        return true;
      }
      else {
        return false;
      }
    }
    // Compares if two rectangles are different
    bool operator!=(const Bounds& other)
    {
      if(other.x1_ != x1_ || other.x2_ != x2_ || other.y1_ != y1_ || other.y2_ != y2_) {
        return true;
      }
      else {
        return false;
      }
    }

    // Getters: Get the coordinates
    int x1() const { return x1_; }
    int x2() const { return x2_; }
    int y1() const { return y1_; }
    int y2() const { return y2_; }

    // Getters: Get modifiable references to the coordinates
    int& x1Ref() { return x1_; }
    int& x2Ref() { return x2_; }
    int& y1Ref() { return y1_; }
    int& y2Ref() { return y2_; }

    // Setters: Set the coordinates
    void SetX(int x1, int x2)
    {
      x1_ = x1;
      x2_ = x2;
    }

    void SetY(int y1, int y2)
    {
      y1_ = y1;
      y2_ = y2;
    }

    void SetX1(int x) { x1_ = x; }
    void SetX2(int x) { x2_ = x; }
    void SetY1(int y) { y1_ = y; }
    void SetY2(int y) { y2_ = y; }

    // Sets the entire rectangle at once
    void SetBounds(int x1, int y1, int x2, int y2)
    {
      x1_ = x1;
      x2_ = x2;
      y1_ = y1;
      y2_ = y2;
    }

    // Expands the rectangle in all directions
    void PadBounds(unsigned int size)
    {
      x1_ -= size;
      y1_ -= size;
      x2_ += size;
      y2_ += size;
    }

    // Shrinks the rectangle in all directions
    void ErodeBounds(unsigned int size)
    {
      unsigned int max_x_size = (GetWidth() + 1) / 2 - 1;
      unsigned int max_y_size = (GetHeight() + 1) / 2 - 1;
      x1_ += size <= max_x_size ? size : max_x_size;
      y1_ += size <= max_x_size ? size : max_x_size;
      x2_ -= size <= max_y_size ? size : max_y_size;
      y2_ -= size <= max_y_size ? size : max_y_size;
    }

    // Expands the rectangle with different values for X and Y
    void PadBounds(unsigned int x, unsigned int y)
    {
      x1_ -= x;
      y1_ -= y;
      x2_ += x;
      y2_ += y;
    }

    // Modifies the rectangle to be the intersection with another
    void Intersect(const Bounds& other)
    {
      x1_ = x1_ < other.x1_ ? other.x1_ : x1_;
      y1_ = y1_ < other.y1_ ? other.y1_ : y1_;
      x2_ = x2_ > other.x2_ ? other.x2_ : x2_;
      y2_ = y2_ > other.y2_ ? other.y2_ : y2_;
    }

    // Checks if two rectangles overlap
    bool Intersects(const Bounds& other)
    {
      if(other.x2_ < x1_) {
        return false;
      }
      if(other.y2_ < y1_) {
        return false;
      }
      if(other.x1_ > x2_) {
        return false;
      }
      if(other.y1_ > y2_) {
        return false;
      }
      return true;
    }

    // Returns a new rectangle with the intersection
    Bounds GetIntersection(const Bounds& other) const
    {
      imgcore::Bounds new_bounds = *this;
      new_bounds.Intersect(other);
      return new_bounds;
    }

    // Returns a new expanded rectangle
    Bounds GetPadBounds(unsigned int size) const
    {
      Bounds padded = *this;
      padded.PadBounds(size);
      return padded;
    }
    Bounds GetPadBounds(unsigned int x, unsigned int y) const
    {
      Bounds padded = *this;
      padded.PadBounds(x, y);
      return padded;
    }

    // Return width and height
    unsigned int GetWidth() const { return x2_ - x1_ + 1; }
    unsigned int GetHeight() const { return y2_ - y1_ + 1; }

    // Checks if a point is inside the rectangle
    bool WithinBounds(int x, int y) const
    {
      if(x < x1_ || x > x2_ || y < y1_ || y > y2_) {
        return false;
      }
      else {
        return true;
      }
    }

    // Checks if another rectangle is completely inside
    bool WithinBounds(const Bounds& other) const
    {
      if(other.x1_ < x1_ || other.x2_ > x2_ || other.y1_ < y1_ || other.y2_ > y2_) {
        return false;
      }
      else {
        return true;
      }
    }

    // Check only one dimension
    bool WithinBoundsX(int x) const
    {
      if(x < x1_ || x > x2_) {
        return false;
      }
      else {
        return true;
      }
    }
    bool WithinBoundsY(int y) const
    {
      if(y < y1_ || y > y2_) {
        return false;
      }
      else {
        return true;
      }
    }

    // Calculates the center of the rectangle
    float GetCenterX() const { return static_cast<float>(x2_ - x1_) / 2.0f + x1_; }
    float GetCenterY() const { return static_cast<float>(y2_ - y1_) / 2.0f + y1_; }

    // Limit a value to be within the rectangle
    int ClampX(int x) const { return x > x1_ ? (x < x2_ ? x : x2_) : x1_; }
    int ClampY(int y) const { return y > y1_ ? (y < y2_ ? y : y2_) : y1_; }
  };

}  // namespace imgcore

#endif  // BOUNDS_H