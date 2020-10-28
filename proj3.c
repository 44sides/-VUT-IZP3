/**
 * @file:       proj3.c
 * @date:       14.12.2019
 * @author:     Vladyslav Tverdokhlib, xtverd01@stud.fit.vutbr.cz
 * Projekt 3:   Prace s datovymi strukturami
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// tisk napovedy pouzivani programu
void help_output()
{
  printf("--test name.txt\n Kontrola bludiste\n");
  printf("--rpath R C name.txt\n Pruchod bludistem na vstupu na radku R a sloupci C pomoci pravidla prave ruky\n");
  printf("--lpath R C name.txt\n Pruchod bludistem na vstupu na radku R a sloupci C pomoci pravidla leve ruky\n");
}

#define UPWARD (1)
#define DOWNWARD (0)

#define LEFTHAND (0)
#define RIGHTHAND (1)

// Tisk vstupnich souradnic a vsechno
#define PRINT_ONCE (printf("%d,%d\n", *r + 1, *c + 1))

// Rozsahy uhlu a stran, kterymi muzeme vstoupit do bludiste
#define TOP_LEFT ((*r == 0) && (*c == 0))
#define TOP_RIGHT ((*r == 0) & (*c == map.cols))
#define BOTTOM_LEFT ((*r == map.rows) && (*c == 0))
#define BOTTOM_RIGHT ((*r == map.rows) && (*c == map.cols))
#define LEFT_COLUMN ((*r > 0 && *r < map.rows) && (*c == 0))
#define RIGHT_COLUMN ((*r > 0 && *r < map.rows) && (*c == map.cols))
#define TOP_ROW ((*r == 0) && (*c > 0 && *c < map.cols))
#define BOTTOM_ROW ((*r == map.rows) && (*c > 0 && *c < map.cols))

#define ERROR_INPUT (printf("ERROR: Bad input data\n"))
#define ERROR_FILE (printf("ERROR: File don't exist\n"))
#define ERROR_MAZE (printf("ERROR: Invalid maze\n"))
#define ERROR_CLOSED (printf("ERROR: No entrance to the maze\n"))

// Bludiste struct. *cells obsahuje prkvy bludiste
typedef struct {
  int rows;
  int cols;
  unsigned char *cells;
} Map;

// Policko/bunka struct
typedef struct {
  int r;
  int c;
  int edge;
} Cell;

// TRUE soubor existuje
// FALSE soubor nexistuje
bool file_check(char *fname)
{
  FILE *fp;
  fp = fopen(fname, "r");

  if (fp == NULL)
    return false;

  fclose(fp);
  return true;
}

// nacteni struktury Map
Map map_fill(char *fname)
{
  Map m;
  m.rows = 0; m.cols = 0;
  char sym[11]; // pole pro prvky v souboru (maximalne deset cislic). sym[10]='\n'

  FILE *fp; fp = fopen(fname, "r");

  fscanf(fp, "%s", sym); m.rows = atoi(sym); // prevod do int
  fscanf(fp, "%s", sym); m.cols = atoi(sym); // prevod do int

  m.cells = malloc(m.rows*m.cols * sizeof(char)); // pamet'

  for (int i = 0; i < m.rows*m.cols; i++)
  {
    fscanf(fp, "%s", sym);

    if(sym[0] >= 48 && sym[0] <= 55 && sym[1] == '\0') // kontrola pro 'testmaze_values'
      m.cells[i] = atoi(sym); // kdyz jednotlivy prvek 0-7
    else
      m.cells[i] = 10; // jinak se rovna cislu, ktere je zakazano => overeni se nezdari v 'testmaze_values'
  }

  fclose(fp);
  return m;
}

// TRUE sousedici policka MAJI stejny typ sdilene hranice
bool testmaze_common(Map *map)
{
  char left, right;

  for (int t = 0; t < map->rows; t++)
  {
    for (int i = 1; i < map->cols; i++)
    {
      left = map->cells[(map->cols*t + i) - 1] >> 1; // presuneme binarni kod doprava tak, aby byla potrebni posledni cislice
      right = map->cells[(map->cols*t + i)];

      // oddelime posledni hodnotu v binarnim formatu
      left = left % 2;
      right = right % 2;

      if (left != right)
        return false;
    }
  }
  return true;
}
// TRUE hodnoty od 0 do 7
bool testmaze_values(Map *map)
{
  for (int i = 0; i <= map->rows*map->cols; i++)
  {
    if (map->cells[i] > 7)
      return false;
  }
  return true;
}

// prevod strany (0, 1, 2) do steny (001, 010, 100)
char edge_to_wall(int edge)
{
  return (1 << edge);
}

// TRUE strana 'edge' v [r; c] je stena
bool isborder(Map map, int r, int c, int edge)
{
  char wall;
  wall = edge_to_wall(edge);

  if ((map.cells[map.cols*r + c] & wall) != 0) // binarni &
    return true;
  else
    return false;
}

// RETURN DOWNWARD trojuhelnik dolu
// RETURN UPWARD trojuhelnik nahoru
int orientation(int row, int col)
{
  if((row + col) % 2 == 0)
    return DOWNWARD;
  else
    return UPWARD;
}

// TRUE mozno vstoupit do blusdiste pres trojuhelnik [r; c]
// FALSE(1) mozno vstoupit a okamzite vystoupit (napise jenom vstupni souradnice)
// FALSE(2) nelze vstoupit
bool first_cell_checkup(Map map, int *r, int *c)
{
  map.rows--; map.cols--; // pro pohodli

  int orien = orientation(*r, *c);

  if(map.cells[map.cols * *r + *c] == 7){ // trojuhelnik je zavreny ze vsech stran
    ERROR_CLOSED; return false;}

  if (TOP_LEFT) // v leve horni casti mapy
  {
    map.rows++; map.cols++; // pro 'isborder'
    if(isborder(map, *r, *c, 0) && isborder(map, *r, *c, 2)){ // vchod zcela je uzavren se stenami v 0 a 2 v leve horni casti
      ERROR_CLOSED; return false;}
    if(isborder(map, *r, *c, 1)) // vstoupime a hned vystoupime
    {
      PRINT_ONCE;
      return false;
    }

    return true;
  }

  if (LEFT_COLUMN) // --||--
  {
    map.rows++; map.cols++;
    if(isborder(map, *r, *c, 1) && isborder(map, *r, *c, 2))
    {
      PRINT_ONCE;
      return false;
    }
    if(isborder(map, *r, *c, 0)){
      ERROR_CLOSED; return false;}

    return true;
  }

  if (BOTTOM_LEFT) // --||--
  {
    map.rows++; map.cols++;
    if(orien == DOWNWARD) // pripad pro DOWNWARD
    {
      if(isborder(map, *r, *c, 1) && isborder(map, *r, *c, 2))
      {
        PRINT_ONCE;
        return false;
      }
      if(isborder(map, *r, *c, 0)){
        ERROR_CLOSED; return false;}
    }
    if(orien == UPWARD) // pripad pro UPWARD
    {
      if(isborder(map, *r, *c, 0) && isborder(map, *r, *c, 2)){
        ERROR_CLOSED; return false;}
      if(isborder(map, *r, *c, 1))
      {
        PRINT_ONCE;
        return false;
      }
    }

    return true;
  }

  if (TOP_RIGHT) // --||--
  {
    map.rows++; map.cols++;
    if(orien == DOWNWARD) // --||--
    {
      if(isborder(map, *r, *c, 1) && isborder(map, *r, *c, 2)){
        ERROR_CLOSED; return false;}
      if(isborder(map, *r, *c, 0))
      {
        PRINT_ONCE;
        return false;
      }
    }
    if(orien == UPWARD) // --||--
    {
      if(isborder(map, *r, *c, 0) && isborder(map, *r, *c, 2))
      {
        PRINT_ONCE;
        return false;
      }
      if(isborder(map, *r, *c, 1)){
        ERROR_CLOSED; return false;}
    }

    return true;
  }

  if (RIGHT_COLUMN) // --||--
  {
    map.rows++; map.cols++;
    if(isborder(map, *r, *c, 0) && isborder(map, *r, *c, 2))
    {
      PRINT_ONCE;
      return false;
    }
    if(isborder(map, *r, *c, 1)){
      ERROR_CLOSED; return false;}

    return true;
  }

  if (BOTTOM_RIGHT) // --||--
  {
    map.rows++; map.cols++;
    if(orien == DOWNWARD) // --||--
    {
      if(isborder(map, *r, *c, 1) && isborder(map, *r, *c, 2)){
        ERROR_CLOSED; return false;}
      if(isborder(map, *r, *c, 0))
      {
        PRINT_ONCE;
        return false;
      }
    }
    if(orien == UPWARD) // --||--
    {
      if(isborder(map, *r, *c, 0) && isborder(map, *r, *c, 2))
      {
        PRINT_ONCE;
        return false;
      }
      if(isborder(map, *r, *c, 1)){
        ERROR_CLOSED; return false;}
    }

    return true;
  }

  if (TOP_ROW) // --||--
  {
    map.rows++; map.cols++;
    if(isborder(map, *r, *c, 0) && isborder(map, *r, *c, 1))
    {
      PRINT_ONCE;
      return false;
    }
    if(isborder(map, *r, *c, 2)){
      ERROR_CLOSED; return false;}

    return true;
  }

  if (BOTTOM_ROW) // --||--
  {
    map.rows++; map.cols++;
    if(isborder(map, *r, *c, 0) && isborder(map, *r, *c, 1))
    {
      PRINT_ONCE;
      return false;
    }
    if(isborder(map, *r, *c, 2)){
      ERROR_CLOSED; return false;}

    return true;
  }

  return true;
}

// RETURN pocatecni stranu ve vstupnim trojuhelniku
int start_border(Map map, int *r, int *c)
{
  map.rows--; map.cols--;
  int orien = orientation(*r, *c);

  // Leva strana

  if(TOP_LEFT) // ignorujeme prvni policko a jdeme na dalsi
  {
    PRINT_ONCE; // napsali souradnice
    *c = *c + 1; // dalsi policko
    return 0;
  }
  else if(LEFT_COLUMN)
    return 0;
  else if(BOTTOM_LEFT)
  {
    if(orien == DOWNWARD)
      return 0;
    if(orien == UPWARD) // dalsi policko
    {
      PRINT_ONCE;
      *c = *c + 1;
      return 0;
    }
  }

  // Prava strana

  else if(TOP_RIGHT)
  {
    if(orien == DOWNWARD) // dalsi policko
    {
      PRINT_ONCE;
      *c = *c - 1;
      return 1;
    }
    if(orien == UPWARD)
      return 1;
  }
  else if(RIGHT_COLUMN)
  {
    return 1;
  }
  else if(BOTTOM_RIGHT)
  {
    if(orien == DOWNWARD)
      return 1;
    if(orien == UPWARD) // dalsi policko
    {
      PRINT_ONCE;
      *c = *c - 1;
      return 1;
    }
  }

  // Nahore

  else if(TOP_ROW)
    return 2;

  // Dolu

  else if(BOTTOM_ROW)
    return 2;

  return 0;
}

// RETURN hodnotu, ktera presune na dalsi stranu v trojuhelniku, v zavislosti na pravidle a orientaci
int sum_edge(int orientation, int leftright)
{
  if(leftright == RIGHTHAND)
  {
    if(orientation == DOWNWARD)
      return 1;
    else // UPWARD
      return -1;
  }
  else // LEFTHAND
  {
    if(orientation == DOWNWARD)
      return -1;
    else // UPWARD
      return 1;
  }
}

// prechod do dalsiho policka
void go(Cell *cell, int orientation)
{
  // Zmena souradnic trojuhelniku

  if(cell->edge == 0) // posun <--
    cell->c--;
  else if(cell->edge == 1) // posun -->
    cell->c++;
  else if(cell->edge == 2) // posun nahoru nebo dolu
  {
    if(orientation == DOWNWARD) // posun nahoru
      cell->r--;
    else // UPWARD // posun dolu
      cell->r++;
  }

  // Zmena stran trojuhelniku

  if(cell->edge == 0) // 0 -> 1
    cell->edge++;
  else if(cell->edge == 1) // 1 -> 0
    cell->edge--;
// else if edge = 0, se nezmeni
}

// proces cesty
void path(Map *map, Cell *cell, int leftright)
{
  int orien;

  while(cell->r >= 0 && cell->r < map->rows && cell->c >= 0 && cell->c < map->cols) // dokud jsme v mape
  {
    printf("%d,%d\n", cell->r + 1, cell->c + 1);

    orien = orientation(cell->r, cell->c); // definice orientaci

    while(1) // hleadame otevrenou stranu trojuhelnika
    {
    cell->edge = (cell->edge + sum_edge(orien, leftright) + 3) % 3; // prechod na jinou stranu trojuhelnika, zalezi na orientace a pravidlu

    if(!isborder(*map, cell->r, cell->c, cell->edge)) // otevrena strana je nalezena
    {
      go(cell, orien); // posun na dalsi policko
      break;
    }
    }
  }
}

// Kontrola argumentu row a col
bool rowcols_check(Map map, int r, int c)
{
// Nahore

  if(r == 0 && (c % 2 == 0 || c == map.cols - 1)) // prvni, sude nebo posledni
    return true;

// Leva strana bludiste nebo prava strana bludiste, uprostred

  if((r > 0 && r < map.rows - 1) && (c == 0 || c == map.cols - 1))
    return true;

// Dolu

  if(r == map.rows - 1) // if Dolu
  {
    if((map.rows - 1) % 2 == 1) // if prvni UPWARD
    {
      if(c % 2 == 0) // => sude
        return true;
    }
    else if((map.rows - 1) % 2 == 0) // if prvni DOWNWARD
    {
      if(c % 2 == 1) // => sude
        return true;
    }
    else if(c == map.cols - 1) // posledni
      return true;
  }

  return false;
}


int main (int argc, char *argv[])
{
  Map map;
  Cell cell;
  if (argc != 1)
  {

  if((strcmp(argv[1], "--help") == 0) && argc == 2) // if argument --help a je jediny
    help_output();

  else if((strcmp(argv[1], "--test") == 0) && argc == 3) // if argument --test a jsou dva argumenty
  {
    if(file_check(argv[2]))
    {
    map = map_fill(argv[2]);
    if(testmaze_common(&map) == true && testmaze_values(&map) == true) // test definice bludiste
      printf("Valid\n");
    else
      printf("Invalid\n");
    }
    else
      ERROR_FILE;
  }

  else if((strcmp(argv[1], "--rpath") == 0) && argc == 5) // if argument --rpath a jsou ctyri argumenty
  {
    cell.r = atoi(argv[2]); cell.r--; // pro zero-bazed prace
    cell.c = atoi(argv[3]); cell.c--; // pro zero-bazed prace

    if(file_check(argv[4]))
    {
    map = map_fill(argv[4]);
    if(testmaze_common(&map) == true && testmaze_values(&map) == true) // test definice bludiste
    {
    if(rowcols_check(map, cell.r, cell.c)) // kontrola argumentu
    {
      if(first_cell_checkup(map, &cell.r, &cell.c) == true) // overeni vstupni bunky
      {
        cell.edge = start_border(map, &cell.r, &cell.c); // nastaveni vstupniho bordera
        path(&map, &cell, RIGHTHAND); // proces cesty
      }
    }
    else
      ERROR_INPUT;
    }
    else
      ERROR_MAZE;
    }
    else
      ERROR_FILE;
  }

  else if((strcmp(argv[1], "--lpath") == 0) && argc == 5) // if argument --lpath a jsou ctyri argumenty
  {
    cell.r = atoi(argv[2]); cell.r--; // pro zero-bazed prace
    cell.c = atoi(argv[3]); cell.c--; // pro zero-bazed prace

    if(file_check(argv[4]))
    {
    map = map_fill(argv[4]);
    if(testmaze_common(&map) == true && testmaze_values(&map) == true) // test definice bludiste
    {
    if(rowcols_check(map, cell.r, cell.c)) // kontrola argumentu
    {
      if(first_cell_checkup(map, &cell.r, &cell.c) == true) // overeni vstupni bunky
      {
        cell.edge = start_border(map, &cell.r, &cell.c); // nastaveni vstupniho bordera
        path(&map, &cell, LEFTHAND); // proces cesty
      }
    }
    else
      ERROR_INPUT;
    }
    else
      ERROR_MAZE;
    }
    else
      ERROR_FILE;
  }
  else
    ERROR_INPUT;

  }
  else
    printf("Type anything!\n");

  free(map.cells); map.rows = 0; map.cols = 0; map.cells = NULL; // uvolneni prvku struktury mapy
  return 0;
}
