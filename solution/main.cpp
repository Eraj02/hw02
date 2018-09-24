/*******************************************************************************
Author :      Waqar Saleem
Email  :      waqar.saleem@sse.habib.edu.pk
File   :      main.cpp
Date   :      21 Sep 2018

Solves the dungeon game given in Assignment 2 of CS 224 OOP, Fall 2018.

Credits: Borrows heavily from the solution provided by Umair Azfar Khan.
*******************************************************************************/

//===== Headers. ===============================================================
// C headers come first in alphabetical order.
#include <cstdlib>
#include <ctime>
// C++ headers come next in alphabetical order.
#include <iostream>

//===== Structures. ============================================================
// A point keeps track of locations.
struct Point
{
  int x, y;
  Point(int _x, int _y): x(_x), y(_y) {}
};
//======================================

// The Player.
struct Player
{
  // Status information.
  int x, y;  // Location.
  int food, health;
  bool is_alive;

  // Initialize player with reasonable food and health.
  Player() : food(64), health(10), is_alive(true)
  {}
  // Initialize player with reasonable food and health at location.
  Player(const Point& location) : food(64), health(10), is_alive(true),
				  x(location.x), y(location.y)
  {}
  // Player dies if food reaches 0.
  void lose_food() {
    food -= (food > 0) ? 1 : 0;
    is_alive = (food == 0) ? false : is_alive;
  }
  // Player dies if health reaches 0.
  void lose_health() {
    health -= (health > 0) ? 1 : 0;
    is_alive = (health == 0) ? false : is_alive;
  }
  // Health cannot be more than 10.
  void gain_health() {
    health += (health<10) ? 1 : 0;
  }
  // Is player at location?
  bool is_at(const Point& location) {
    return x == location.x && y == location.y;
  }
  // Show status.
  void show_status() {
    std::cout << "You have " << health << " health and food for " << food
	      << " days.\n"; 
  }
};

//===== Typedefs. ==============================================================
typedef const int cint;
typedef const Point cPoint;

//===== Function prototypes. ===================================================

// Creates and returns the dungeon.
char* create_dungeon(int, int, Point&, Point&);
// Moves player inside the dungeon.
void traversal(char*, Point&, cPoint&, int, int);
// A single move.
void do_turn(char*, cint, cint, Player&);
// Process a player's move to pt.
void move_player_to(char *dungeon, cint w, cint h, Player& p, cPoint& pt);
// Combats enemies.
void combat(Player&, int);

//===== Output. ========================
// Shows statements corresponding to trap.
void trap_statements();
// Shows statements corresponding to finding food.
void food_statements();
// Shows statements corresponding to hitting an enemy.
void hit_statements();
// Shows statements corresponding to getting hit.
void get_hit_statements();

//===== Helpers. =======================
// Gets dungeon dimensions from user and saves.
void input_dimenions(int& width, int& height);
// Generates and returns object to be placed in the dungeon.
char generate_dungeon_object();
// Places letter in the dungeon in a row in the given column if
// possible, otherwise in default_row. Returns the row in which letter
// was placed.
int place_in_column(char* dungeon, cint w, cint h, cint column,
		    char letter, int default_row);
// Prints dungeon.
void print_dungeon(char*, cint, cint);
// Returns the letter/object at point in the dungeon.
char get_object(char* dungeon, cint w, cint h, cPoint& point);
char get_object(char* dungeon, cint w, cint h, cint x, cint y);
// Sets the letter/object at point in the dungeon to letter.
void set_object(char* dungeon, cint w, cint h, cPoint& point, char letter);
void set_object(char* dungeon, cint w, cint h, cint x, cint y, char letter);

//===== Main program. ==========================================================

int main(int argc, char** argv)
{
  // Seed random number generator.
  srand(time(0));
  // Get dungeon dimensions and create the dungeon.
  int width, height;
  input_dimenions(width, height);
  Point start_point(0,0);
  Point exit_point(0,0);
  char* dungeon = create_dungeon(width, height, start_point, exit_point);
  traversal(dungeon, start_point, exit_point, width, height);

  delete [] dungeon;
  
  return 0;
}

//===== Function bodies. =======================================================

// Get dungeon dimensions from user and save.
void input_dimenions(int& width, int& height) {
  std::cout << "Enter the width and size of the dungeon you want to play in:\n";
  std::cout << "Width: ";
  std::cin >> width;
  std::cout << "Height: ";
  std::cin >> height;
  if (width < 8 || height < 8) {
    std::cout << "Invalid values. Declaring the default 16 x 16 dungeon\n";
    width = height = 16;
  }
}
//======================================

char* create_dungeon(int width, int height,
		     Point& player_loc, Point& exit_loc)
{
  // Create empty dungeon.
  int length = width*height;
  char* dungeon = new char[length];
  std::fill_n(dungeon, length, ' ');
  // Place top and bottom walls.
  char* d = dungeon;
  std::fill_n(d, width, 'w');
  d += length-width;
  std::fill_n(d, width, 'w');
  // Place left and right walls.
  for (d = dungeon+width; d < dungeon+length-width; d += width) {
    *d = *(d+width-1) = 'w';
  }
  // Place objects in the dungeon.
  for (d = dungeon+width+1; d < dungeon+length-width; d += width) {
    std::generate_n(d, width-2, generate_dungeon_object);
  }
  // Place the player and the exit in the dungeon.
  player_loc.x = 1;
  player_loc.y = place_in_column(dungeon, width, height, 1, 'P', 1);
  exit_loc.x = width-2;
  exit_loc.y = place_in_column(dungeon, width, height, width-2, 'X', height-2);
  return dungeon;
}
//======================================

// Places letter in the dungeon in a row in the given column if
// possible, otherwise in default_row. Returns the row in which letter
// was placed.
int place_in_column(char* dungeon, int width, int height, int column,
		    char letter, int default_row) {
  // Count vacant spots in column excluding the wall in the top and
  // bottom rows.
  int length = width * height;
  int n_vacant = 0;
  char* d = dungeon + width + column;
  for (; d < dungeon + length - width; d += width) {
    n_vacant += (*d == ' ') ? 1 : 0;
  }
  // Place letter with equal probability in any of the vacant
  // spots.
  bool is_placed = false;
  int placed_row;
  if (n_vacant > 0) {
    float placement_probability = 1.0 / n_vacant;
    d = dungeon + width + column;
    while (d < dungeon+length-width && not is_placed) {
      if (*d == ' ' && rand()/float(RAND_MAX) < placement_probability) {
	*d = letter;
	placed_row = (d-dungeon)/width;
	is_placed = true;
      }
      d += width;
    }
  }
  // Place letter in default_row if not yet placed.
  if (not is_placed) {
    dungeon[width*default_row+column] = letter;
    placed_row = default_row;
    is_placed = true;
  }
  return placed_row;
}
//======================================

// Generate and return object to be placed in the dungeon.
char generate_dungeon_object()
{
  // 20% chance of encountering an object.
  int random = rand() % 100;
  if (random >= 20) {
    return ' ';
  }
  /**
     Objects generated randomly as follows:
     [ 0,  15) - enemy
     [15,  30) - health
     [30,  45) - trap
     [45,  60) - food
     [60, 100)  - wall
  */
  random = rand() % 100;
  return "EHTFwww"[random/15];
}
//======================================

// Print dungeon.
void print_dungeon(char* dungeon, int width, int height) {
  for (char* d = dungeon; d < dungeon + width*height; d+=width) {
    std::copy(d, d+width, std::ostream_iterator<char>(std::cout,""));
    std::cout << "\n";
  }
}
//======================================

void trap_statements()
{
    const char* l[] =
      {"You walked into some spikes that sprung out of the floor.",
       "You stepped into a bear trap and got yourself injured.",
       "An arrow flew out of a nearby wall and hit you in your posterior. That will leave a scar!"};
    std::cout << l[rand()%3] << "\n";
}
//======================================

void food_statements()
{
  const char* l[] =
    {"You looked at the food and it was a well cooked chicken ... well at least it looked like one.",
     "It is a bread roll in this dungeon? Maybe there is a secret bakery around here.",
     "It is a rat as big as a rabbit. It will go down well with a bit of wasp juice."};
  std::cout << l[rand()%3] << "\n";
}
//======================================

void hit_statements()
{
  const char* l[] =
    {"You made an excellent jab that knocked the lights out of your enemy.",
     "Your speed is unmatched and delivered a Stone Cold Stunner.",
     "Your roundhouse kick sent your enemy flying right into a gutter."};
  std::cout << l[rand()%3] << "\n";
}
//======================================

void get_hit_statements()
{
  const char* l[] =
    {"The enemy avoided your attack and gave you a nasty scratch.",
     "You were not prepared for a sudden lunging attack that hit you hard.",
     "The enemy threw a rock that hit you on the temple and shook you bad."};
  std::cout << l[rand()%3] << "\n";
}
//======================================

// Moves player inside the dungeon.
void traversal(char* dungeon, Point& start_point, cPoint& exit_point, int width, int height) {
  // Create player, show initial message.
  Player p(start_point);
  std::cout << "After being captured by a raid of some robbers on your caravan,\n"
	    << "you find yourself alone in a dark dungeon. With nothing but your\n"
	    << "wits, you choose to take a step...\n\n";
  print_dungeon(dungeon, width, height);
  // Make turns until game ends, i.e. either player wins or player dies.
  while (p.is_alive && not p.is_at(exit_point)) {
    p.show_status();
    do_turn(dungeon, width, height, p);
    print_dungeon(dungeon, width, height);
  }
  // Print message corresponding to exit condition.
  if (p.is_at(exit_point)) {
    std::cout << "***********************************************************\n"
	      << "*********   You found the exit... You are free!   *********\n"
	      << "***********************************************************\n";
  }
  else {
    std::cout << "*************************************************\n"
	      << "*****************   You Died!   *****************\n"
	      << "*************************************************\n";  
  }
}
//======================================

// A single turn in the dungeon.
void do_turn(char* dungeon, cint width, cint height, Player& player)
{
  // Get move from user.
  char move = ' ';
  std::cout << "In which direction do you want to move?\n"
	    << "(U,D,L,R; Press X if you want to give up and die.)\n";
  std::cin >> move;
  // Compute destination for move.
  Point destination(player.x, player.y);
  if (move == 'u' or move == 'U') {
    destination.y--;
  }
  else if (move == 'd' or move == 'D') {
    destination.y++;
  }
  else if (move == 'l' or move == 'L') {
    destination.x--;
  }
  else if (move == 'r' or move == 'R') {
    destination.x++;
  }
  else if (move == 'x' or move == 'X') {
    player.is_alive = false;
  }
  else {
    std::cout << "Incorrect move. You just wasted a turn.\n\n";
  }
  // Process the move. Take care not to move past the edge.
  if (destination.x == 0 or destination.x == width-1 or 
      destination.y == 0 or destination.y == height-1) {
    std::cout << "You are at the edge of the room\n\n";
  }
  else if (player.is_alive) {
    move_player_to(dungeon, width, height, player, destination);
  }
  // Lose food every turn.
  player.lose_food();
}
//======================================

// Process the player's move to pt.
void move_player_to(char *dungeon, cint w, cint h, Player& player, cPoint& pt)
{
  std::cout << "Player: " << player.x << ", " << player.y << "\n";
  std::cout << "Destination: " << pt.x << ", " << pt.y << "\n";
  // Get the current status of the destiantion. Assume a successful move.
  char c = get_object(dungeon, w, h, pt);
  bool moved = true;
  // Process the move to the destination. Nothing to do here if the
  // destination is the exit.
  if (c == ' ') {  // empty space - move player.
    std::cout << "There is nothing here.\n";
  }
  else if (c == 'w') {  // wall - cannot move.
    std::cout << "There is a wall there, you cannot move.\n";
    moved = false;
  }
  else if (c == 'H') {  // H - gain health and move player.
    player.gain_health();
    std::cout << "You found some health.\nYour current health is: "
	      << player.health <<'\n';
  }
  else if (c == 'T') {  // trap - trap statements and move player.
    trap_statements();
    player.lose_health();
    std::cout << "Your current health is: " << player.health << '\n';
  }
  else if (c == 'F') {  // food - increment food for 4 to 8 days and move player.
    int food = 4 + (rand()%5); 
    player.food += food;
    std::cout << "You found some food that will last you for " << food
	      << " more days\n.";
    food_statements();
  }
  else if (c == 'E') {  // enemies - combat 2 to 4 enemies and move player if alive.
    int enemies = 2 + (rand()%3); 
    std::cout << "You came across " << enemies << " enemies. "
	      << "You will have to fight.\n";
    combat(player, enemies);
  }
  // Update dungeon and player position for the move.
  if (moved) {
    set_object(dungeon, w, h, player.x, player.y, ' ');
    if (player.is_alive) {
      set_object(dungeon, w, h, pt, 'P');
      player.x = pt.x;
      player.y = pt.y;
    }
  }    
}
//======================================

void combat(Player& player, int enemies)
{
  std::cout << "*************************************************\n"
	    << "**************   Start of Combat   **************\n"
	    << "*************************************************\n\n";
  // Combat until player dies or all enemies are eliminated.
  while (enemies && player.is_alive) {
    // 30% chance for player to hit enemy.
    if (rand() % 100 < 30) {
	hit_statements();
	std::cout << "^_^ You killed one enemy!\n";
	enemies--;
    }
    // 10% chance for enemy to hit the player.
    for (int i = 0; i < enemies; i++) {
      if(rand()%100 < 10) {
	get_hit_statements();
	std::cout << "x_x You lost 1 health\n";
	player.lose_health();
      }
    }
  }
  std::cout << "\n*************************************************\n"
	    << "***************   end of Combat   ***************\n"
	    << "*************************************************\n\n";
}
//======================================

// Returns the letter/object at point in the dungeon.
char get_object(char* dungeon, cint w, cint h, cPoint& point)
{
  return get_object(dungeon, w, h, point.x, point.y);
}
//======================================

// Returns the letter/object at point in the dungeon.
char get_object(char* dungeon, cint w, cint h, cint x, cint y)
{
  return dungeon[y*w+x];
}
//======================================

// Sets the letter/object at point in the dungeon to letter.
void set_object(char* dungeon, cint w, cint h, cPoint& point, char letter)
{
  set_object(dungeon, w, h, point.x, point.y, letter);
}
//======================================

// Sets the letter/object at point in the dungeon to letter.
void set_object(char* dungeon, cint w, cint h, cint x, cint y, char letter)
{
  dungeon[y*w+x] = letter;
}
//======================================

