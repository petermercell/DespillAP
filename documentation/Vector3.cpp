#include "Vector3.h"

#include <iostream>


using namespace DD::Image;

int main()
{
  // =====================================
  // CREACIÓN Y INICIALIZACIÓN DE VECTORES
  // =====================================

  // Constructor por defecto (0, 0, 0)
  Vector3 vect;

  // Constructor con valores específicos
  Vector3 vect2(1.0f, 2.0f, 3.0f);

  // Constructor desde array
  float valores[] = {4.0f, 5.0f, 6.0f};
  Vector3 vect3(valores);

  // =====================================
  // SETEAR VALORES
  // =====================================

  // Método set() - la forma más directa
  vect.set(10.0f, 20.0f, 30.0f);

  // Acceso directo a componentes
  vect.x = 15.0f;
  vect.y = 25.0f;
  vect.z = 35.0f;

  // Usando el operador []
  vect[0] = 100.0f;  // x
  vect[1] = 200.0f;  // y
  vect[2] = 300.0f;  // z

  // =====================================
  // OBTENER VALORES
  // =====================================

  // Acceso directo
  std::cout << "Vector vect: (" << vect.x << ", " << vect.y << ", " << vect.z << ")\n";

  // Usando operador []
  std::cout << "Componentes: x=" << vect[0] << " y=" << vect[1] << " z=" << vect[2] << "\n";

  // Obtener puntero para OpenGL
  const float* array_ptr = vect.array();
  std::cout << "Array: [" << array_ptr[0] << ", " << array_ptr[1] << ", " << array_ptr[2] << "]\n";

  // =====================================
  // OPERACIONES MATEMÁTICAS BÁSICAS
  // =====================================

  Vector3 a(1.0f, 2.0f, 3.0f);
  Vector3 b(4.0f, 5.0f, 6.0f);

  // Suma
  Vector3 suma = a + b;
  std::cout << "a + b = (" << suma.x << ", " << suma.y << ", " << suma.z << ")\n";

  // Resta
  Vector3 resta = a - b;
  std::cout << "a - b = (" << resta.x << ", " << resta.y << ", " << resta.z << ")\n";

  // Multiplicación por escalar
  Vector3 escalado = a * 2.0f;
  std::cout << "a * 2 = (" << escalado.x << ", " << escalado.y << ", " << escalado.z << ")\n";

  // División por escalar
  Vector3 dividido = a / 2.0f;
  std::cout << "a / 2 = (" << dividido.x << ", " << dividido.y << ", " << dividido.z << ")\n";

  // Multiplicación componente a componente
  Vector3 mult_comp = a * b;
  std::cout << "a * b (componente) = (" << mult_comp.x << ", " << mult_comp.y << ", " << mult_comp.z
            << ")\n";

  // =====================================
  // OPERACIONES GEOMÉTRICAS
  // =====================================

  // Longitud del vector
  float longitud = a.length();
  std::cout << "Longitud de a: " << longitud << "\n";

  // Longitud al cuadrado (más eficiente)
  float long_cuadrado = a.lengthSquared();
  std::cout << "Longitud al cuadrado: " << long_cuadrado << "\n";

  // Producto punto (dot product)
  float dot_product = a.dot(b);
  std::cout << "a · b (dot product): " << dot_product << "\n";

  // Producto cruz (cross product)
  Vector3 cross_product = a.cross(b);
  std::cout << "a × b (cross product): (" << cross_product.x << ", " << cross_product.y << ", "
            << cross_product.z << ")\n";

  // Normalizar vector (convertir a longitud unitaria)
  Vector3 a_copia = a;
  float longitud_original = a_copia.normalize();
  std::cout << "Vector normalizado: (" << a_copia.x << ", " << a_copia.y << ", " << a_copia.z
            << ")\n";
  std::cout << "Longitud original era: " << longitud_original << "\n";

  // Distancia entre dos puntos
  float distancia = a.distanceBetween(b);
  std::cout << "Distancia entre a y b: " << distancia << "\n";

  // =====================================
  // OPERACIONES DE MODIFICACIÓN IN-PLACE
  // =====================================

  Vector3 modificable(1.0f, 2.0f, 3.0f);

  // Suma in-place
  modificable += Vector3(1.0f, 1.0f, 1.0f);
  std::cout << "Después de += (1,1,1): (" << modificable.x << ", " << modificable.y << ", "
            << modificable.z << ")\n";

  // Multiplicación in-place
  modificable *= 2.0f;
  std::cout << "Después de *= 2: (" << modificable.x << ", " << modificable.y << ", "
            << modificable.z << ")\n";

  // Negar el vector
  modificable.negate();
  std::cout << "Después de negate(): (" << modificable.x << ", " << modificable.y << ", "
            << modificable.z << ")\n";

  // =====================================
  // COMPARACIONES
  // =====================================

  Vector3 v1(1.0f, 2.0f, 3.0f);
  Vector3 v2(1.0f, 2.0f, 3.0f);
  Vector3 v3(1.0f, 2.0f, 4.0f);

  if(v1 == v2) {
    std::cout << "v1 es igual a v2\n";
  }

  if(v1 != v3) {
    std::cout << "v1 es diferente a v3\n";
  }

  // =====================================
  // OPERACIONES ÚTILES ADICIONALES
  // =====================================

  // Reflexión respecto a una normal
  Vector3 normal(0.0f, 1.0f, 0.0f);      // Normal hacia arriba
  Vector3 incidente(1.0f, -1.0f, 0.0f);  // Rayo hacia abajo y derecha
  Vector3 reflejado = incidente.reflect(normal);
  std::cout << "Rayo reflejado: (" << reflejado.x << ", " << reflejado.y << ", " << reflejado.z
            << ")\n";

  // Mínimo y máximo componente a componente
  Vector3 min_vec = a.minimum(b);
  Vector3 max_vec = a.maximum(b);
  std::cout << "Mínimo: (" << min_vec.x << ", " << min_vec.y << ", " << min_vec.z << ")\n";
  std::cout << "Máximo: (" << max_vec.x << ", " << max_vec.y << ", " << max_vec.z << ")\n";

  // Distancia desde un plano (Ax + By + Cz + D = 0)
  float dist_plano = a.distanceFromPlane(1.0f, 0.0f, 0.0f, -5.0f);  // Plano x = 5
  std::cout << "Distancia del punto al plano x=5: " << dist_plano << "\n";

  return 0;
}