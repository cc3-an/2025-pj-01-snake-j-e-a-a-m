#include <stdio.h>
#include <string.h>

#include "snake_utils.h"
#include "state.h"

int main(int argc, char* argv[]) {
  char* in_filename = NULL;
  char* out_filename = NULL;
  game_state_t* state = NULL;

  // Parsea los argumentos recibidos
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-i") == 0 && i < argc - 1) {
      in_filename = argv[i + 1];
      i++;
      continue;
    }
    if (strcmp(argv[i], "-o") == 0 && i < argc - 1) {
      out_filename = argv[i + 1];
      i++;
      continue;
    }
    fprintf(stderr, "Usage: %s [-i filename] [-o filename]\n", argv[0]);
    return 1;
  }

  // NO MODIFIQUEN NADA ARRIBA DE ESTA LINEA.

  /* Tarea 7 */

  // Validar argumento de entrada
  if (in_filename == NULL) {
    return -1;
  }

  // Cargar el tablero desde el archivo
  state = load_board(in_filename);
  if (state == NULL) {
    return -1;
  }

  // Inicializar estructuras de serpientes
  state = initialize_snakes(state);
  if (state == NULL) {
    free_state(state);
    return -1;
  }

  // Avanzar el estado del juego un paso de tiempo (deterministic_food esta implementada en snakes_utils.h)
  update_state(state, deterministic_food);

  // Guardar o imprimir el tablero actualizado
  if (out_filename != NULL) {
    save_board(state, out_filename);
  } else {
    print_board(state, stdout);
  }

  // Liberar toda la memoria
  free_state(state);
  return 0;
}
