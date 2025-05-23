#include "DDMath.h"

#include <iostream>

#include "Vector3.h"

using namespace DD::Image;

int main()
{
  // =====================================
  // CONSTANTES MATEMÁTICAS
  // =====================================

  std::cout << "=== CONSTANTES MATEMÁTICAS ===\n";
  std::cout << "PI: " << M_PI_F << "\n";
  std::cout << "PI/2: " << M_PI_2_F << "\n";
  std::cout << "PI/4: " << M_PI_4_F << "\n";
  std::cout << "1/PI: " << M_1_PI_F << "\n";
  std::cout << "2/PI: " << M_2_PI_F << "\n";
  std::cout << "SQRT(2): " << M_SQRT2_F << "\n";
  std::cout << "1/SQRT(2): " << M_SQRT1_2_F << "\n\n";

  // =====================================
  // CONVERSIONES ANGULARES
  // =====================================

  std::cout << "=== CONVERSIONES ANGULARES ===\n";
  float grados = 90.0f;
  float rad = radiansf(grados);
  float volver_grados = degreesf(rad);

  std::cout << grados << " grados = " << rad << " radianes\n";
  std::cout << rad << " radianes = " << volver_grados << " grados\n";

  // Ejemplo práctico con vectores
  Vector3 vector_rotado;
  float angulo = radiansf(45.0f);  // 45 grados en radianes
  vector_rotado.set(cosf(angulo), sinf(angulo), 0.0f);
  std::cout << "Vector a 45°: (" << vector_rotado.x << ", " << vector_rotado.y << ", "
            << vector_rotado.z << ")\n\n";

  // =====================================
  // FUNCIONES MIN/MAX/CLAMP
  // =====================================

  std::cout << "=== FUNCIONES MIN/MAX/CLAMP ===\n";

  float a = 5.5f, b = 3.2f, c = 8.1f;
  std::cout << "a=" << a << ", b=" << b << ", c=" << c << "\n";
  std::cout << "MIN(a,b): " << MIN(a, b) << "\n";
  std::cout << "MAX(a,b): " << MAX(a, b) << "\n";
  std::cout << "MAX(MIN(a,b), c): " << MAX(MIN(a, b), c) << "\n";

  // Clamp - restringir a un rango
  float valor = 15.0f;
  float minimo = 0.0f, maximo = 10.0f;
  float restringido = clamp(valor, minimo, maximo);
  std::cout << "clamp(" << valor << ", " << minimo << ", " << maximo << ") = " << restringido
            << "\n";

  // Clamp a [0,1] - muy común en gráficos
  float valores[] = {-0.5f, 0.3f, 0.8f, 1.5f};
  std::cout << "Clamp a [0,1]: ";
  for(int i = 0; i < 4; i++) {
    std::cout << valores[i] << "->" << clamp(valores[i]) << " ";
  }
  std::cout << "\n";

  // Clamp aplicado a vectores (componente por componente)
  Vector3 vect_original(2.5f, -1.0f, 0.5f);
  Vector3 vect_clamped(clamp(vect_original.x, 0.0f, 1.0f), clamp(vect_original.y, 0.0f, 1.0f),
                       clamp(vect_original.z, 0.0f, 1.0f));
  std::cout << "Vector original: (" << vect_original.x << ", " << vect_original.y << ", "
            << vect_original.z << ")\n";
  std::cout << "Vector clamped [0,1]: (" << vect_clamped.x << ", " << vect_clamped.y << ", "
            << vect_clamped.z << ")\n\n";

  // =====================================
  // FUNCIONES DE INTERPOLACIÓN
  // =====================================

  std::cout << "=== FUNCIONES DE INTERPOLACIÓN ===\n";

  // Interpolación lineal (lerp)
  float inicio = 10.0f, fin = 20.0f;
  for(int i = 0; i <= 5; i++) {
    float t = i / 5.0f;  // 0.0 a 1.0
    float interpolado = lerp(inicio, fin, t);
    std::cout << "lerp(" << inicio << ", " << fin << ", " << t << ") = " << interpolado << "\n";
  }

  // Interpolación entre vectores
  Vector3 pos_inicio(0.0f, 0.0f, 0.0f);
  Vector3 pos_fin(10.0f, 5.0f, -3.0f);

  std::cout << "\nInterpolación entre vectores:\n";
  for(int i = 0; i <= 4; i++) {
    float t = i / 4.0f;
    Vector3 pos_interpolada(lerp(pos_inicio.x, pos_fin.x, t), lerp(pos_inicio.y, pos_fin.y, t),
                            lerp(pos_inicio.z, pos_fin.z, t));
    std::cout << "t=" << t << ": (" << pos_interpolada.x << ", " << pos_interpolada.y << ", "
              << pos_interpolada.z << ")\n";
  }

  // Step function - útil para transiciones bruscas
  std::cout << "\nStep function:\n";
  float umbral = 5.0f;
  float test_values[] = {2.0f, 5.0f, 8.0f};
  for(int i = 0; i < 3; i++) {
    bool resultado = step(umbral, test_values[i]);
    std::cout << "step(" << umbral << ", " << test_values[i] << ") = " << resultado << "\n";
  }

  // Smoothstep - interpolación suave
  std::cout << "\nSmoothstep (transición suave):\n";
  float edge0 = 2.0f, edge1 = 8.0f;
  for(int i = 0; i <= 10; i++) {
    float x = i * 1.0f;  // 0 a 10
    float smooth = smoothstep(edge0, edge1, x);
    std::cout << "smoothstep(" << edge0 << ", " << edge1 << ", " << x << ") = " << smooth << "\n";
  }
  std::cout << "\n";

  // =====================================
  // FUNCIONES RÁPIDAS (FAST FUNCTIONS)
  // =====================================

  std::cout << "=== FUNCIONES RÁPIDAS ===\n";

  double valores_test[] = {2.3, 5.7, -3.1, 0.9};

  for(int i = 0; i < 4; i++) {
    double val = valores_test[i];
    long fast_r = fast_rint(val);
    long fast_f = fast_floor(val);
    long normal_r = (long)rint(val);
    long normal_f = (long)floor(val);

    std::cout << "Valor: " << val << "\n";
    std::cout << "  fast_rint: " << fast_r << " vs rint: " << normal_r << "\n";
    std::cout << "  fast_floor: " << fast_f << " vs floor: " << normal_f << "\n";
  }
  std::cout << "\n";

  // =====================================
  // NÚMEROS ALEATORIOS
  // =====================================

  std::cout << "=== NÚMEROS ALEATORIOS ===\n";

  srand48(42);  // Semilla para reproducibilidad

  std::cout << "10 números aleatorios [0,1):\n";
  for(int i = 0; i < 10; i++) {
    double random = drand48();
    std::cout << random << " ";
  }
  std::cout << "\n";

  // Vectores aleatorios
  std::cout << "\n5 vectores aleatorios:\n";
  srand48(123);
  for(int i = 0; i < 5; i++) {
    Vector3 random_vec(drand48() * 10.0f - 5.0f,  // [-5, 5]
                       drand48() * 10.0f - 5.0f, drand48() * 10.0f - 5.0f);
    std::cout << "Vector " << i << ": (" << random_vec.x << ", " << random_vec.y << ", "
              << random_vec.z << ")\n";
  }
  std::cout << "\n";

  // =====================================
  // VALORES ESPECIALES (INF, NAN)
  // =====================================

  std::cout << "=== VALORES ESPECIALES ===\n";

  float infinito = INFINITY;
  float no_numero = NAN;

  std::cout << "INFINITY: " << infinito << "\n";
  std::cout << "NAN: " << no_numero << "\n";

  // Comprobar si un número es infinito o NaN
  float test_inf = 1.0f / 0.0f;  // Infinito
  float test_nan = 0.0f / 0.0f;  // NaN

  std::cout << "1/0 es infinito: " << (test_inf == INFINITY) << "\n";
  std::cout << "0/0 es NaN: " << (test_nan != test_nan) << " (NaN != NaN es true)\n\n";

  // =====================================
  // EJEMPLO PRÁCTICO: ANIMACIÓN SUAVE
  // =====================================

  std::cout << "=== EJEMPLO PRÁCTICO: ANIMACIÓN SUAVE ===\n";

  // Simular una animación de 2 segundos a 30 FPS
  Vector3 pos_origen(0.0f, 0.0f, 0.0f);
  Vector3 pos_destino(100.0f, 50.0f, 25.0f);

  int frames_totales = 60;  // 2 segundos a 30 FPS

  std::cout << "Animación suave de posición (cada 10 frames):\n";
  for(int frame = 0; frame <= frames_totales; frame += 10) {
    float t = (float)frame / frames_totales;  // [0, 1]

    // Usar smoothstep para una animación más natural
    float t_suave = smoothstep(0.0f, 1.0f, t);

    Vector3 pos_actual(lerp(pos_origen.x, pos_destino.x, t_suave),
                       lerp(pos_origen.y, pos_destino.y, t_suave),
                       lerp(pos_origen.z, pos_destino.z, t_suave));

    std::cout << "Frame " << frame << " (t=" << t << ", t_suave=" << t_suave << "): ";
    std::cout << "(" << pos_actual.x << ", " << pos_actual.y << ", " << pos_actual.z << ")\n";
  }

  return 0;
}