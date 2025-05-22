#ifndef ATTRIBUTE_H
#define ATTRIBUTE_H

#include <boost/ptr_container/ptr_list.hpp>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

// ========================================================================
// CONFIGURACIÓN DE EXPORTACIÓN DE DLL (SOLO WINDOWS)
// ========================================================================
#ifdef _WIN32
#ifdef _MSC_VER
#pragma warning(disable : 4251)  // Deshabilita warning sobre DLL export
#endif
#define WINLIB_EXPORT __declspec(dllexport)  // Macro para exportar desde DLL
#else
#define WINLIB_EXPORT  // En otros sistemas, macro vacía
#endif

namespace imgcore
{

  // ========================================================================
  // ESTRUCTURA ATTRIBUTE - Par nombre-valor simple
  // ========================================================================
  struct Attribute
  {
    std::string name;  // Nombre del atributo (ej: "thread_id", "channel")
    int value;         // Valor entero asociado

    // Constructor que inicializa ambos campos
    Attribute(std::string name, int value) : name(name), value(value) {}
  };

  // ========================================================================
  // CLASE BASE ATTRIBUTEBASE - Gestiona colección de atributos
  // ========================================================================
  class WINLIB_EXPORT AttributeBase
  {
   public:
    // Añade un solo atributo por nombre y valor
    void AddAttribute(const std::string& name, int value)
    {
      attributes_.push_back(Attribute(name, value));
    }

    // Añade múltiples atributos de una vez
    void AddAttributes(std::vector<Attribute> attributes)
    {
      // Inserta todos los atributos al final del vector existente
      attributes_.insert(attributes_.end(), attributes.begin(),
                         attributes.end());
    }

    // Busca un atributo por nombre y devuelve su valor
    int GetAttribute(const std::string& name) const
    {
      // Busca usando lambda: compara el nombre de cada atributo
      auto it = std::find_if(attributes_.begin(), attributes_.end(),
                             [name](const Attribute& attr) {
                               return attr.name.compare(name) == 0;
                             });

      if(it != attributes_.end()) {
        return it->value;  // Encontrado: devuelve el valor
      }
      else {
        // No encontrado: lanza excepción con mensaje descriptivo
        throw std::out_of_range(std::string("No attribute named ") + name);
      }
    }

   protected:
    std::vector<Attribute> attributes_;  // Almacena la colección de atributos
  };

  // ========================================================================
  // CLASE TEMPLATE ARRAY - Contenedor de objetos con atributos
  // ========================================================================
  template <typename T>
  class Array
  {
   public:
    // ---- DEFINICIÓN DE TIPOS PARA ITERADORES ----
    typedef typename boost::ptr_list<T>::iterator ptr_list_it;
    typedef typename boost::ptr_list<T>::reverse_iterator ptr_list_rit;

    // ---- GESTIÓN BÁSICA DEL CONTENEDOR ----

    // Añade un nuevo objeto T al final del array
    virtual void Add() { array_.push_back(new T()); }

    // Limpia todo el contenedor
    void Clear() { array_.clear(); }

    // Obtiene puntero al último elemento
    T* GetBackPtr() { return &array_.back(); }

    // ---- BÚSQUEDA POR ATRIBUTOS ----

    // Busca objeto por un solo atributo (nombre + valor)
    T* GetPtrByAttribute(const std::string& name, int value)
    {
      ptr_list_it it;
      // Recorre todos los elementos del array
      for(it = array_.begin(); it != array_.end(); ++it) {
        // Compara el atributo del elemento actual con el buscado
        if(it->GetAttribute(name) == value) {
          break;
        }
      }

      if(it != array_.end()) {
        return &(*it);  // Encontrado: devuelve puntero al objeto
      }
      else {
        // No encontrado: crea mensaje de error y lanza excepción
        std::stringstream ss;
        ss << "imgcore::Array - no image with attribute '" << name
           << "' = " << value;
        throw std::out_of_range(ss.str());
      }
    }

    // Busca objeto que coincida con TODOS los atributos de la lista
    T* GetPtrByAttributes(std::vector<Attribute> list)
    {
      ptr_list_it it;
      // Recorre todos los elementos del array
      for(it = array_.begin(); it != array_.end(); ++it) {
        unsigned int num_found = 0;

        // Para cada elemento, verifica cuántos atributos coinciden
        for(auto a_it = list.begin(); a_it != list.end(); ++a_it) {
          if(it->GetAttribute(a_it->name) == a_it->value) {
            num_found++;  // Contador de coincidencias
          }
        }

        // Si coinciden TODOS los atributos, encontramos el objeto
        if(num_found == list.size()) {
          break;
        }
      }

      if(it != array_.end()) {
        return &(*it);  // Encontrado
      }
      else {
        // No encontrado: construye mensaje detallado con todos los atributos buscados
        std::stringstream ss;
        ss << "imgcore::Array - No image with attributes: ";
        for(auto a_it = list.begin(); a_it != list.end(); ++a_it) {
          ss << "'" << a_it->name << "'' = " << a_it->value << "'";
          if(a_it + 1 != list.end()) {
            ss << ", ";
          }  // Separador entre atributos
        }
        throw std::out_of_range(ss.str());
      }
    }

    // ---- VERIFICACIÓN DE EXISTENCIA (SIN LANZAR EXCEPCIONES) ----

    // Verifica si existe un objeto con el atributo especificado
    bool HasAttribute(const std::string& name, int value)
    {
      bool found = false;
      for(ptr_list_it it = array_.begin(); it != array_.end(); ++it) {
        if(it->GetAttribute(name) == value) {
          found = true;
          break;  // Termina en cuanto encuentra uno
        }
      }
      return found;
    }

    // Verifica si existe un objeto con TODOS los atributos especificados
    bool HasAttributes(std::vector<Attribute> list)
    {
      bool found = false;
      for(ptr_list_it it = array_.begin(); it != array_.end(); ++it) {
        unsigned int num_found = 0;

        // Cuenta coincidencias (mismo algoritmo que GetPtrByAttributes)
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

    // ---- ACCESO A ITERADORES PARA RECORRIDO MANUAL ----
    ptr_list_it GetBegin() { return array_.begin(); }  // Iterador al inicio
    ptr_list_it GetEnd() { return array_.end(); }      // Iterador al final
    ptr_list_rit GetRBegin()
    {
      return array_.rbegin();
    }  // Iterador reverso al inicio
    ptr_list_rit GetREnd()
    {
      return array_.rend();
    }  // Iterador reverso al final

   private:
    // ---- VALIDACIÓN EN TIEMPO DE COMPILACIÓN ----
    // Asegura que T herede de AttributeBase (sino, error de compilación)
    static_assert(std::is_base_of<AttributeBase, T>::value,
                  "T must inherit from AttributeBase");

   protected:
    boost::ptr_list<T>
        array_;  // Contenedor que maneja automáticamente la memoria
  };

}  // namespace imgcore

#endif  // ATTRIBUTE_H_