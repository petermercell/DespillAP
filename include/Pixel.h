#ifndef PIXEL_H
#define PIXEL_H

namespace imgcore
{
  // ========================================================================
  // CLASE PackedPixel - Para navegación eficiente en memoria contigua
  // ========================================================================
  template <class T>
  class PackedPixel
  {
   public:
    // Constructor básico: solo puntero, stride=0 (memoria contigua)
    explicit PackedPixel(T* ptr) : ptr_(ptr), stride_(0) {}

    // Constructor con stride: para saltar elementos (ej: RGB -> solo R)
    PackedPixel(T* ptr, size_t stride) : ptr_(ptr), stride_(stride) {}

    // Constructor de copia: copia solo el puntero (shallow copy)
    PackedPixel(const PackedPixel& other) { ptr_ = other.ptr_; }

    // Operador de asignación
    PackedPixel& operator=(const PackedPixel& other)
    {
      ptr_ = other.ptr_;
      return *this;
    }

    // ---- OPERADORES PARA COMPORTARSE COMO UN PUNTERO ----

    // Desreferencia: obtiene el valor apuntado
    T& operator*() { return *ptr_; }

    // Asignación directa de puntero
    void operator=(T* ptr) { ptr_ = ptr; }

    // Operador flecha: acceso a miembros si T es struct/class
    T* operator->() { return ptr_; }

    // ---- OPERADORES DE INCREMENTO/DECREMENTO CON STRIDE ----

    // Pre-incremento: avanza usando stride y devuelve nuevo puntero
    T* operator++()
    {
      ptr_ += stride_;
      return ptr_;
    }

    // Post-incremento: avanza pero devuelve puntero anterior
    T* operator++(int)
    {
      T* old = ptr_;
      ptr_ += stride_;
      return old;
    }

    // Suma con asignación: avanza N posiciones
    T* operator+=(const size_t& stride)
    {
      T* old = ptr_;
      ptr_ += stride;
      return old;
    }

    // Pre-decremento: retrocede usando stride
    T* operator--()
    {
      ptr_ -= stride_;
      return ptr_;
    }

    // Post-decremento: retrocede pero devuelve puntero anterior
    T* operator--(int)
    {
      T* old = ptr_;
      ptr_ -= stride_;
      return old;
    }

   private:
    T* ptr_;         // Puntero actual a los datos
    size_t stride_;  // Cantidad de elementos a saltar en cada incremento
  };

  // ========================================================================
  // CLASE Pixel - Para manejar múltiples canales de un pixel
  // ========================================================================
  template <class T>
  class Pixel
  {
   public:
    // Constructor por defecto: crea pixel RGB (3 canales)
    Pixel() : pointers_(nullptr) { Allocate_(3); }

    // Constructor con tamaño específico (ej: RGBA=4, Grayscale=1)
    explicit Pixel(unsigned int size) : pointers_(nullptr) { Allocate_(size); }

    // Constructor de copia
    Pixel(const Pixel& other) { CopyPixel_(other); }

    // Operador de asignación
    Pixel& operator=(const Pixel& other)
    {
      CopyPixel_(other);
      return *this;
    }

    // Destructor: libera memoria
    ~Pixel() { Dispose_(); }

    // ---- MÉTODOS PÚBLICOS PARA NAVEGACIÓN Y ACCESO ----

    // Avanza todos los punteros al siguiente pixel
    void NextPixel()
    {
      for(unsigned int i = 0; i < size_; ++i) {
        if(pointers_ != nullptr) {
          pointers_[i]++;  // Incrementa cada canal
        }
      }
    }

    // Post-incremento: llama a NextPixel()
    void operator++(int) { NextPixel(); }

    // Acceso a canal por índice: devuelve referencia al valor
    T& operator[](unsigned int index) { return *pointers_[index]; }

    // Establece puntero para un canal específico
    void SetPtr(T* ptr, unsigned int index) { pointers_[index] = ptr; }

    // Establece valor para un canal específico
    void SetVal(T val, unsigned int index) { *pointers_[index] = val; }

    // Obtiene puntero de un canal
    T* GetPtr(unsigned int index) const { return pointers_[index]; }

    // Obtiene valor de un canal
    T GetVal(unsigned int index) const { return *pointers_[index]; }

    // Obtiene número de canales
    unsigned int GetSize() const { return size_; }

   private:
    T** pointers_;       // Array de punteros (uno por canal)
    unsigned int size_;  // Número de canales

    // ---- MÉTODOS PRIVADOS DE GESTIÓN DE MEMORIA ----

    // Reserva memoria para el array de punteros
    void Allocate_(unsigned int size)
    {
      Dispose_();  // Libera memoria previa
      size_ = size;
      pointers_ = new T*[size_];  // Crea array de punteros
      for(unsigned int x = 0; x < size_; ++x) {
        pointers_[x] = nullptr;  // Inicializa a nullptr
      }
    }

    // Copia desde otro pixel (shallow copy de punteros)
    void CopyPixel_(const Pixel& other)
    {
      Allocate_(other.size_);
      for(unsigned int i = 0; i < size_; ++i) {
        pointers_[i] = other.pointers_[i];  // Copia punteros, no datos
      }
    }

    // Libera memoria del array de punteros
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