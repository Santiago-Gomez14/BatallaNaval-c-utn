# BatallaNaval-c-utn
Batalla naval en C. TP Integrador de programación 1.

# ⚓ Batalla Naval en C — TP Integrador

Proyecto integrador desarrollado para la materia **Programación 1** de la **Tecnicatura Universitaria en Programación (TUP)** en la **Universidad Tecnológica Nacional - Facultad Regional La Plata (UTN FRLP)**.

---

## 👥 Integrantes y Equipo de Trabajo

- **Santiago Ariel Gomez** *(Coordinación de código y pantalla compartida)*
- **Hilda Aldana Filardo** *(Investigación bibliográfica y marco teórico)*
- **Camila Harguindeguy** *(Documentación, toma de notas e informe)*

**Docentes:** Sebastián Carlos Marinier | Analía Matilde Bié  
**Comisión:** TUP 13  
**Fecha:** Julio de 2026

---

## 📌 Sobre el Proyecto

El objetivo principal fue desafiar nuestra capacidad de abstracción y lógica implementando el clásico juego de **Batalla Naval** en lenguaje C puro. Adoptamos un enfoque de diseño **descendente (*top-down*)**, dividiendo la lógica general en submódulos independientes para garantizar un código modular, mantenible y robusto.

### 👥 Metodología y Dinámica de Trabajo
- **Desarrollo colaborativo:** Se realizó mediante videollamadas en tiempo real, debatiendo cada funcionalidad en conjunto para asegurar la comprensión total del código por parte de los tres integrantes.
- **Autonomía:** Por decisión unánime, desactivamos las extensiones de asistencia automática por IA durante el tipeo para validar nuestro aprendizaje.
- **Uso responsable de IA:** Consultamos herramientas externas (como Claude o NotebookLM) únicamente ante trabas lógicas o de compilación puntuales (ej. limpieza de búfer con `scanf`, errores de `undefined reference`, persistencia con `fclose()`, alineación visual del tablero y manejo de UTF-8 en consola), pidiendo explicaciones teóricas en lugar de código directo.

---

## 🛠️ Conceptos de C Aplicados

- **Estructuras de datos y tipos:** `struct`, alias con `typedef`, arrays unidimensionales, matrices bidimensionales ($12 \times 12$), arrays de estructuras.
- **Memoria dinámica y listas enlazadas:** Uso de `malloc` y `free` para gestionar la lista dinámica de jugadores con puntero a puntero (`Jugador **cabeza`).
- **Manejo de archivos:** Lectura y escritura (`fopen`, `fprintf`, `fgets`, `fclose`) para persistencia de perfiles, historial y estadísticas.
- **Modularización y punteros:** Uso intensivo de pasaje de parámetros por valor y por referencia. Prototipado completo pre-`main()`.
- **Preprocesador y Directivas:** Uso de `#define` para constantes simbólicas y compilación condicional (`#ifdef _WIN32`) para soportar caracteres especiales (`ñ`, tildes) en Windows mediante `SetConsoleCP` y `utf-8`.
- **Librerías estándar:** `string.h`, `time.h` (registro de fecha/hora real en partidas), `locale.h`, `stdlib.h`.

---

## 🧱 Estructura de Módulos del Código

El sistema está organizado formalmente en cuatro bloques principales:

1. **Bloque N°1 — Estructuras Principales (`struct`):**
   - `Barco`: Registra nombre, dimensión, vidas, orientación y coordenadas.
   - `Tablero`: Matriz del mapa ($12 \times 12$), lista de 4 barcos y tamaño activo.
   - `Jugador`: Datos de perfil, estadísticas (partidas jugadas/ganadas/perdidas), tableros de flota/disparos y puntero al siguiente nodo (`sig_Jugador`).

2. **Bloque N°2 — Declaración de Prototipos:**
   - Prototipado anticipado de todas las funciones antes del `main()` para garantizar una compilación limpia y resolver conflictos de dependencias entre módulos.

3. **Bloque N°3 — Lógica de Juego, Flujo e Interfaz:**
   - **Inicialización y Posicionamiento:** `inicializarTablero`, `esPosicionValida`, `noHaySuperposicion`, `colocarEnMatriz`, `borrarBarcoDeMatriz`, `colocarBarcos`.
   - **Mecánica de Combate:** `procesarDisparo`, `encontrarBarcoEnCelda`, `marcarHundidoEnMatriz`, `todosLosBarcosHundidos`.
   - **Renderizado Visual:** `mostrarTablero` (unificado con parámetro booleano para enmascarar la flota rival).
   - **Persistencia e Historial:** `buscarOCrearJugador`, `guardarHistorial`, `mostrarHistorial`, `guardarListaJugadores`, `cargarListaJugadores`, `liberarListaJugadores`.
   - **Validaciones e Interfaz:** `leerEnteroValido`, `leerCoordenada`, `pausarConsola`.

4. **Bloque N°4 — Función Principal (`main`):**
   - Configuración regional e inicialización de consola UTF-8.
   - Carga automática de base de datos de jugadores desde archivo.
   - Menú interactivo (`do...while` y `switch`) para orquestar las opciones de Juego, Reglas, Historial, Estadísticas y Salida segura con liberación de memoria.

---

## 🚀 Compilación y Ejecución

### Requisitos Previos
- Compilador de C (GCC / MinGW recomendado).

### Pasos para compilar y ejecutar

1. **Clonar el repositorio:**
   ```bash
   git clone [https://github.com/Santiago-Gomez14/BatallaNaval-c-utn.git](https://github.com/Santiago-Gomez14/BatallaNaval-c-utn.git)
   cd BatallaNaval-c-utn
