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

#ifndef ATTRIBUTE_H
#define ATTRIBUTE_H

#include <boost/ptr_container/ptr_list.hpp>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

// ========================================================================
// DLL EXPORT CONFIGURATION (WINDOWS ONLY)
// ========================================================================
#ifdef _WIN32
#ifdef _MSC_VER
#pragma warning(disable : 4251)  // Disable warning about DLL export
#endif
#define WINLIB_EXPORT __declspec(dllexport)  // Macro to export from DLL
#else
#define WINLIB_EXPORT  // On other systems, empty macro
#endif

namespace imgcore
{

  // ========================================================================
  // ATTRIBUTE STRUCTURE - Simple name-value pair
  // ========================================================================
  struct Attribute
  {
    std::string name;  // Attribute name (e.g.: "thread_id", "channel")
    int value;         // Associated integer value

    // Constructor that initializes both fields
    Attribute(std::string name, int value) : name(name), value(value) {}
  };

  // ========================================================================
  // ATTRIBUTEBASE BASE CLASS - Manages collection of attributes
  // ========================================================================
  class WINLIB_EXPORT AttributeBase
  {
   public:
    // Adds a single attribute by name and value
    void AddAttribute(const std::string& name, int value)
    {
      attributes_.push_back(Attribute(name, value));
    }

    // Adds multiple attributes at once
    void AddAttributes(std::vector<Attribute> attributes)
    {
      // Inserts all attributes at the end of the existing vector
      attributes_.insert(attributes_.end(), attributes.begin(), attributes.end());
    }

    // Searches for an attribute by name and returns its value
    int GetAttribute(const std::string& name) const
    {
      // Search using lambda: compares the name of each attribute
      auto it = std::find_if(attributes_.begin(), attributes_.end(), [name](const Attribute& attr) {
        return attr.name.compare(name) == 0;
      });

      if(it != attributes_.end()) {
        return it->value;  // Found: return the value
      }
      else {
        // Not found: throw exception with descriptive message
        throw std::out_of_range(std::string("No attribute named ") + name);
      }
    }

   protected:
    std::vector<Attribute> attributes_;  // Stores the collection of attributes
  };

  // ========================================================================
  // ARRAY TEMPLATE CLASS - Container for objects with attributes
  // ========================================================================
  template <typename T>
  class Array
  {
   public:
    // ---- ITERATOR TYPE DEFINITIONS ----
    typedef typename boost::ptr_list<T>::iterator ptr_list_it;
    typedef typename boost::ptr_list<T>::reverse_iterator ptr_list_rit;

    // ---- BASIC CONTAINER MANAGEMENT ----

    // Adds a new T object to the end of the array
    virtual void Add() { array_.push_back(new T()); }

    // Clears the entire container
    void Clear() { array_.clear(); }

    // Gets pointer to the last element
    T* GetBackPtr() { return &array_.back(); }

    // ---- SEARCH BY ATTRIBUTES ----

    // Searches for object by a single attribute (name + value)
    T* GetPtrByAttribute(const std::string& name, int value)
    {
      ptr_list_it it;
      // Iterate through all elements in the array
      for(it = array_.begin(); it != array_.end(); ++it) {
        // Compare the current element's attribute with the searched one
        if(it->GetAttribute(name) == value) {
          break;
        }
      }

      if(it != array_.end()) {
        return &(*it);  // Found: return pointer to object
      }
      else {
        // Not found: create error message and throw exception
        std::stringstream ss;
        ss << "imgcore::Array - no image with attribute '" << name << "' = " << value;
        throw std::out_of_range(ss.str());
      }
    }

    // Searches for object that matches ALL attributes in the list
    T* GetPtrByAttributes(std::vector<Attribute> list)
    {
      ptr_list_it it;
      // Iterate through all elements in the array
      for(it = array_.begin(); it != array_.end(); ++it) {
        unsigned int num_found = 0;

        // For each element, check how many attributes match
        for(auto a_it = list.begin(); a_it != list.end(); ++a_it) {
          if(it->GetAttribute(a_it->name) == a_it->value) {
            num_found++;  // Match counter
          }
        }

        // If ALL attributes match, we found the object
        if(num_found == list.size()) {
          break;
        }
      }

      if(it != array_.end()) {
        return &(*it);  // Found
      }
      else {
        // Not found: build detailed message with all searched attributes
        std::stringstream ss;
        ss << "imgcore::Array - No image with attributes: ";
        for(auto a_it = list.begin(); a_it != list.end(); ++a_it) {
          ss << "'" << a_it->name << "'' = " << a_it->value << "'";
          if(a_it + 1 != list.end()) {
            ss << ", ";
          }  // Separator between attributes
        }
        throw std::out_of_range(ss.str());
      }
    }

    // ---- EXISTENCE CHECKING (WITHOUT THROWING EXCEPTIONS) ----

    // Checks if an object with the specified attribute exists
    bool HasAttribute(const std::string& name, int value)
    {
      bool found = false;
      for(ptr_list_it it = array_.begin(); it != array_.end(); ++it) {
        if(it->GetAttribute(name) == value) {
          found = true;
          break;  // Stop as soon as one is found
        }
      }
      return found;
    }

    // Checks if an object with ALL specified attributes exists
    bool HasAttributes(std::vector<Attribute> list)
    {
      bool found = false;
      for(ptr_list_it it = array_.begin(); it != array_.end(); ++it) {
        unsigned int num_found = 0;

        // Count matches (same algorithm as GetPtrByAttributes)
        for(auto a_it = list.begin(); a_it != list.end(); ++a_it) {
          if(it->GetAttribute(a_it->name) == a_it->value) {
            num_found++;
          }
        }

        if(num_found == list.size()) {
          found = true;
          break;
        }
      }
      return found;
    }

    // ---- ITERATOR ACCESS FOR MANUAL TRAVERSAL ----
    ptr_list_it GetBegin() { return array_.begin(); }     // Iterator to beginning
    ptr_list_it GetEnd() { return array_.end(); }         // Iterator to end
    ptr_list_rit GetRBegin() { return array_.rbegin(); }  // Reverse iterator to beginning
    ptr_list_rit GetREnd() { return array_.rend(); }      // Reverse iterator to end

   private:
    // ---- COMPILE-TIME VALIDATION ----
    // Ensures that T inherits from AttributeBase (otherwise, compilation error)
    static_assert(std::is_base_of<AttributeBase, T>::value, "T must inherit from AttributeBase");

   protected:
    boost::ptr_list<T> array_;  // Container that automatically manages memory
  };

}  // namespace imgcore

#endif  // ATTRIBUTE_H_