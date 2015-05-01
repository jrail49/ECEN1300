
/*** Includes ***/
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>

/*** Defines ***/
#define TICKRATE 100
#define TRUE 1
#define FALSE 0

/*** Globals ***/
static int WORLD_WIDTH = 50;
static int WORLD_HEIGHT = 20;

/*** Enums ***/
enum userInput {
	LEFT,
	RIGHT,
	FIRE,
	DO_NOTHING
};

/*** Structs ***/
typedef struct weapon{
	int x;
	int y;
	char bulletChar;
	struct weapon *nextBullet;
	int active;
} bullet;


typedef struct cEnemy{
	int x;
	int y;
	char ship;
	bullet attacks[];
} commonEnemy;

typedef struct user{
	int x;
	int y;
	char ship;
	bullet *firstBullet;
	bullet *lastBullet;
} player;


/*** Prototypes ***/
int move_player(WINDOW *win, int userInput, player *p);
int updateInvaders(WINDOW *win, commonEnemy invaders[]);
void drawPlayer(WINDOW *win, player *p);
void drawInvaders(WINDOW *win, commonEnemy invaders[]);
void updateWorld(WINDOW *win, int userInput, player *p);
void createEnemies(const int numberOfEnemies, commonEnemy *invaders);
void fire(player *p);

/**
Function:	main
Input:	width and height of the world.
Output: None
Comments:	if the user only enters one varable, make the world a
			box.
Status:	Incomplete
*/
int main(int argc, char *argv[])
{
	// Local varables
	WINDOW *world;
	int offsetx, offsety;

	//	Prepare the terminal for curses mode, allocates memory for 
	//	stdcsr
	initscr();
	
	//	Disable terminal echo
	noecho();
	//	Disable line buffering and feed input directly to program
	cbreak();
	//	Set timeout in milliseconds
	timeout(TICKRATE);
	//	Enable special characters
	keypad(stdscr, TRUE);

	
	//	Display title of applications in the top right hand corner 
	//	with version number. 
	printw("spaceInvaders v. 1.0 - Press ESC to quit.");

	//	Refresh the original terminal
	refresh();

	//	Get offset of game box inside of window
	offsetx = (COLS - WORLD_WIDTH) / 2;
	offsety = (LINES - WORLD_HEIGHT) / 2;

	//	Create world window
	world = newwin(WORLD_HEIGHT,		// width
				   WORLD_WIDTH,		// height
				   offsety,			// vertical buffer
				   offsetx);		// horizontal buffer
	box(world, 0, 0);

	int input;
	//	draw the player
	player p1;
	p1.x = WORLD_WIDTH/2;
	p1.y = WORLD_HEIGHT - 2;
	p1.ship = '#';
	p1.firstBullet = 0;
	p1.lastBullet = 0;
		
	int ship_command = DO_NOTHING;
	while((input = getch()) != 'x'){
		updateWorld(world, ship_command, &p1);
		if(input != ERR){
			switch(input){
				case KEY_LEFT:
					ship_command = LEFT;
					break;
				case KEY_RIGHT:
					ship_command = RIGHT;
					break;
				case ' ':
					ship_command = FIRE;
				default:
					break;
			}
		} else {
			ship_command = DO_NOTHING;
		}

	}

	//	Get character input from user
	//getch();

	//	Deallocate memory and return the terminal to its former state
	delwin(world);
	endwin();

	return 0;
}

void fire(player *p){

	struct weapon *newbullet;
	newbullet = malloc(sizeof(struct weapon));
	newbullet->x = p->x;
	newbullet->y = p->y-1;
	newbullet->bulletChar = '*';
	newbullet->active = TRUE;

	// tell this bullet it is the last bullet
	newbullet->nextBullet = 0;

	if (p->firstBullet == 0){
		//	If there are no other bullets currently being tracked, set 
		//	this bullet to be the first and the last bullet.
		p->firstBullet = newbullet;
		p->lastBullet = newbullet;
	}
	else {
		//	If there is another bullet being tracked, update the last
		//	bullet to be this new bullet. 
		p->lastBullet->nextBullet = newbullet;
		p->lastBullet = newbullet;
	}
}

int move_player(WINDOW *win, int userInput, player *p)
{
	switch (userInput){
		case LEFT:
			p->x - 1 == 0 ? p->x = WORLD_WIDTH - 2 : --p->x;
			break;
		case RIGHT:
			p->x + 1 == WORLD_WIDTH - 1 ? p->x = 1 : ++p->x;
			break;
		case FIRE:
			fire(p);
			break;
		default:
			break;
	}
	//x = p.x;
	drawPlayer(win, p);
	//mvwaddch(win, 2, 2, p.x);
	return 0;
}

void drawPlayer(WINDOW *win, player *p)
{
	// move all of the bullets. 
	if (p->firstBullet != 0)
	{
		
		struct weapon *node;
		struct weapon *freeThis;
		node = p->firstBullet;
		//	Free all the memory of bullets untill we find a bullet 
		//	that is still active. 
		while ((node->y <= 0)){
			if (node->nextBullet == 0){
				free(node);
				p->firstBullet = 0;
				break;
			}
			
			p->firstBullet = node->nextBullet;
			freeThis = node;
			node = node->nextBullet;
			free(freeThis);
		}
		
		//	Update the remaining bullets;
		while(node){
			node->y -= 1; 
			mvwaddch(win, node->y, node->x, node->bulletChar);
			node = node->nextBullet;
		}
		
	}

	wmove(win, p->y, p->x);

	//mvwaddch(win, p->y, p->x, p->ship);
}

void drawInvaders(WINDOW *win, commonEnemy invaders[])
{
	for(int i = 0; i < 5; ++i){
		mvwaddch(win, invaders[i].y, invaders[i].x, invaders[i].ship);
	}
}

void updateWorld(WINDOW *win, int userInput, player* p){
	static int once = 1;
	static const int numberOfEnemies = 5;
	static commonEnemy invaders[numberOfEnemies];
	
	if (once){
		createEnemies(numberOfEnemies, invaders);
		once = 0;
	}

	// clear the child window
	wclear(win);

	// draw enemies
	drawInvaders(win, invaders);

	// update the players location
	move_player(win, userInput, p);

	//	Draw the game box
	box(win, 0, 0);

	//	Put the buffer on the screen
	wrefresh(win);
}

void createEnemies(const int numberOfEnemies, commonEnemy invaders[]){
	//commonEnemy invaders[numberOfEnemies];

	int xbuf = WORLD_WIDTH/(numberOfEnemies+1);

	for(int i = 0; i < numberOfEnemies; ++i){
		commonEnemy newInvader;
		newInvader.x = xbuf + xbuf*i;
		newInvader.y = 3;
		newInvader.ship = 'V';
		invaders[i] = newInvader;
	}

}



// EOF //