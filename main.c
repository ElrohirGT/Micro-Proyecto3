#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define FG_RED "\033[91m"
#define FG_BLUE "\033[34m"
#define FG_YELLOW "\033[33m"
#define FG_WHITE "\033[97m"
#define FG_GREEN "\033[32m"

#define BG_GREEN "\033[42m"
#define RESET_COLOR "\033[0m"

void printMatrix(int arr[], int height, int width);
void fillMatrix(int arr[], int height, int width);
void printIntro();
void printOutro();
void clearConsole();

int dron_tick_count = 0;
int velocidad_dron = 0;
int dron_termino = 0;
int frame_count = 0;
int empleados_tick_count = 0;
int velocidad_conjunta_empleados = 0;
int empleados_terminaron = 0;

pthread_mutex_t lock_dron;
pthread_mutex_t lock_empleados;

pthread_cond_t dron_tick_finished;
pthread_cond_t empleados_tick_finished;

typedef struct {
  int *parcela_empleados;
  unsigned int wait_microseconds;
  int total_a_fumigar;

} EmpleadosArgs;

typedef struct {
  int *parcela_dron;
  unsigned int wait_microseconds;
  int total_a_fumigar;
} DronArgs;

void *rutina_dron(void *args) {
  DronArgs *dron_args = (DronArgs *)args;

  unsigned int wait_microseconds = dron_args->wait_microseconds;
  int *parcela_dron = dron_args->parcela_dron;
  int total_a_fumigar = dron_args->total_a_fumigar;

  int seccion_sin_fumigar = -1;

  // Drone fumiga parcela..

  while (!dron_termino) {
    //pthread_mutex_lock(&lock_dron);

    dron_tick_count += 1;

    for (int i = 0; i <= velocidad_dron; i++) {
      seccion_sin_fumigar += 1;
      parcela_dron[seccion_sin_fumigar] = 1;
      dron_termino = seccion_sin_fumigar == (total_a_fumigar - 1);

      if (dron_termino) {
        break;
      }
    }
    usleep(wait_microseconds);
    //pthread_cond_signal(&dron_tick_finished);
  }

  return NULL;
}

void *rutina_empleados(void *args) {
  EmpleadosArgs *empleado_args = (EmpleadosArgs *)args;
  int *parcela_empleados = empleado_args->parcela_empleados;
  unsigned int wait_microseconds = empleado_args->wait_microseconds;
  int total_a_fumigar = empleado_args->total_a_fumigar;

  int seccion_sin_fumigar = -1;
  // Debes asignar la velocidad apropiada

  while (!empleados_terminaron) {
    //pthread_mutex_lock(&lock_empleados);

    empleados_tick_count += 1;

    for (int i = 0; i <= velocidad_conjunta_empleados; i++) {

      seccion_sin_fumigar += 1;
      parcela_empleados[seccion_sin_fumigar] = 1;
      empleados_terminaron = seccion_sin_fumigar == (total_a_fumigar - 1);

      if (empleados_terminaron) {
        break;
      }
    }
    usleep(wait_microseconds);
    //pthread_cond_signal(&empleados_tick_finished);
  }

  return NULL;
}

void showMatrices(int arr1[], int arr2[], int height, int width) {
  clearConsole();
  printf("\033[92mParcela Empleados:\n");
  printMatrix(arr1, height, width);

  printf("\033[92mParcela Dron:\n");
  printMatrix(arr2, height, width);
  printf("Frame %d...\n", ++frame_count);
}

int main(int argc, char *argv[]) {

  pthread_attr_t attr;
  if (pthread_mutex_init(&lock_dron, NULL) !=
      0) // inicializacion de mutex no completada
  {
    printf("\n Inicialización de mutex fallo\n");
    return 1;
  }
  if (pthread_mutex_init(&lock_empleados, NULL) !=
      0) // inicializacion de mutex no completada
  {
    printf("\n Inicialización de mutex fallo\n");
    return 1;
  }

  pthread_cond_init(&empleados_tick_finished, NULL);
  pthread_cond_init(&dron_tick_finished, NULL);

  int ticks_por_segundo = 1;
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

  // Inicializar la estructura de argumento para empleados
  EmpleadosArgs empleados_args;
  DronArgs dron_args;

  int largo_parcela = 0;
  int ancho_parcela = 0;

  if (argc < 2) {
    printIntro();
  }
  int velocidades_empleados[256];

  // ---largo----
  //            |
  //            |
  //            ancho
  //            |
  //            |

  printf("Ingresa el largo y ancho de la parcela:\n");
  fscanf(stdin, "%d %d", &largo_parcela, &ancho_parcela);
  fgetc(stdin); // Quita el \n del final

  int cuenta_empleados = 0;
  printf("Ingresa las velocidades de los empleados:\n");
  while ((fscanf(stdin, "%d", &velocidades_empleados[cuenta_empleados])) == 1) {
    fgetc(stdin); // Quitar el \n del final.
    cuenta_empleados++;
  }
  fgetc(stdin); // Quitar \n después del '-'
  printf("Ingresa la velocidad del dron:\n");
  fscanf(stdin, "%d", &velocidad_dron);
  printf("Ingresa los ticks por segundo:\n");
  fscanf(stdin, "%d", &ticks_por_segundo);

  for (int i = 0; i < cuenta_empleados; i++) {
    printf("La velocidad del empleado %d es %d\n", i, velocidades_empleados[i]);
  }

  int parcela_empleados[largo_parcela * ancho_parcela];
  int parcela_dron[largo_parcela * ancho_parcela];
  unsigned int wait_microseconds = 1000000 / ticks_por_segundo;
  int total_a_fumigar = largo_parcela * ancho_parcela;
  empleados_args.wait_microseconds = wait_microseconds;
  empleados_args.total_a_fumigar = total_a_fumigar;
  empleados_args.parcela_empleados = parcela_empleados;

  dron_args.wait_microseconds = wait_microseconds;
  dron_args.total_a_fumigar = total_a_fumigar;
  dron_args.parcela_dron = parcela_dron;

  pthread_t emp_id;
  pthread_t dron_id;

  fillMatrix(dron_args.parcela_dron = parcela_dron, largo_parcela,
             ancho_parcela);
  fillMatrix(empleados_args.parcela_empleados, largo_parcela, ancho_parcela);

  for (int empleadoI = 0; empleadoI < cuenta_empleados; empleadoI++) {
    velocidad_conjunta_empleados += velocidades_empleados[empleadoI];
  }

  printf("Los empleados en conjunto fumigan %d secciones por tick\n",
         velocidad_conjunta_empleados);

  pthread_create(&emp_id, &attr, rutina_empleados, (void *)&empleados_args);
  pthread_create(&dron_id, &attr, rutina_dron, (void *)&dron_args);

  // Mostrar matrices...

  while (!empleados_terminaron || !dron_termino) {
    showMatrices(parcela_empleados, parcela_dron, ancho_parcela, largo_parcela);
    usleep(1000000 / 60); // 60 frames per second.
    //pthread_cond_wait(&empleados_tick_finished, &lock_empleados);
    //pthread_mutex_unlock(&lock_empleados);

    //pthread_cond_wait(&dron_tick_finished, &lock_dron);
    //pthread_mutex_unlock(&lock_dron);
  }

  pthread_join(emp_id, NULL);
  pthread_join(dron_id, NULL);

  showMatrices(empleados_args.parcela_empleados, dron_args.parcela_dron,
               ancho_parcela, largo_parcela);

  printf("Se termino de fumigar, calculando datos...\n");
  printf("Los empleados se tardaron %d ticks \n", empleados_tick_count);
  printf("El dron se tardo %d ticks \n", dron_tick_count);
  if (empleados_tick_count > dron_tick_count) {
    int vpromempleados = velocidad_conjunta_empleados / cuenta_empleados;

    int empleados_necesarios =
        (empleados_tick_count - dron_tick_count) / vpromempleados;
    if (empleados_necesarios == 0) {
      printf("Se necesita por lo menos 1 empleado más para alcanzar el "
             "tiempo de fumigación del dron\n");
    } else {
      printf("Los empleados se tardaron más que el dron, se necesitan : %d "
             "empleados en promedio para igualar el tiempo del dron \n",
             empleados_necesarios);
    }

  } else {
    int drones_necesarios =
        (dron_tick_count - empleados_tick_count) / velocidad_dron;
    if (drones_necesarios == 0) {
      printf("Se necesita por lo menos 1 dron mas para alcanzar el tiempo "
             "de fumigacion de los empleados\n");
    } else {
      printf("Los empleados se tardaron más que el dron, se necesitan : %d "
             "empleados en promedio para igualar el tiempo del dron \n",
             drones_necesarios);
    }
  }

  printOutro();

  return 0;
}

void printMatrix(int arr[], int height, int width) {
  for (int i = 0; i < height; i++) {
    for (int j = 0; j < width; j++) {
      int index = i * width + j;
      int should_color = arr[index];

      if (should_color) {
        printf(FG_BLUE "⬛" RESET_COLOR);
      } else {
        printf(FG_RED "⬜" RESET_COLOR);
      }
    }
    printf("\n"); // Moverse a la siguiente fila
  }
}

void fillMatrix(int arr[], int height, int width) {
  for (int i = 0; i < height; i++) {
    for (int j = 0; j < width; j++) {
      arr[i * width + j] = 0;
    }
  }
}

void printAnimated(char txt[], int ms_duration);
void printTyping(char txt[]);
void printTypingWithColor(char txt[], char color[]);
void printSlow(char txt[]);
void printNewLine();
void simulateLoading(char start[], char middle[], char end[], char color[]);
void printAnimatedWithColor(char txt[], char color[], int ms_duration);

void showTitle(int ms_duration_per_char) {
  printAnimated(FG_YELLOW "██╗   ██╗███████╗██╗", ms_duration_per_char);
  printNewLine();
  printAnimated("██║   ██║██╔════╝██║", ms_duration_per_char);
  printNewLine();
  printAnimated("██║   ██║███████╗██║", ms_duration_per_char);
  printNewLine();
  printAnimated("██║   ██║╚════██║██║", ms_duration_per_char);
  printNewLine();
  printAnimated("╚██████╔╝███████║██║", ms_duration_per_char);
  printNewLine();
  printAnimated(FG_YELLOW " ╚═════╝ ╚══════╝╚═╝" RESET_COLOR,
                ms_duration_per_char);
}

void printIntro() {

  showTitle(5);
  printNewLine();

  printTyping("WELCOME!!");
  printNewLine();
  printTypingWithColor("Universal simulation industries®", FG_YELLOW);
  printTyping(" thanks you for using our "
              "software beta");
  printNewLine();
  printTyping("NOTE: Please read the manual carefully before using!");
  printNewLine();
  printTyping(FG_YELLOW
              "Universal simulation industries®" RESET_COLOR
              " isn't responsible for any "
              "injuries, false positives or universe annihilation caused by "
              "the misuse of this program.");
  printNewLine();

  simulateLoading("Loading universe", "....", "DONE!", FG_GREEN);
  printNewLine();

  simulateLoading("Retrieving world assets", "....", "DONE!", FG_GREEN);
  printNewLine();

  printTyping("Starting");
  printSlow("...");

  sleep(2);
  clearConsole();
}

void printOutro() {
  printAnimated(FG_GREEN "⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣀⣀⣀⣀⣀⣀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀", 3);
  printNewLine();
  printAnimated("⠀⠀⠀⠀⠀⠀⢀⣠⡴⠾⠛⠋⠉⠉⠉⠉⠙⠛⠷⢦⣄⡀⠀⠀⠀⠀⠀⠀⠀", 3);
  printNewLine();
  printAnimated("⠀⠀⠀⠀⣠⣴⠟⢁⣠⠖⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠈⠻⣦⣄⠀⠀⠀⠀⠀", 3);
  printNewLine();
  printAnimated("⠀⠀⢀⣼⠟⠁⣰⡿⠋⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠈⠻⣧⡀⠀⠀⠀", 3);
  printNewLine();
  printAnimated("⠀⢀⡾⠁⢠⣾⡟⠁⠀⠀⠀⣠⣾⣿⣿⣷⣄⠀⠀⠀⠀⠀⠀⠀⠈⢷⡀⠀⠀", 3);
  printNewLine();
  printAnimated("⢀⣾⠁⣠⣿⠏⠀⠀⠀⠀⢰⣿⣿⣿⣿⣿⣿⡆⠀⠀⠀⠀⠀⠀⠀⠈⣷⡀⠀", 3);
  printNewLine();
  printAnimated("⢸⡇⠀⣿⠏⠀⠀⢀⣴⣷⡀⠻⣿⣿⣿⣿⠟⢀⣾⣦⡀⠀⠀⠀⠀⠀⢸⡇⠀", 3);
  printNewLine();
  printAnimated("⢸⡇⠀⠀⠀⠀⠀⢸⣿⣿⣿⣦⣤⠈⠁⣤⣴⣿⣿⣿⡇⠀⠀⠀⢰⠃⢸⡇⠀", 3);
  printNewLine();
  printAnimated("⠈⢿⡀⠀⠀⠀⠀⠀⠻⠿⡿⠿⠃⠀⠀⠘⠿⢿⠿⠟⠀⠀⠀⡰⠟⢀⡿⠁⠀", 3);
  printNewLine();
  printAnimated("⠀⠈⢿⣤⡀⠀⣀⣤⣤⣤⣀⠀⠀⠀⠀⠀⠀⣀⣤⣤⣤⣀⠀⢀⣤⡿⠁⠀⠀", 3);
  printNewLine();
  printAnimated("⠀⠀⠀⠉⠛⠛⠋⣡⣤⣌⠙⠻⠶⣦⣴⠶⠟⠋⣡⣤⣌⠙⠛⠛⠉⠀⠀⠀⠀", 3);
  printNewLine();
  printAnimated("⠀⠀⠀⠀⠀⠀⣾⣿⣿⣿⢀⣴⣶⠄⠠⣶⣦⡀⣿⣿⣿⣷⠀⠀⠀⠀⠀⠀⠀", 3);
  printNewLine();
  printAnimated("⠀⠀⠀⠀⠀⠀⢿⣿⣿⡏⠈⢿⣿⠀⠀⣿⡿⠁⢹⣿⣿⡿⠀⠀⠀⠀⠀⠀⠀", 3);
  printNewLine();
  printAnimated("⠀⠀⠀⠀⠀⠀⠈⠻⣿⣿⠀⠀⠉⠀⠀⠉⠀⠀⣿⣿⠟⠁⠀⠀⠀⠀⠀⠀⠀", 3);
  printNewLine();
  printAnimated(FG_GREEN "⠀⠀⠀⠀⠀⠀⠀⠀⠀⠉⠀⠀⠀⠀⠀⠀⠀⠀⠉⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀" RESET_COLOR, 3);
  printNewLine();
}

void printPikachu() {
  printAnimated(FG_YELLOW "⢀⣠⣾⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⠀⠀⠀⠀⣠⣤⣶⣶", 1);
  printNewLine();
  printAnimated("⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⠀⠀⠀⢰⣿⣿⣿⣿", 1);
  printNewLine();
  printAnimated("⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣧⣀⣀⣾⣿⣿⣿⣿", 1);
  printNewLine();
  printAnimated("⣿⣿⣿⣿⣿⡏⠉⠛⢿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡿⣿", 1);
  printNewLine();
  printAnimated("⣿⣿⣿⣿⣿⣿⠀⠀⠀⠈⠛⢿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⠿⠛⠉⠁⠀⣿", 1);
  printNewLine();
  printAnimated("⣿⣿⣿⣿⣿⣿⣧⡀⠀⠀⠀⠀⠙⠿⠿⠿⠻⠿⠿⠟⠿⠛⠉⠀⠀⠀⠀⠀⣸⣿", 1);
  printNewLine();
  printAnimated("⣿⣿⣿⣿⣿⣿⣿⣷⣄⠀⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⣴⣿⣿", 1);
  printNewLine();
  printAnimated("⣿⣿⣿⣿⣿⣿⣿⣿⣿⠏⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠠⣴⣿⣿⣿⣿", 1);
  printNewLine();
  printAnimated("⣿⣿⣿⣿⣿⣿⣿⣿⡟⠀⠀⢰⣹⡆⠀⠀⠀⠀⠀⠀⣭⣷⠀⠀⠀⠸⣿⣿⣿⣿", 1);
  printNewLine();
  printAnimated("⣿⣿⣿⣿⣿⣿⣿⣿⠃⠀⠀⠈⠉⠀⠀⠤⠄⠀⠀⠀⠉⠁⠀⠀⠀⠀⢿⣿⣿⣿", 1);
  printNewLine();
  printAnimated("⣿⣿⣿⣿⣿⣿⣿⣿⢾⣿⣷⠀⠀⠀⠀⡠⠤⢄⠀⠀⠀⠠⣿⣿⣷⠀⢸⣿⣿⣿", 1);
  printNewLine();
  printAnimated("⣿⣿⣿⣿⣿⣿⣿⣿⡀⠉⠀⠀⠀⠀⠀⢄⠀⢀⠀⠀⠀⠀⠉⠉⠁⠀⠀⣿⣿⣿", 1);
  printNewLine();
  printAnimated("⣿⣿⣿⣿⣿⣿⣿⣿⣧⠀⠀⠀⠀⠀⠀⠀⠈⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢹⣿⣿", 1);
  printNewLine();
  printAnimated(FG_YELLOW "⣿⣿⣿⣿⣿⣿⣿⣿⣿⠃⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢸⣿⣿" RESET_COLOR, 1);
  printNewLine();
}

void printTyping(char txt[]) { printAnimated(txt, 60); }
void printTypingWithColor(char txt[], char color[]) {
  printAnimatedWithColor(txt, color, 40);
}
void printSlow(char txt[]) { printAnimated(txt, 1000); }
void printNewLine() { printf("\n"); }
void simulateLoading(char start[], char middle[], char end[], char color[]) {
  printTyping(start);
  printSlow(middle);
  printTypingWithColor(end, color);
}

void printAnimatedWithColor(char txt[], char color[], int ms_duration) {
  printf("%s", color);
  char current = txt[0];
  for (int i = 0; current != *"\0"; i++) {
    current = txt[i];
    for (int j = 0; j < i + 1; j++) {
      printf("%c", txt[j]);
    }
    fflush(stdout);
    usleep(ms_duration * 1000);

    if (current == *"\0") {
      break;
    }

    for (int j = 0; j < i + 1; j++) {
      printf("\b");
    }
  }

  printf(RESET_COLOR);
}

void printAnimated(char txt[], int ms_duration) {
  char current = txt[0];
  for (int i = 0; current != *"\0"; i++) {
    current = txt[i];
    for (int j = 0; j < i + 1; j++) {
      printf("%c", txt[j]);
    }
    fflush(stdout);
    usleep(ms_duration * 1000);

    if (current == *"\0") {
      break;
    }

    for (int j = 0; j < i + 1; j++) {
      printf("\b");
    }
  }
}

void clearConsole() {
  printf("\033[H\033[2J");
  fflush(stdout);
}
