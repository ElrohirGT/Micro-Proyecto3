#include <omp.h>
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

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printIntro();
  }
  int velocidades_empleados[256];
  int velocidad_dron = 0;

  // ---largo----
  //            |
  //            |
  //            ancho
  //            |
  //            |
  int largo_parcela = 0;
  int ancho_parcela = 0;
  int ticks_por_segundo = 1;
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

  int empleados_terminaron = 0;
  int dron_termino = 0;

  int dron_tick_count = 0;
  int empleados_tick_count = 0;

  int parcela_empleados[largo_parcela * ancho_parcela];
  int parcela_dron[largo_parcela * ancho_parcela];
  uint wait_microseconds = 1000000 / ticks_por_segundo;
  int total_a_fumigar = largo_parcela * ancho_parcela;

  fillMatrix(parcela_dron, largo_parcela, ancho_parcela);
  fillMatrix(parcela_empleados, largo_parcela, ancho_parcela);

  int velocidad_conjunta_empleados = 0;
#pragma omp parallel for reduction(+ : velocidad_conjunta_empleados)
  for (int empleadoI = 0; empleadoI < cuenta_empleados; empleadoI++) {
    velocidad_conjunta_empleados += velocidades_empleados[empleadoI];
  }

  printf("Los empleados en conjunto fumigan %d secciones por tick\n",
         velocidad_conjunta_empleados);

#pragma omp parallel sections shared(empleados_terminaron, dron_termino,       \
                                     parcela_dron, parcela_empleados)
  {
#pragma omp section
    {
      // Mostrar matrices...
      int frame_count = 0;
      while (!empleados_terminaron || !dron_termino) {
        clearConsole();
        printf("\033[92mParcela Empleados:\n");
        printMatrix(parcela_empleados, ancho_parcela, largo_parcela);

        printf("\033[92mParcela Dron:\n");
        printMatrix(parcela_dron, ancho_parcela, largo_parcela);
        printf("Frame %d...\n", ++frame_count);
        usleep(1000000 / 60); // 60 frames per second.
      }
      // Mostrando estado final...
      clearConsole();
      printf("\033[92mParcela Empleados:\n");
      printMatrix(parcela_empleados, ancho_parcela, largo_parcela);

      printf("\033[92mParcela Dron:\n");
      printMatrix(parcela_dron, ancho_parcela, largo_parcela);
      printf("Frame %d...\n", ++frame_count);

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
    }

#pragma omp section
    {
      int seccion_sin_fumigar = -1;
      // Drone fumiga parcela...
      while (!dron_termino) {
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
      }
    }

#pragma omp section
    {
      int seccion_sin_fumigar = -1;

      while (!empleados_terminaron) {
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
      }
    }
  }

  return 0;
}

void printMatrix(int arr[], int height, int width) {
  for (int i = 0; i < height; i++) {
    for (int j = 0; j < width; j++) {
      int index = i * width + j;
      int should_color = arr[index];

      if (should_color) {
        printf(FG_BLUE "1" RESET_COLOR);
      } else {
        printf(FG_RED "0" RESET_COLOR);
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
  printTyping(" thank you for using our "
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
  printAnimatedWithColor(txt, color, 60);
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
