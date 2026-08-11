/* Inclusión de bibliotecas estándar de C */
#include <stdio.h> 
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <locale.h>
#include <time.h> 


#ifdef _WIN32
#include <windows.h>
#endif

// Definición de macros para representar el estado de las celdas en el tablero.
#define AGUA '~'    
#define BARCO 'B'   
#define IMPACTO 'X' 
#define FALLIDO 'O' 
#define HUNDIDO '#' 
#define REPETIDO 'R'

/* ================================================================================= */
/* Bloque 1: Definición de las estructuras de datos para el juego.                   */
/* ================================================================================= */

// Representa un barco individual: su nombre, tamaño, vidas restantes,
// si ya está hundido (estado) y su posición/orientación en el tablero.
typedef struct
{
    char nom_barco[20]; 
    int tamanio;        
    int can_vidas;      
    bool estado;        
    int fila;           
    char columns;      
    int col_num;        
    char orientacion;   
} Barco;

// Representa un tablero de juego: la matriz de celdas (agua/barco/impacto/etc.),
// el arreglo con los 4 barcos de la flota y el tamaño real usado (8 a 12).
typedef struct
{
    char tablero[12][12];
    Barco barcos[4];
    int tamanio_tablero;
} Tablero;

// Nodo de una lista enlazada de jugadores: guarda sus estadísticas históricas
// y, mientras dura la partida, su flota propia y el registro de sus disparos.
// El puntero sig_Jugador (memoria dinámica) encadena a los demás jugadores.
typedef struct jugador 
{
    char nombre[20];         
    int partidas_jugadas;    
    int part_ganadas;        
    int part_perdidas;       
    Tablero mi_flota;        
    Tablero mis_disparos;    
    struct jugador *sig_Jugador;
} Jugador; 

/* ================================================================================= */
/* Bloque 2: Prototipos de funciones (Para un correcto enlazado)                     */
/* ================================================================================= */
//1. Preparación y manejo de matrices (El tablero)
void inicializarTablero(Tablero *t, int n);
int esPosicionValida(int fila, int col, char orientacion, int tamanio, int n);
int noHaySuperposicion(Tablero *t, int fila, int col, char orientacion, int tamanio);
void colocarEnMatriz(Tablero *t, int indiceBarco, int fila, int col, char orientacion, int tamanio);
void borrarBarcoDeMatriz(Tablero *t, int indiceBarco);
void colocarBarcos(Tablero *t);

//2. Logica del juego y disparos (El motor)
int encontrarBarcoEnCelda(Tablero *t, int fila, int col);
char procesarDisparo(Tablero *t, int fila, int col);
void marcarHundidoEnMatriz(Tablero *t, int indiceBarco);
int todosLosBarcosHundidos(Tablero *t);
void mostrarTablero(Tablero *t, bool ocultarBarcos);
int jugarPartida(Tablero *t1, Tablero *t2, const char *nombreJ1, const char *nombreJ2);
void gestionarPartida(Jugador **listaJugadores);

//3. Jugadores, Estadisticas y Archivos (La Persistencia)
Jugador *buscarOCrearJugador(Jugador **cabeza, const char *nombre);
void actualizarEstadisticas(Jugador *j1, Jugador *j2, int resultado);
void mostrarEstadisticas(Jugador *cabeza);
void liberarListaJugadores(Jugador *cabeza);
void guardarHistorial(const char *jugador1, const char *jugador2, const char *ganador);
void mostrarHistorial(void);
void guardarListaJugadores(Jugador *cabeza);
void cargarListaJugadores(Jugador **cabeza);

//4. Interfaz y Validaciones (Interaccion con el Usuario)
void mostrarReglas(void);
int leerEnteroValido(const char *mensaje, int minimo, int maximo);
void pausarConsola(void);
int leerCoordenada(const char *mensaje, int limite, int *fila, int *col, bool permitirSalir);

/* ================================================================================= */
/* Bloque 3: Desarrollo de Funciones Lógicas y Visuales                              */
/* ================================================================================= */



// Deja el tablero completamente en AGUA y prepara la flota fija de 4 barcos
// (Portaaviones, Acorazado, Crucero, Destructor) con sus tamaños y vidas iniciales.
void inicializarTablero(Tablero *t, int n) {
    t->tamanio_tablero = n;
    for (int fila = 0; fila < n; fila++)
    {
        for (int columna = 0; columna < n; columna++)
        {
            t->tablero[fila][columna] = AGUA;
        }
    }
    int tamanios[4] = {5, 4, 3, 2};
    for (int i = 0; i < 4; i++) {
        t->barcos[i].tamanio = tamanios[i];     
        t->barcos[i].can_vidas = tamanios[i];   
        t->barcos[i].estado = false;            
    }
}

// Valida que la celda inicial esté dentro del tablero y que, según la
// orientación (Horizontal o Vertical), el barco completo no se salga del borde.
int esPosicionValida(int fila, int col, char orientacion, int tamanio, int n)
{
    if (fila < 0 || fila >= n || col < 0 || col >= n)
    {
        return 0; 
    }
    if (orientacion == 'H') 
    {
        return (col + tamanio <= n); 
    }
    else 
    {
        return (fila + tamanio <= n); 
    }
}

// Recorre las celdas que ocuparía el barco (avanzando en fila si es V,
// o en columna si es H) y devuelve 0 si alguna ya tiene otro barco.
int noHaySuperposicion(Tablero *t, int fila, int col, char orientacion, int tamanio)
{
    for (int i = 0; i < tamanio; i++) 
    {  //Operador ternario, if y else más reducidos.
        int f_actual = (orientacion == 'V') ? fila + i : fila; 
        int c_actual = (orientacion == 'H') ? col + i : col; 

        if (t->tablero[f_actual][c_actual] != AGUA)
        {
	        return 0; 
        }
    }
    return 1; 
} 

// Escribe BARCO en cada celda ocupada por el barco y guarda su posición de
// origen y orientación en la estructura Barco (necesario para ubicarlo luego).
void colocarEnMatriz(Tablero *t, int indiceBarco, int fila, int col, char orientacion, int tamanio) 
{
    for (int i = 0; i < tamanio; i++) 
    {
        int f = (orientacion == 'V') ? fila + i : fila;
        int c = (orientacion == 'H') ? col + i : col;
        t->tablero[f][c] = BARCO;
    }
    t->barcos[indiceBarco].fila = fila;                     
    t->barcos[indiceBarco].columns = (char)('A' + col);    
    t->barcos[indiceBarco].col_num = col;                   
    t->barcos[indiceBarco].orientacion = orientacion;       
}
/*Borra un barco de la matriz dejándola como AGUA, útil para reubicarlo */
void borrarBarcoDeMatriz(Tablero *t, int indiceBarco)
{
    Barco *b = &t->barcos[indiceBarco];
    for (int i = 0; i < b->tamanio; i++)
    {
        int f = (b->orientacion == 'V') ? b->fila + i : b->fila;
        int c = (b->orientacion == 'H') ? b->col_num + i : b->col_num;
        t->tablero[f][c] = AGUA;
    }
}
// Cuando un barco pierde su última vida, pinta todas sus celdas como HUNDIDO.
void marcarHundidoEnMatriz(Tablero *t, int indiceBarco)
{
    Barco *b = &t->barcos[indiceBarco];
    for (int i = 0; i < b->tamanio; i++)
    {
        int f = (b->orientacion == 'V') ? b->fila + i : b->fila;
        int c = (b->orientacion == 'H') ? b->col_num + i : b->col_num;
        t->tablero[f][c] = HUNDIDO;
    }
}

// Guía al jugador para ubicar sus 4 barcos y luego le permite confirmar
// o reubicar alguno antes de empezar la partida.
void colocarBarcos(Tablero *t) {
    const char *nombresBarco[4] = {"Portaaviones (5)", "Acorazado (4)", "Crucero (3)", "Destructor (2)"};

    // --- FASE 1: COLOCACIÓN INICIAL ---
    for (int i = 0; i < 4; i++) {
        int colocado = 0; 
        // Repite el pedido de posición hasta que sea válida y sin superposición
        while (!colocado) 
        {
            int fila, col;
            char orientacion;  

            printf("\nColocar %s\n", nombresBarco[i]);
            
            
            char mensajeCoord[80];
            sprintf(mensajeCoord, "Posicion inicial (Columna y Fila, ej: A1, de A a %c): ", (char)('A' + t->tamanio_tablero - 1));
            leerCoordenada(mensajeCoord, t->tamanio_tablero, &fila, &col, false);
            
            printf("Orientacion (H/V): ");
            scanf(" %c", &orientacion); 

            if (orientacion >= 'a' && orientacion <= 'z') {
                orientacion = (char)(orientacion - 'a' + 'A');
            }

            int tamanio = t->barcos[i].tamanio; 

            if (!esPosicionValida(fila, col, orientacion, tamanio, t->tamanio_tablero)) 
            {
                printf("Error: El barco no entra o la orientacion es invalida (use H o V).\n");
            } 
            else if (!noHaySuperposicion(t, fila, col, orientacion, tamanio)) 
            {
                printf("Error: Hay superposicion con otro barco. Intenta de nuevo.\n");
            } 
            else 
            {
                colocarEnMatriz(t, i, fila, col, orientacion, tamanio); 
                colocado = 1; 
                printf("Barco %s colocado exitosamente.\n", nombresBarco[i]);
                
                // MUESTRA EL PROGRESO DESPUÉS DE CADA BARCO
                mostrarTablero(t, false);
            }
        }
    }

    // --- FASE 2: EDICIÓN Y CONFIRMACIÓN ---
    // Permite reubicar barcos ya colocados antes de comenzar el juego.
    char respuesta;
    int edicionTerminada = 0;
    
    while (!edicionTerminada) {
        printf("\nTodos tus barcos estan en el tablero.\n");
        printf("¿Deseas cambiar la posicion de algun barco? (S/N): ");
        scanf(" %c", &respuesta);
        
        if (respuesta == 'N' || respuesta == 'n') {
            edicionTerminada = 1; // El jugador está conforme, sale del bucle
        } 
        else if (respuesta == 'S' || respuesta == 's') {
            printf("\n¿Cual barco deseas mover?\n");
            for (int i = 0; i < 4; i++) {
                printf("%d. %s\n", i + 1, nombresBarco[i]);
            }
            
            // Pide qué barco mover (1-4) y lo convierte a índice (0-3)
            int opcion = leerEnteroValido("Elija un barco (1-4): ", 1, 4);
            int indice = opcion - 1;
            
            // BORRAR EL BARCO VIEJO PARA QUE NO CHOQUE CONSIGO MISMO
            borrarBarcoDeMatriz(t, indice);
            
            // CICLO PARA VOLVER A COLOCAR ESE BARCO ESPECÍFICO
            int reubicado = 0;
            while (!reubicado) {
                printf("\nReubicando %s\n", nombresBarco[indice]);

                int fila, col;
                char mensajeCoord[80];
                sprintf(mensajeCoord, "Nueva posicion (Columna y Fila, ej: A1, de A a %c): ", (char)('A' + t->tamanio_tablero - 1));
                leerCoordenada(mensajeCoord, t->tamanio_tablero, &fila, &col, false);
                
                char orientacion;
                printf("Orientacion (H/V): ");
                scanf(" %c", &orientacion);
                if (orientacion >= 'a' && orientacion <= 'z') orientacion = (char)(orientacion - 'a' + 'A');

                int tamanio = t->barcos[indice].tamanio;

                if (!esPosicionValida(fila, col, orientacion, tamanio, t->tamanio_tablero)) {
                    printf("Error: El barco no entra o la orientacion es invalida.\n");
                } else if (!noHaySuperposicion(t, fila, col, orientacion, tamanio)) {
                    printf("Error: Hay superposicion con otro barco.\n");
                } else {
                    colocarEnMatriz(t, indice, fila, col, orientacion, tamanio);
                    reubicado = 1;
                    printf("Barco %s reubicado exitosamente.\n", nombresBarco[indice]);
                    mostrarTablero(t, false); // Muestra el tablero actualizado
                }
            }
        } 
        else {
            printf("Respuesta no valida. Ingrese S o N.\n");
        }
    }
}

// Dada una celda, busca en los 4 barcos cuál la ocupa (según su orientación
// H o V) y devuelve su índice, o -1 si ninguno coincide.
int encontrarBarcoEnCelda (Tablero *t, int fila, int col) 
{
    for (int i = 0; i < 4; i++) 
    {
        Barco b = t->barcos[i]; 
        if (b.orientacion == 'H') 
        {
 	        if((fila == b.fila) && (col >= b.col_num) && (col < b.col_num + b.tamanio)) 
            {
		        return i; 
		    }
        } 
        else if (b.orientacion == 'V') 
        {
            if ((col == b.col_num) && (fila >= b.fila) && (fila < b.fila + b.tamanio)) 
            {
                return i; 
            }
        }
    }
    return -1; 
}

// Aplica un disparo sobre el tablero rival: detecta si es barco, agua o
// celda repetida, actualiza vidas/estado del barco afectado y devuelve
// el código de resultado ('X' impacto, '#' hundido, 'O' agua, 'R' repetido).
char procesarDisparo(Tablero *t, int fila, int col) 
{
    char celda = t->tablero[fila][col];
    if (celda == BARCO) 
    {
        t->tablero[fila][col] = IMPACTO;
        int indiceBarco = encontrarBarcoEnCelda(t, fila, col);
        t->barcos[indiceBarco].can_vidas = t->barcos[indiceBarco].can_vidas - 1;

        if (t->barcos[indiceBarco].can_vidas == 0) 
        {
            t->barcos[indiceBarco].estado = 1;
            marcarHundidoEnMatriz(t, indiceBarco);
            return '#';
        } 
        else 
        {
            return 'X';
        }
    } 
    else if (celda == AGUA) {
        t->tablero[fila][col] = FALLIDO;
        return 'O';
    }
    else 
    {
        return 'R';
    }
}

// Condición de victoria: recorre la flota y devuelve 1 solo si los 4
// barcos ya están hundidos (estado == true).
int todosLosBarcosHundidos (Tablero *t) 
{
    for (int i = 0; i < 4; i++) 
    {
        if (t->barcos[i].estado == false)
        {
            return 0; 
        }
    }
    return 1;
}


// Imprime el tablero con encabezado de letras (columnas) y números (filas).
// Si ocultarBarcos es true (tablero rival), muestra los barcos como AGUA
// para no revelar su posición al oponente.
void mostrarTablero(Tablero *t, bool ocultarBarcos) 
{
    int n = t->tamanio_tablero;

    printf("\n     "); 
    for (int c = 0; c < n; c++) 
    {
        printf("%c ", 'A' + c); 
    }
    printf("\n    +");
    for (int c = 0; c < n; c++) printf("--");
    printf("\n");

    for (int f = 0; f < n; f++) 
    {
        printf("%2d | ", f + 1); 
        
        for (int c = 0; c < n; c++) 
        {
            char celda = t->tablero[f][c];
            
            if (ocultarBarcos && celda == BARCO) 
            {
                printf("%c ", AGUA);
            } 
            else 
            {
                printf("%c ", celda); 
            }
        }
        printf("\n");
    }
}

// Bucle principal de la partida: alterna turnos entre los dos jugadores,
// muestra el tablero propio y el del rival (oculto), pide una coordenada
// de disparo y termina cuando alguien hunde toda la flota enemiga o
// abandona la partida (devuelve -1/-2 en ese caso).
int jugarPartida (Tablero *t1, Tablero *t2, const char *nombreJ1, const char *nombreJ2) 
{
    int turno = 1;
    int fila, colm_numero;
    char resultado;
    bool hayGanador = false; 

    while (hayGanador == false) 
    {
        Tablero *propio = (turno == 1) ? t1 : t2;
        Tablero *rival = (turno == 1) ? t2 : t1;
        const char *nombreActual = (turno == 1) ? nombreJ1 : nombreJ2;

        printf("\n==============================");
        printf("\nTURNO DE: %s", nombreActual);
        printf("\n==============================\n");

        printf("\n--- TU PROPIO TABLERO ---\n");
        mostrarTablero(propio, false); // Muestra sus propios barcos
        
        printf("\n--- TABLERO DEL RIVAL (Tus ataques) ---\n");
        mostrarTablero(rival, true);  // Oculta los barcos del enemigo

        
        // permitirSalir=true habilita la opción 'Q' para abandonar la partida
        int coordenadaOk = leerCoordenada(
"\nIngrese coordenada de disparo (ej: A1) o 'Q' para guardar y terminar la partida: ", rival->tamanio_tablero, &fila, &colm_numero, true);

        if (coordenadaOk == 0)
        {
            printf("\n%s decidio terminar la partida. Se guardara el progreso en el historial.\n", nombreActual);
            
            return (turno == 1) ? -1 : -2;
        }

        resultado = procesarDisparo(rival, fila, colm_numero);
        
        if (resultado == 'X') printf("\n>>> IMPACTO! <<<\n");
        else if (resultado == '#') printf("\n>>> HUNDIDO! <<<\n");
        else if (resultado == 'O') printf("\n>>> AGUA... <<<\n");
        else if (resultado == 'R') printf("\n>>> Ya habias disparado aqui. Perdiste el turno. <<<\n");

        if (todosLosBarcosHundidos(rival) == 1) 
        {
            printf("\n--------------------------------");
            printf("\n  FELICIDADES %s! HAS GANADO.", nombreActual);
            printf("\n--------------------------------");
            hayGanador = true; 
        } 
        else 
        {
            turno = (turno == 1) ? 2 : 1;
            printf("\nCambiando de turno...");
            pausarConsola();
            
            #ifdef _WIN32
                system("cls");  
            #else
                system("clear"); 
            #endif
        }
    }
    return turno;
}

// Orquesta una partida completa: pide nombres y tamaño de tablero, inicializa
// ambos tableros, hace que cada jugador coloque su flota, ejecuta la partida
// y al finalizar actualiza estadísticas y guarda el resultado en el historial.
void gestionarPartida(Jugador **listaJugadores) 
{
    char nombre1[20], nombre2[20];
    
    printf("\nNombre Jugador 1: ");
    scanf("%19s", nombre1);
    printf("Nombre Jugador 2: ");
    scanf("%19s", nombre2);

    int tamanio = leerEnteroValido("Tamaño del tablero (8 a 12): ", 8, 12);
    system("cls");

    Tablero t1, t2;
    inicializarTablero(&t1, tamanio);
    inicializarTablero(&t2, tamanio);

    printf("\n%s, coloca tus barcos.\n", nombre1);
    colocarBarcos(&t1);
    system("cls"); //Limpia la terminal, para evitar se vea el tablero rival.
    printf("\n%s, coloca tus barcos.\n", nombre2);
    colocarBarcos(&t2);
    system("cls");
    int ganador = jugarPartida(&t1, &t2, nombre1, nombre2);

    
    // Si algún jugador salió con 'Q', se registra como partida abandonada
    // y no se computa en las estadísticas de victorias/derrotas.
    if (ganador == -1 || ganador == -2)
    {
        const char *nombreAbandono = (ganador == -1) ? nombre1 : nombre2;
        char resultadoHistorial[64];
        sprintf(resultadoHistorial, "Abandonada por %s", nombreAbandono);
        guardarHistorial(nombre1, nombre2, resultadoHistorial);
        printf("\nLa partida se guardo en el historial como abandonada.\n");
        pausarConsola();
        return;
    }

    Jugador *j1 = buscarOCrearJugador(listaJugadores, nombre1);
    Jugador *j2 = buscarOCrearJugador(listaJugadores, nombre2);
    
    if (j1 != NULL && j2 != NULL) 
    {
        actualizarEstadisticas(j1, j2, ganador);
    }

    const char *nombreGanador = (ganador == 1) ? nombre1 : nombre2;
    guardarHistorial(nombre1, nombre2, nombreGanador);
    pausarConsola();
}

// Recorre la lista enlazada de jugadores buscando el nombre; si existe lo
// devuelve, y si no, reserva memoria dinámica (malloc) para un nodo nuevo
// y lo agrega al final de la lista.
Jugador *buscarOCrearJugador(Jugador **cabeza, const char *nombre) 
{
    Jugador *actual = *cabeza;
    Jugador *ultimo = NULL;
    while (actual != NULL) 
    {
        if (strcmp(actual->nombre, nombre) == 0) 
        {
            return actual; 
        }
        ultimo = actual;   
        actual = actual->sig_Jugador; 
    }

    Jugador *nuevo = (Jugador *)malloc(sizeof(Jugador));
    if (nuevo == NULL) 
    {
        printf("Error: No hay memoria suficiente.\n");
        return NULL;
    }

    strcpy(nuevo->nombre, nombre); 
    nuevo->partidas_jugadas = 0;
    nuevo->part_ganadas = 0;
    nuevo->part_perdidas = 0;
    nuevo->sig_Jugador = NULL; 

    if (*cabeza == NULL) 
    {
        *cabeza = nuevo;
    } 
    else 
    {
        ultimo->sig_Jugador = nuevo;
    }
    return nuevo; 
}

// Suma una partida jugada a ambos y, según quién ganó (1 o 2),
// incrementa sus contadores de victorias/derrotas.
void actualizarEstadisticas(Jugador *j1, Jugador *j2, int resultado) 
{ 
    j1->partidas_jugadas++; 
    j2->partidas_jugadas++; 
    if (resultado == 1) 
    { 
        j1->part_ganadas++; 
        j2->part_perdidas++; 
    }
    else if (resultado == 2) 
    {
        j2->part_ganadas++; 
        j1->part_perdidas++; 
    }
}

// Recorre la lista enlazada de jugadores e imprime las estadísticas
// acumuladas de cada uno.
void mostrarEstadisticas(Jugador *cabeza) 
{
    Jugador *actual = cabeza;
    if (actual == NULL) 
    {
        printf("No hay jugadores registrados en el historial.\n");
        return; 
    }
    printf("=== HISTORIAL Y ESTADISTICAS DE JUGADORES ===\n");
    while (actual != NULL) 
    {
        printf("----------------------------------------\n");
        printf("Jugador: %s\n", actual->nombre);
        printf("  Partidas Jugadas: %d\n", actual->partidas_jugadas);
        printf("  Partidas Ganadas: %d\n", actual->part_ganadas);
        printf("  Partidas Perdidas: %d\n", actual->part_perdidas);
        actual = actual->sig_Jugador;
    }
    printf("----------------------------------------\n");
}

// Libera toda la memoria dinámica reservada para la lista de jugadores,
// guardando el siguiente nodo antes de liberar el actual para no perder la referencia.
void liberarListaJugadores(Jugador *cabeza) 
{
    Jugador *actual = cabeza;
    while (actual != NULL) 
    {
        Jugador *siguienteNodo = actual->sig_Jugador;
        free(actual); 
        actual = siguienteNodo;
    }
}

// Agrega (modo "a" = append) una línea al archivo historial.txt con la
// fecha actual y el resultado de la partida, sin borrar lo ya guardado.
void guardarHistorial(const char *jugador1, const char *jugador2, const char *ganador) 
{
    FILE *archivo = fopen("historial.txt", "a");
    if (archivo == NULL) 
    {
        printf("Error: No se pudo abrir el archivo de historial.\n");
        return;
    }

    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    char fecha[12];
    sprintf(fecha, "%02d/%02d/%d", tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900);

    
    fprintf(archivo, "%s - %s vs %s - Resultado: %s\n", fecha, jugador1, jugador2, ganador);
    fclose(archivo); 
}

// Lee el archivo historial.txt línea por línea y lo muestra por consola;
// si el archivo no existe o está vacío, avisa que aún no hay partidas.
void mostrarHistorial(void) 
{
    FILE *archivo = fopen("historial.txt", "r");
    if (archivo == NULL) 
    {
        printf("\nAun no hay partidas registradas en el historial.\n");
        return; 
    }

    char linea[256]; 
   
    bool hayContenido = false;

    printf("\n=== HISTORIAL DE PARTIDAS ===\n");
    while (fgets(linea, sizeof(linea), archivo) != NULL) 
    {
        if (linea[0] != '\n' && linea[0] != '\r' && linea[0] != '\0')
        {
            hayContenido = true;
            printf("%s", linea); 
        }
    }

    if (!hayContenido)
    {
        printf("Aun no hay partidas registradas en el historial.\n");
    }
    printf("=============================\n");
    fclose(archivo); 
}

// Sobrescribe (modo "w") jugadores.txt con toda la lista actual en formato
// CSV (nombre,jugadas,ganadas,perdidas), para persistir los datos entre ejecuciones.
void guardarListaJugadores(Jugador *cabeza) {
    FILE *archivo = fopen("jugadores.txt", "w");
    if (archivo == NULL) {
        printf("Error: No se pudo abrir el archivo de jugadores para guardar.\n");
        return;
    }
    Jugador *actual = cabeza;
    while (actual != NULL) {
        fprintf(archivo, "%s,%d,%d,%d\n", 
                actual->nombre, 
                actual->partidas_jugadas, 
                actual->part_ganadas, 
                actual->part_perdidas);
        actual = actual->sig_Jugador; 
    }
    fclose(archivo);
}

// Reconstruye la lista de jugadores leyendo el CSV guardado por
// guardarListaJugadores; si el archivo no existe (primera ejecución), no hace nada.
void cargarListaJugadores(Jugador **cabeza) 
{
    FILE *archivo = fopen("jugadores.txt", "r");
    if (archivo == NULL) 
    {
        return; 
    }
    char nombre[20];
    int jugadas, ganadas, perdidas;

    // %19[^,] lee el nombre hasta encontrar una coma, evitando desbordar el buffer
    while (fscanf(archivo, "%19[^,],%d,%d,%d\n", nombre, &jugadas, &ganadas, &perdidas) == 4) 
    {
        Jugador *nuevo = buscarOCrearJugador(cabeza, nombre);
        if (nuevo != NULL) 
        {
            nuevo->partidas_jugadas = jugadas;
            nuevo->part_ganadas = ganadas;
            nuevo->part_perdidas = perdidas;
        }
    }
    fclose(archivo);
}

void mostrarReglas(void)
{
    printf("\n===================== REGLAS DE LA BATALLA NAVAL =====================\n");
    printf("1. Cada jugador tiene un tablero cuadrado (por defecto de 8 a 12 casilleros\n");
    printf("   de lado), identificado con letras en las columnas (A, B, C...) y\n");
    printf("   numeros en las filas (0, 1, 2...).\n\n");
    printf("2. Cada jugador cuenta con una flota de 4 barcos, que debe ubicar en su\n");
    printf("   propio tablero antes de comenzar:\n");
    printf("     - Portaaviones: 5 casilleros\n");
    printf("     - Acorazado:    4 casilleros\n");
    printf("     - Crucero:      3 casilleros\n");
    printf("     - Destructor:   2 casilleros\n\n");
    printf("3. Los barcos se colocan en forma Horizontal (H) o Vertical (V), indicando\n");
    printf("   la fila y columna de inicio. No pueden salirse del tablero ni\n");
    printf("   superponerse con otro barco ya colocado.\n\n");
    printf("4. El juego se desarrolla por turnos. En su turno, un jugador ve su propio\n");
    printf("   tablero y el tablero de disparos sobre el rival (donde el rival\n");
    printf("   oculta sus barcos), y elige una coordenada para disparar.\n\n");
    printf("5. Al disparar a una celda puede pasar:\n");
    printf("     - AGUA ('~' -> 'O'): no habia barco en esa celda.\n");
    printf("     - IMPACTO ('X'): el disparo toco parte de un barco, que sigue a flote.\n");
    printf("     - HUNDIDO ('#'): el impacto fue el ultimo que le quedaba de vida a ese\n");
    printf("       barco, por lo que queda hundido por completo.\n");
    printf("     - REPETIDO ('R'): ya se habia disparado antes a esa misma celda,\n");
    printf("       se pierde el turno.\n\n");
    printf("6. Los turnos se van alternando entre ambos jugadores hasta que uno de\n");
    printf("   ellos logra hundir los 4 barcos de la flota rival.\n\n");
    printf("7. Gana la partida el primer jugador que logra hundir toda la flota\n");
    printf("   enemiga.\n");
    printf("=========================================================================\n");
}

// Pide un número entero por teclado hasta que sea válido y esté dentro
// del rango [minimo, maximo]; limpia el buffer de entrada en cada intento
// para evitar bucles infinitos si el usuario ingresa texto.
int leerEnteroValido(const char *mensaje, int minimo, int maximo)
{
    int valor;         
    int lecturaOk;     
    do
    {
        printf("%s", mensaje);
        lecturaOk = scanf("%d", &valor);

        // Descarta el resto de la línea (incluye el \n) sobrante en el buffer
        int c;
        while ((c = getchar()) != '\n' && c != EOF) { }

        if (lecturaOk != 1)
        {
            printf("Entrada invalida. Debe ingresar un numero entero.\n");
        }
        else if (valor < minimo || valor > maximo)
        {
            printf("Fuera de rango. Ingrese un valor entre %d y %d.\n", minimo, maximo);
        }
    } while (lecturaOk != 1 || valor < minimo || valor > maximo);

    return valor;
}


// Lee una coordenada en formato "Letra Numero" (ej: A 1), la valida contra
// el límite del tablero y la convierte a índices internos base 0 (*fila, *col).
// Si permitirSalir es true, la letra 'Q' corta la función devolviendo 0.
int leerCoordenada(const char *mensaje, int limite, int *fila, int *col, bool permitirSalir)
{
    char letra;
    int filaVisual;
    int lecturaOk;

    while (1)
    {
        printf("%s", mensaje);
        lecturaOk = scanf(" %c", &letra);

        if (lecturaOk != 1)
        {
            int c;
            while ((c = getchar()) != '\n' && c != EOF) { }
            printf("Entrada invalida. Intenta de nuevo.\n");
            continue;
        }

        if (letra >= 'a' && letra <= 'z')
        {
            letra = (char)(letra - 'a' + 'A');
        }

        // Opcion de salir y guardar la partida (solo habilitada durante los turnos de juego)
        if (permitirSalir && letra == 'Q')
        {
            int c;
            while ((c = getchar()) != '\n' && c != EOF) { }
            return 0;
        }

        if (letra < 'A' || letra >= (char)('A' + limite))
        {
            int c;
            while ((c = getchar()) != '\n' && c != EOF) { }
            printf("Columna fuera del tablero. Ingrese una letra entre A y %c.\n", (char)('A' + limite - 1));
            continue;
        }

        lecturaOk = scanf("%d", &filaVisual);
        int c;
        while ((c = getchar()) != '\n' && c != EOF) { }

        if (lecturaOk != 1)
        {
            printf("Entrada invalida. Debe ingresar letra y luego numero (ej: A 1).\n");
            continue;
        }

        if (filaVisual < 1 || filaVisual > limite)
        {
            printf("Fila fuera del tablero. Ingrese un valor entre 1 y %d.\n", limite);
            continue;
        }

        // Convierte letra->índice de columna y fila visual (1-based) -> índice interno (0-based)
        *col = letra - 'A';
        *fila = filaVisual - 1;
        return 1;
    }
}


void pausarConsola(void)
{
    printf("\nPresiona Enter para continuar...");
    int c;
    while ((c = getchar()) != '\n' && c != EOF) { }
}

/* ================================================================================= */
/* Bloque 4: Función Principal (Menú de Selección)                                   */
/* ================================================================================= */

// Punto de entrada: configura la consola (UTF-8 en Windows), carga el
// historial de jugadores desde disco y muestra el menú principal en un
// bucle hasta que el usuario elige salir.
int main() 
{
    
    #ifdef _WIN32
        system("chcp 65001 > nul");
        SetConsoleCP(CP_UTF8);
        SetConsoleOutputCP(CP_UTF8);
    #endif
    setlocale(LC_ALL, ""); //utilizado para agregar caracteres especiales



    Jugador *listaJugadores = NULL; 
    cargarListaJugadores(&listaJugadores); 
    int opcion; 
    // Bucle del menú principal: se repite hasta elegir la opción 5 (Salir)
    do 
    {
        printf("\n=========================================\n");
        printf("        BATALLA NAVAL - MENU PRINCIPAL\n");
        printf("=========================================\n");
        printf("1. Jugar\n");
        printf("2. Ver reglas\n");
        printf("3. Ver historial de partidas\n");
        printf("4. Ver estadisticas de jugadores\n");
        printf("5. Salir\n");
        

        opcion = leerEnteroValido("Elija una opcion (1-5): ", 1, 5);
       
        switch (opcion) 
        {
            case 1: 
                gestionarPartida(&listaJugadores);
                system("cls");
                break;
                
            case 2: 
                mostrarReglas();
                pausarConsola();
                system("cls");
                break;
                
            case 3: 
                mostrarHistorial();
                system("pause > nul");
                system("cls");
                break;
                
            case 4: 
                mostrarEstadisticas(listaJugadores);
                system("pause > nul");
                system("cls");
                break;
                
            case 5: 
                // Antes de salir, persiste las estadísticas en jugadores.txt
                printf("\nGuardando estadisticas y saliendo...\n");
                guardarListaJugadores(listaJugadores);
                break;
        }

    } while (opcion != 5); 

    // Libera toda la memoria dinámica de la lista antes de terminar el programa
    liberarListaJugadores(listaJugadores);
    return 0; 
}