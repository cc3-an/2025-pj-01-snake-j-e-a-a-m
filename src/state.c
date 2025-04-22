#define _POSIX_C_SOURCE 200809L
#include "state.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "snake_utils.h"
#include <unistd.h>
#include <sys/types.h>

// Definiciones de funciones de ayuda.
static void set_board_at(game_state_t* state, unsigned int row, unsigned int col, char ch);
static bool is_tail(char c);
static bool is_head(char c);
static bool is_snake(char c);
static char body_to_tail(char c);
static char head_to_body(char c);
static unsigned int get_next_row(unsigned int cur_row, char c);
static unsigned int get_next_col(unsigned int cur_col, char c);
static void find_head(game_state_t* state, unsigned int snum);
static char next_square(game_state_t* state, unsigned int snum);
static void update_tail(game_state_t* state, unsigned int snum);
static void update_head(game_state_t* state, unsigned int snum);

/* Tarea 1 */
game_state_t* create_default_state() {
  game_state_t* state = malloc(sizeof(game_state_t));
  state->num_rows = 18;
  state->board = malloc(state->num_rows * sizeof(char*));
  const char* rows[18] = {
    "####################",
    "#                  #",
    "# d>D    *         #",
    "#                  #",
    "#                  #",
    "#                  #",
    "#                  #",
    "#                  #",
    "#                  #",
    "#                  #",
    "#                  #",
    "#                  #",
    "#                  #",
    "#                  #",
    "#                  #",
    "#                  #",
    "#                  #",
    "####################"
  };

  for(unsigned int i = 0; i < state->num_rows; i++) {
    size_t len = strlen(rows[i]);
    state->board[i] = malloc(len + 1);
    strcpy(state->board[i], rows[i]);
  }
  state->num_snakes = 1;
  state->snakes = malloc(sizeof(snake_t));
  // Serpiente inicial: cola en (2,2), cabeza en (2,4)
  state->snakes[0].tail_row = 2;
  state->snakes[0].tail_col = 2;
  state->snakes[0].head_row = 2;
  state->snakes[0].head_col = 4;
  state->snakes[0].live = true;
  return state;
}

/* Tarea 2 */
void free_state(game_state_t* state) {
  if (!state){
    return;
  }
  
  // Liberar tablero
  for(unsigned int i = 0; i < state->num_rows; i++) {
    free(state->board[i]);
  }
  free(state->board);

  // Liberar serpientes
  if (state->snakes){
    free(state->snakes);
  }
  free(state);
}

/* Tarea 3 */
void print_board(game_state_t* state, FILE* fp) {
  for(unsigned int i = 0; i < state->num_rows; i++) {
    fprintf(fp, "%s\n", state->board[i]);
  }
}

/**
 * Guarda el estado actual a un archivo. No modifica el objeto/struct state.
 * (ya implementada para que la utilicen)
*/
void save_board(game_state_t* state, char* filename) {
  FILE* f = fopen(filename, "w");
  print_board(state, f);
  fclose(f);
}

/* Tarea 4.1 */


/**
 * Funcion de ayuda que obtiene un caracter del tablero dado una fila y columna
 * (ya implementado para ustedes).
*/
char get_board_at(game_state_t* state, unsigned int row, unsigned int col) {
  return state->board[row][col];
}


/**
 * Funcion de ayuda que actualiza un caracter del tablero dado una fila, columna y
 * un caracter.
 * (ya implementado para ustedes).
*/
static void set_board_at(game_state_t* state, unsigned int row, unsigned int col, char ch) {
  state->board[row][col] = ch;
}


/**
 * Retorna true si la variable c es parte de la cola de una snake.
 * La cola de una snake consiste de los caracteres: "wasd"
 * Retorna false de lo contrario.
*/
static bool is_tail(char c) {
  return c == 'w' || c == 'a' || c == 's' || c == 'd';
}


/**
 * Retorna true si la variable c es parte de la cabeza de una snake.
 * La cabeza de una snake consiste de los caracteres: "WASDx"
 * Retorna false de lo contrario.
*/
static bool is_head(char c) {
  return c == 'W' || c == 'A' || c == 'S' || c == 'D' || c == 'x';
}


/**
 * Retorna true si la variable c es parte de una snake.
 * Una snake consiste de los siguientes caracteres: "wasd^<v>WASDx"
*/
static bool is_snake(char c) {
  return is_tail(c) || is_head(c) || c == '^' || c == '<' || c == 'v' || c == '>';
}


/**
 * Convierte un caracter del cuerpo de una snake ("^<v>")
 * al caracter que correspondiente de la cola de una
 * snake ("wasd").
*/
static char body_to_tail(char c) {
  if (c == '^') return 'w';
  if (c == '<') return 'a';
  if (c == 'v') return 's';
  if (c == '>') return 'd';
  return c;
}


/**
 * Convierte un caracter de la cabeza de una snake ("WASD")
 * al caracter correspondiente del cuerpo de una snake
 * ("^<v>").
*/
static char head_to_body(char c) {
  if (c == 'W') return '^';
  if (c == 'A') return '<';
  if (c == 'S') return 'v';
  if (c == 'D') return '>';
  return c;
}


/**
 * Retorna cur_row + 1 si la variable c es 'v', 's' o 'S'.
 * Retorna cur_row - 1 si la variable c es '^', 'w' o 'W'.
 * Retorna cur_row de lo contrario
*/
static unsigned int get_next_row(unsigned int cur_row, char c) {
  if (c == 'v' || c == 's' || c == 'S') return cur_row + 1;
  if (c == '^' || c == 'w' || c == 'W') return cur_row - 1;
  return cur_row;
}


/**
 * Retorna cur_col + 1 si la variable c es '>' or 'd' or 'D'.
 * Retorna cur_col - 1 si la variable c es '<' or 'a' or 'A'.
 * Retorna cur_col de lo contrario
*/
static unsigned int get_next_col(unsigned int cur_col, char c) {
  if (c == '>' || c == 'd' || c == 'D') return cur_col + 1;
  if (c == '<' || c == 'a' || c == 'A') return cur_col - 1;
  return cur_col;
}


/**
 * Tarea 4.2
 *
 * Funcion de ayuda para update_state. Retorna el caracter de la celda
 * en donde la snake se va a mover (en el siguiente paso).
 *
 * Esta funcion no deberia modificar nada de state.
*/
static char next_square(game_state_t* state, unsigned int snum) {
  snake_t* sn = &state->snakes[snum];
  unsigned int hr = sn->head_row;
  unsigned int hc = sn->head_col;
  char dir = get_board_at(state, hr, hc);
  unsigned int nr = get_next_row(hr, dir);
  unsigned int nc = get_next_col(hc, dir);
  return get_board_at(state, nr, nc);
}


/**
 * Tarea 4.3
 *
 * Funcion de ayuda para update_state. Actualiza la cabeza de la snake...
 *
 * ... en el tablero: agregar un caracter donde la snake se va a mover (¿que caracter?)
 *
 * ... en la estructura del snake: actualizar el row y col de la cabeza
 *
 * Nota: esta funcion ignora la comida, paredes, y cuerpos de otras snakes
 * cuando se mueve la cabeza.
*/
static void update_head(game_state_t* state, unsigned int snum) {
  snake_t* sn = &state->snakes[snum];
  if (!sn->live) return;
  unsigned int hr = sn->head_row;
  unsigned int hc = sn->head_col;
  char head_ch = get_board_at(state, hr, hc);
  // Convertir cabeza antigua a cuerpo
  set_board_at(state, hr, hc, head_to_body(head_ch));
  // Setear posicion de cabeza
  unsigned int nr = get_next_row(hr, head_ch);
  unsigned int nc = get_next_col(hc, head_ch);
  // Colocar nueva cabeza
  set_board_at(state, nr, nc, head_ch);
  // Actualizar estructura
  sn->head_row = nr;
  sn->head_col = nc;
}


/**
 * Tarea 4.4
 *
 * Funcion de ayuda para update_state. Actualiza la cola de la snake...
 *
 * ... en el tablero: colocar un caracter blanco (spacio) donde se encuentra
 * la cola actualmente, y cambiar la nueva cola de un caracter de cuerpo (^<v>)
 * a un caracter de cola (wasd)
 *
 * ...en la estructura snake: actualizar el row y col de la cola
*/
static void update_tail(game_state_t* state, unsigned int snum) {
  snake_t* sn = &state->snakes[snum];
  if (!sn->live) return;
  unsigned int tr = sn->tail_row;
  unsigned int tc = sn->tail_col;
  char tail_ch = get_board_at(state, tr, tc);
  // Siguiente posicion de tail
  unsigned int nr = get_next_row(tr, tail_ch);
  unsigned int nc = get_next_col(tc, tail_ch);
  // Quitar tail antigua
  set_board_at(state, tr, tc, ' ');
  // Convertir cuerpo a una nueva tail
  char new_tail = body_to_tail(get_board_at(state, nr, nc));
  set_board_at(state, nr, nc, new_tail);
  // Actualizar estructura
  sn->tail_row = nr;
  sn->tail_col = nc;
}

/* Tarea 4.5 */
void update_state(game_state_t* state, int (*add_food)(game_state_t* state)) {
  for(unsigned int i = 0; i < state->num_snakes; i++) {
    snake_t* sn = &state->snakes[i];
    if (!sn->live){
      continue;
    }
    char next = next_square(state, i);
    // Colisión con pared o serpiente
    if (next == '#' || is_snake(next)) {
      // Marcar muerte
      unsigned int hr = sn->head_row;
      unsigned int hc = sn->head_col;
      set_board_at(state, hr, hc, 'x');
      sn->live = false;
      continue;
    }
    // Comer fruta
    if (next == '*') {
      update_head(state, i);
      // Agregar nueva fruta
      add_food(state);
      continue;
    }
    // Movimiento normal
    update_head(state, i);
    update_tail(state, i);
  }
}

/* Tarea 5 */
game_state_t* load_board(char* filename) {
  FILE* fp = fopen(filename, "r");
  if (!fp){
    return NULL;
  }
  game_state_t* state = malloc(sizeof(game_state_t));
  char* line = NULL;
  size_t len = 0;
  ssize_t read;
  unsigned int rows = 0;
  char** temp = NULL;
  while ((read = getline(&line, &len, fp)) != -1) {
    if (read > 0 && line[read-1] == '\n') {
      line[--read] = '\0';
    }
    temp = realloc(temp, (rows + 1) * sizeof(char*));
    temp[rows] = malloc((size_t)read + 1);
    strcpy(temp[rows], line);
    rows++;
  }
  free(line);
  fclose(fp);
  state->num_rows = rows;
  state->board = temp;
  state->num_snakes = 0;
  state->snakes = NULL;
  return state;
}


/**
 * Tarea 6.1
 *
 * Funcion de ayuda para initialize_snakes.
 * Dada una structura de snake con los datos de cola row y col ya colocados,
 * atravezar el tablero para encontrar el row y col de la cabeza de la snake,
 * y colocar esta informacion en la estructura de la snake correspondiente
 * dada por la variable (snum)
*/
static void find_head(game_state_t* state, unsigned int snum) {
  snake_t* sn = &state->snakes[snum];
  unsigned int r = sn->tail_row;
  unsigned int c = sn->tail_col;
  char ch = get_board_at(state, r, c);
  // Avanzar hasta cabeza
  while (!is_head(ch)) {
    unsigned int nr = get_next_row(r, ch);
    unsigned int nc = get_next_col(c, ch);
    r = nr; c = nc;
    ch = get_board_at(state, r, c);
  }
  sn->head_row = r;
  sn->head_col = c;
}

/* Tarea 6.2 */
game_state_t* initialize_snakes(game_state_t* state) {
  unsigned int count = 0;
  // Contar colas
  for (unsigned int i = 0; i < state->num_rows; i++) {
    size_t len = strlen(state->board[i]);
    for (unsigned int j = 0; j < len; j++) {
      if (is_tail(state->board[i][j])) {
        count++;
      }
    }
  }
  snake_t* snakes = malloc(count * sizeof(snake_t));
  unsigned int idx = 0;
  // Inicializar colas
  for (unsigned int i = 0; i < state->num_rows; i++) {
    size_t len = strlen(state->board[i]);
    for (unsigned int j = 0; j < len; j++) {
      if (is_tail(state->board[i][j])) {
        snakes[idx].tail_row = i;
        snakes[idx].tail_col = j;
        snakes[idx].live = true;
        idx++;
      }
    }
  }
  state->num_snakes = count;
  state->snakes = snakes;
  // Encontrar cabezas
  for (unsigned int i = 0; i < count; i++) {
    find_head(state, i);
  }
  return state;
}
