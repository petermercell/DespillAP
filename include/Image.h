#ifndef IMAGE_H
#define IMAGE_H

// ========================================================================
// INCLUDES PARA GESTIÓN DE MEMORIA MULTIPLATAFORMA
// ========================================================================
#ifndef _WIN32
#include <malloc.h>  // Linux/Unix: funciones de memoria alineada
#else
#include <stdlib.h>  // Windows: funciones de memoria estándar
#endif

// ========================================================================
// INCLUDES NECESARIOS
// ========================================================================
#include <boost/ptr_container/ptr_list.hpp>  // Contenedores inteligentes
#include <cstdint>                           // Tipos enteros de tamaño fijo
#include <memory>                            // Smart pointers (shared_ptr)
#include <vector>                            // Contenedor estándar

#include "include/Attribute.h"  // Sistema de atributos
#include "include/Bounds.h"     // Manejo de regiones rectangulares
#include "include/Pixel.h"      // Clases para manejar píxeles
#include "include/Threading.h"  // Utilidades de threading

struct IppiSize
{
  int width;
  int height;
};

namespace imgcore
{
  // ========================================================================
  // CLASE TEMPLATE IMAGEBASE<T> - Imagen genérica con tipo de dato T
  // ========================================================================
  template <class T>
  class ImageBase : public AttributeBase  // Hereda sistema de atributos
  {
   public:
    // ---- CONSTRUCTORES ----

    // Constructor por defecto: imagen vacía
    ImageBase<T>() : ptr_(nullptr), pitch_(0), region_(imgcore::Bounds()) {}

    // Constructor con región específica
    explicit ImageBase<T>(const imgcore::Bounds& region)
        : ptr_(nullptr), pitch_(0)
    {
      Allocate(region);  // Reserva memoria para la región
    }

    // Constructor con ancho y alto
    ImageBase<T>(unsigned int width, unsigned int height)
        : ptr_(nullptr), pitch_(0)
    {
      Allocate(width, height);  // Crea región desde dimensiones
    }

    // Constructor de copia: crea copia profunda
    ImageBase<T>(const ImageBase<T>& other) : ptr_(nullptr), pitch_(0)
    {
      Copy(other);  // Copia todos los datos
    }

    // Constructor de movimiento: transfiere ownership
    ImageBase<T>(ImageBase<T>&& other)
        : ptr_(other.ptr_), pitch_(other.pitch_), region_(other.region_)
    {
      // Resetea el objeto origen para evitar double-delete
      other.ptr_ = nullptr;
      other.pitch_ = 0;
      other.region_ = imgcore::Bounds();
    }

    // ---- OPERADORES DE ASIGNACIÓN ----

    // Asignación por copia
    ImageBase<T>& operator=(const ImageBase<T>& other)
    {
      Copy(other);
      return *this;
    }

    // Asignación por movimiento
    ImageBase<T>& operator=(ImageBase<T>&& other)
    {
      if(this != &other) {  // Evita auto-asignación
        Deallocate();       // Libera memoria actual
        // Transfiere ownership
        ptr_ = other.ptr_;
        pitch_ = other.pitch_;
        region_ = other.region_;
        // Resetea origen
        other.ptr_ = nullptr;
        other.pitch_ = 0;
        other.region_ = imgcore::Bounds();
      }
      return *this;
    }

    // Destructor: libera memoria automáticamente
    ~ImageBase<T>() { Deallocate(); }

    // ---- GESTIÓN DE MEMORIA ----

    // Reserva memoria para imagen de width x height
    void Allocate(unsigned int width, unsigned int height)
    {
      Allocate(imgcore::Bounds(0, 0, width - 1, height - 1));
    }

    // Reserva memoria para región específica
    void Allocate(const imgcore::Bounds& region)
    {
      Deallocate();  // Libera memoria previa
      region_ = region;

      // Calcula pitch alineado a 64 bytes para optimización SIMD
      pitch_ = (((region_.GetWidth() * sizeof(T) + 63) / 64) * 64);

#ifdef _WIN32
      // Windows: usa _aligned_malloc para alineación a 64 bytes
      ptr_ = reinterpret_cast<T*>(
          _aligned_malloc(64, pitch_ * region_.GetHeight()));
#else
      // Linux/Unix: usa aligned_alloc estándar
      ptr_ =
          reinterpret_cast<T*>(aligned_alloc(64, pitch_ * region_.GetHeight()));
#endif
    }

    // Copia profunda desde otra imagen
    void Copy(const ImageBase<T>& other)
    {
      region_ = other.region_;
      Allocate(region_);  // Reserva misma región

      // Copia fila por fila para manejar diferentes pitch
      for(int y = region_.y1(); y <= region_.y2(); ++y) {
        memcpy(GetPtr(region_.x1(), y), other.GetPtr(other.region_.x1(), y),
               region_.GetWidth() * sizeof(T));
      }
    }

    // Libera memoria reservada
    void Deallocate()
    {
      if(ptr_ != nullptr) {
        free(ptr_);
        ptr_ = nullptr;
      }
    }

    // ---- FUNCIONES DE COPIA DE MEMORIA ----

    // Copia datos HACIA esta imagen desde puntero externo
    void MemCpyIn(const T* ptr, std::size_t pitch)
    {
      const T* source_ptr = ptr;
      T* dest_ptr = GetPtr(region_.x1(), region_.y1());
      std::size_t size = region_.GetWidth() * sizeof(T);

      // Copia fila por fila manejando diferentes pitch
      for(int y = region_.y1(); y <= region_.y2(); ++y) {
        memcpy(dest_ptr, source_ptr, size);
        // Avanza source_ptr usando su pitch
        source_ptr = reinterpret_cast<const T*>(
            (reinterpret_cast<const std::uint8_t*>(source_ptr) + pitch));
        // Avanza dest_ptr usando nuestro pitch
        dest_ptr = this->GetNextRow(dest_ptr);
      }
    }

    // Copia datos hacia región específica
    void MemCpyIn(const T* ptr, std::size_t pitch, imgcore::Bounds region)
    {
      const T* source_ptr = ptr;
      T* dest_ptr = GetPtr(region.x1(), region.y1());
      std::size_t size = region.GetWidth() * sizeof(T);

      for(int y = region.y1(); y <= region.y2(); ++y) {
        memcpy(dest_ptr, source_ptr, size);
        source_ptr = reinterpret_cast<const T*>(
            (reinterpret_cast<const std::uint8_t*>(source_ptr) + pitch));
        dest_ptr = this->GetNextRow(dest_ptr);
      }
    }

    // Copia desde otra imagen con región específica
    void MemCpyIn(const ImageBase<T>& source_image, imgcore::Bounds region)
    {
      MemCpyIn(source_image.GetPtr(region.x1(), region.y1()),
               source_image.GetPitch(), region);
    }

    // Copia desde otra imagen (región completa)
    void MemCpyIn(const ImageBase<T>& source_image)
    {
      // Calcula intersección entre imágenes para evitar overflow
      imgcore::Bounds region =
          source_image.GetBounds().GetIntersection(region_);
      MemCpyIn(source_image.GetPtr(region.x1(), region.y1()),
               source_image.GetPitch(), region);
    }

    // ---- FUNCIONES DE COPIA HACIA AFUERA ----

    // Copia DESDE esta imagen hacia puntero externo
    void MemCpyOut(T* ptr, std::size_t pitch) const
    {
      T* source_ptr = GetPtr(region_.x1(), region_.y1());
      T* dest_ptr = ptr;
      std::size_t size = region_.GetWidth() * sizeof(T);

      for(int y = region_.y1(); y <= region_.y2(); ++y) {
        memcpy(dest_ptr, source_ptr, size);
        source_ptr = this->GetNextRow(dest_ptr);
        dest_ptr = reinterpret_cast<T*>(
            (reinterpret_cast<std::uint8_t*>(dest_ptr) + pitch));
      }
    }

    // Copia región específica hacia puntero externo
    void MemCpyOut(T* ptr, std::size_t pitch, imgcore::Bounds region) const
    {
      T* source_ptr = GetPtr(region.x1(), region.y1());
      T* dest_ptr = ptr;
      std::size_t size = region.GetWidth() * sizeof(T);

      for(int y = region.y1(); y <= region.y2(); ++y) {
        memcpy(dest_ptr, source_ptr, size);
        source_ptr = this->GetNextRow(dest_ptr);
        dest_ptr = reinterpret_cast<T*>(
            (reinterpret_cast<std::uint8_t*>(dest_ptr) + pitch));
      }
    }

    // Copia hacia otra imagen con región específica
    void MemCpyOut(const ImageBase<T>& dest_image, imgcore::Bounds region) const
    {
      MemCpyOut(dest_image.GetPtr(region.x1(), region.y1()),
                dest_image.GetPitch(), region);
    }

    // Copia hacia otra imagen (región completa)
    void MemCpyOut(const ImageBase<T>& dest_image) const
    {
      imgcore::Bounds region = dest_image.GetBounds();
      MemCpyOut(dest_image.GetPtr(region.x1(), region.y1()),
                dest_image.GetPitch(), region);
    }

    // ---- ACCESO A PUNTEROS Y NAVEGACIÓN ----

    // Obtiene puntero base de la imagen
    T* GetPtr() const { return ptr_; }

    // Obtiene puntero a coordenada específica (x,y)
    T* GetPtr(int x, int y) const
    {
      return reinterpret_cast<T*>(reinterpret_cast<std::uint8_t*>(ptr_) +
                                  (y - region_.y1()) * pitch_ +  // Offset fila
                                  (x - region_.x1()) *
                                      sizeof(T));  // Offset columna
    }

    // Obtiene puntero con clampeo automático a bounds
    T* GetPtrBnds(int x, int y) const
    {
      return reinterpret_cast<T*>(reinterpret_cast<std::uint8_t*>(ptr_) +
                                  (region_.ClampY(y) - region_.y1()) * pitch_ +
                                  (region_.ClampX(x) - region_.x1()) *
                                      sizeof(T));
    }

    // Avanza puntero a la siguiente fila
    T* GetNextRow(T* ptr) const
    {
      return reinterpret_cast<T*>(reinterpret_cast<std::uint8_t*>(ptr) +
                                  pitch_);
    }

    // Versión const del avance de fila
    const T* GetNextRow(const T* ptr) const
    {
      return reinterpret_cast<const T*>(
          reinterpret_cast<const std::uint8_t*>(ptr) + pitch_);
    }

    // Retrocede puntero a la fila anterior
    T* GetPreviousRow(T* ptr) const
    {
      return reinterpret_cast<T*>(reinterpret_cast<std::uint8_t*>(ptr) -
                                  pitch_);
    }

    // ---- GETTERS DE PROPIEDADES ----

    // Obtiene el pitch (bytes por fila)
    std::size_t GetPitch() const { return pitch_; }

    // Obtiene la región/bounds de la imagen
    imgcore::Bounds GetBounds() const { return region_; }

    // Verifica si la imagen tiene memoria asignada
    bool IsAllocated() const
    {
      if(ptr_ != nullptr) {
        return true;
      }
      else {
        return false;
      }
    }

    // Obtiene tamaño compatible con Intel IPP
    IppiSize GetSize() const
    {
      IppiSize size = {static_cast<int>(region_.GetWidth()),
                       static_cast<int>(region_.GetHeight())};
      return size;
    }

   private:
    T* ptr_;                  // Puntero a los datos de la imagen
    std::size_t pitch_;       // Bytes por fila (incluyendo padding)
    imgcore::Bounds region_;  // Región/límites de la imagen
  };

  // ========================================================================
  // TYPEDEF PARA IMAGEN DE FLOTANTES
  // ========================================================================
  typedef ImageBase<float> Image;  // Alias común para imágenes de float

  // ========================================================================
  // CLASE IMAGELAYER - Maneja imagen multi-canal (RGB, RGBA, etc.)
  // ========================================================================
  class ImageLayer
  {
   public:
    // Añade nuevo canal con región específica
    void AddImage(const imgcore::Bounds& region)
    {
      channels_.push_back(std::shared_ptr<Image>(new Image(region)));
    }

    // Añade canal desde shared_ptr existente
    void AddImage(const std::shared_ptr<Image>& image_ptr)
    {
      channels_.push_back(image_ptr);
    }

    // Mueve canal (transfer ownership)
    void MoveImage(const std::shared_ptr<Image>& image_ptr)
    {
      channels_.push_back(boost::move(image_ptr));
    }

    // Operador [] para acceso directo a canal
    Image* operator[](int channel) const { return GetChannel(channel); }

    // Obtiene puntero a canal específico con validación
    Image* GetChannel(int channel) const
    {
      if(static_cast<unsigned int>(channel) > channels_.size() - 1) {
        throw std::runtime_error("imgcore::Image - channel does not exist");
      }
      return channels_[channel].get();
    }

    // Obtiene pixel de solo lectura en coordenada (x,y)
    imgcore::Pixel<const float> GetPixel(int x, int y) const
    {
      imgcore::Pixel<const float> pixel(
          static_cast<unsigned int>(channels_.size()));

      // Configura puntero para cada canal
      for(unsigned int i = 0; i < channels_.size(); ++i) {
        const float* ptr = channels_[i].get()->GetPtr(x, y);
        pixel.SetPtr(ptr, i);
      }
      return pixel;
    }

    // Obtiene pixel escribible en coordenada (x,y)
    imgcore::Pixel<float> GetWritePixel(int x, int y) const
    {
      imgcore::Pixel<float> pixel(static_cast<unsigned int>(channels_.size()));

      // Configura puntero escribible para cada canal
      for(unsigned int i = 0; i < channels_.size(); ++i) {
        float* ptr = channels_[i].get()->GetPtr(x, y);
        pixel.SetPtr(ptr, i);
      }
      return pixel;
    }

    // Retorna número de canales
    void ChannelCount() const { channels_.size(); }

   private:
    std::vector<std::shared_ptr<Image> > channels_;  // Vector de canales
  };

  // ========================================================================
  // CLASE IMAGEARRAY - Array de imágenes con sistema de atributos
  // ========================================================================
  class ImageArray : public imgcore::Array<ImageBase<float> >
  {
   public:
    // Añade nueva imagen con región específica
    void Add(const imgcore::Bounds& region)
    {
      this->array_.push_back(new Image(region));
    }
  };
}  // namespace imgcore

#endif  // IMAGE_H