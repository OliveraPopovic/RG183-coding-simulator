#include <GL/freeglut.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <stdlib.h>
#include <ctype.h>
#include <limits.h>
#include "figure.h"
#include "carpet.h"

#define TIMER_INTERVAL 10
#define TIMER_ID 0
#define TIMER_ID_CAMERA_OUT 1
#define TIMER_ID_CAMERA_IN 2

#define MAX_NUM_MOVES 200

// TODO add animation and failure when the figure falls off the tiles

static void on_display();
static void on_reshape(int width, int height);
void draw_boy(void);
void draw_floor();
void on_keyboard(unsigned char key, int x, int y);
static void on_timer(int id);
void change_key_pressed(char);
void draw_special();
void set_arena_for_level(int);
int check_all_specials_activated();
int last_special_tile_activated = 0;

/* used to keep track of the light source so a small cube acting as a 
   lightbulb can be drawn to more easily see where the light is coming from */
static int light_x = 3;
static int light_y = 4;
static int light_z = -5;

float random_array[RANDOM_ARRAY_LENGTH];
float animation_parameter = 0;
char current_pressed_key = '\0';
char previous_pressed_key = 'w'; // because at the beginning the figure is facing forward
int animation_ongoing = 0;
float z = 0;
float x = 0;
float y = 0;
int previous_tile_z = 0;
int previous_tile_x = 0;

unsigned char array_of_moves[MAX_NUM_MOVES];
int current_move_index = 0;

int pressed_enter = 0;

int is_current_special_activated = 0;
int current_level = 1;

float camera_parameter_in_out = 0;
float camera_parameter_in_out_max = 0; // TODO change for each level depending on far you need to zoom out
float camera_parameter_z = 0;
float camera_parameter_x = 0;
int camera_stops_at_x = 0;
int camera_stops_at_y = 0;
int camera_stops_at_z = 0;
int camera_look_at_x = 0;
int camera_look_at_y = 0;
int camera_look_at_z = 0;
int level_failed = 0;
int is_final_level = 0;

FILE* level_file;

typedef struct simple_tile_struct {
	int x;
	int z;
} simple_tile;

typedef struct special_tile_struct {
	int x;
	int z;
	int activated;
} special_tile;
special_tile array_special_tiles[15];
simple_tile array_simple_tiles[150];

int main(int argc, char **argv){
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);

    // glutInitWindowSize(600, 600);
    glutInitWindowSize(700, 600);
    glutInitWindowPosition(100, 100);
    glutCreateWindow(argv[0]);

    glutDisplayFunc(on_display);
    glutReshapeFunc(on_reshape);
    glutKeyboardFunc(on_keyboard);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_NORMALIZE);
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,1);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    float light_position[] = {light_x,light_y,light_z,1};
    float light_ambient[] = {.3f, .3f, .3f, 1};
    float light_diffuse[] = {.7f, .7f, .7f, 1};
    float light_specular[] = {.7f, .7f, .7f, 1};

    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	
	level_file = fopen("levels.txt", "r");
	srand(time(NULL));
	array_of_moves[0] = '\0';
	set_arena_for_level(current_level);
	
	/* initializing carpet */
	int i;
	for (i = 0; i < RANDOM_ARRAY_LENGTH; i++)
	{
		random_array[i] = (float) rand() / RAND_MAX;
	}
	
    glClearColor(0,0, 0, 0);
    glutMainLoop();

	fclose(level_file);
    return 0;
}

void set_arena_for_level(int level)
{

	animation_parameter = 0;
	current_pressed_key = '\0';
	previous_pressed_key = 'w'; // because at the beginning the figure is facing forward, TODO mozda necu ovo za sve nivoe realno
	animation_ongoing = 0;
	z = 0;
	x = 0;
	y = 0;
	previous_tile_z = 0;
	previous_tile_x = 0;

	current_move_index = 0;

	pressed_enter = 0;

	is_current_special_activated = 0;

	camera_parameter_in_out = 0;
	camera_parameter_z = 0;
	camera_parameter_x = 0;

	array_of_moves[0] = '\0';

	glutTimerFunc(250, on_timer, TIMER_ID_CAMERA_OUT);
	
	int num_simple_tiles;
	int num_special_tiles;
	int end;
	int current_x;
	int current_z;
	
	fscanf(level_file, "%d", &end);
	if (end == -1)
	{
		return;
	}
	fscanf(level_file, "%d%d", &num_simple_tiles, &num_special_tiles);
	
	int i;
	for (i = 0; i < num_simple_tiles; i++)
	{
		fscanf(level_file, "%d%d", &current_x, &current_z);
		array_simple_tiles[i].x = current_x; 
		array_simple_tiles[i].z = current_z;
	}
	
	array_simple_tiles[i].x = INT_MAX;
	
	for (i = 0; i < num_special_tiles; i++)
	{
		fscanf(level_file, "%d%d", &current_x, &current_z);
		array_special_tiles[i].x = current_x; 
		array_special_tiles[i].z = current_z;
		array_special_tiles[i].activated = 0;
	}
	
	array_special_tiles[i].x = INT_MAX;

	switch(level)
	{
		case 1:
			camera_parameter_in_out_max = 0.7;
			camera_stops_at_x = 21;
			camera_stops_at_y = 28;
			camera_stops_at_z = 28;
			camera_look_at_x = 0;
			camera_look_at_y = 0;
			camera_look_at_z = 5;
			break;
		case 2:
			camera_parameter_in_out_max = 1;
			camera_stops_at_x = 21;
			camera_stops_at_y = 20;
			camera_stops_at_z = 35;
			camera_look_at_x = 3;
			camera_look_at_y = 0;
			camera_look_at_z = 3;
			break;
		case 3:
			is_final_level = 1;
			camera_parameter_in_out_max = 3;
			camera_stops_at_x = 30;
			camera_stops_at_y = 40;
			camera_stops_at_z = 60;
			camera_look_at_x = 3;
			camera_look_at_y = 0;
			camera_look_at_z = 3;
			break;
	}
	
	glutPostRedisplay();
}

void draw_special() // only if e was pressed on a special tile
{
	if (!is_current_special_activated)
		return;
		
	glPushAttrib(GL_LIGHTING_BIT);
	
	x = previous_tile_x*(-3.75) - 0.3;
	z = previous_tile_z*(-3.75);
	float special_y = - animation_parameter;
	
	glPushAttrib(GL_LIGHTING_BIT);
	GLfloat mat_ambient_special[] ={ 0.5, 1, 1, 1 };
	GLfloat mat_diffuse_special[] ={ 0.5, 1, 1, 1 };
	GLfloat mat_specular_special[] ={ 0.5, 1, 1, 0.922f };
	GLfloat shine_special[] = { 11.264f };

	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, mat_ambient_special);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_diffuse_special);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular_special);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shine_special);
    
    glPushMatrix();
    	glTranslatef(x, special_y, z);
    	glScalef(3.5,0.3,3.5);
    	glutSolidCube(1);
    glPopMatrix();
    
    glPopAttrib();
}

void on_reshape(int width, int height) {
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    gluPerspective(30, (float) width/height, 1, 250);
}

void on_display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    /*
	gluLookAt(18 - camera_parameter_in_out*3 + camera_parameter_x, 16 + camera_parameter_in_out*24, 22 + camera_parameter_in_out*8 - camera_parameter_z,
			  0 + camera_parameter_x, 0, 0 - camera_parameter_in_out*10 - camera_parameter_z,
			  0, 1, 0);
			  */
	
	/*
	gluLookAt(camera_stops_at_x - camera_parameter_in_out*3 + camera_parameter_x, camera_stops_at_y + camera_parameter_in_out*24, camera_stops_at_z + camera_parameter_in_out*8 - camera_parameter_z,
			  0 + camera_parameter_x, 0, 0 - camera_parameter_in_out*10 - camera_parameter_z,
			  0, 1, 0);
	*/
	// TODO change this from magic numbers to something normal
	
		/*
	gluLookAt(18 - camera_parameter_in_out*(camera_stops_at_x - 18) + camera_parameter_x, 16 + camera_parameter_in_out*(camera_stops_at_y - 16), 22 + camera_parameter_in_out*(camera_stops_at_x - 22) - camera_parameter_z,
	
			  0 + camera_parameter_x + camera_parameter_in_out*camera_look_at_x, 0 + camera_parameter_in_out*camera_look_at_y, 0 - camera_parameter_in_out*camera_look_at_z - camera_parameter_z,
			  
			  0, 1, 0);
      */
      
    float camera_location_x = 18 - camera_parameter_in_out*(camera_stops_at_x - 18) + camera_parameter_x;
	float camera_location_y = 16 + camera_parameter_in_out*(camera_stops_at_y - 16);
	float camera_location_z = 22 + camera_parameter_in_out*(camera_stops_at_x - 22) - camera_parameter_z;  
	
	gluLookAt(camera_location_x, camera_location_y, camera_location_z,
	
			  0 + camera_parameter_x + camera_parameter_in_out*camera_look_at_x, 0 + camera_parameter_in_out*camera_look_at_y, 0 - camera_parameter_in_out*camera_look_at_z - camera_parameter_z,
			  
			  0, 1, 0);
              
    if (!pressed_enter)
    {
		glPushAttrib(GL_LIGHTING_BIT);
			glPushMatrix();
				// glRasterPos3i(-15,-20,1);
				glRasterPos3i(-5, -8, 1);
				GLfloat mat_ambient[] ={ 1, 1, 1, 1 };
				glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
				glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, array_of_moves);
			glPopMatrix();
		glPopAttrib();
	}
	
	
    glPushMatrix();
    	glTranslatef(light_x, light_y, light_z);
    	glutSolidCube(0.2);
    glPopMatrix();
    	draw_special();
        draw_girl();
        glPushMatrix();
        //glTranslatef(-3.75, 0, 0);
        //draw_boy();
        glPopMatrix(); 
        draw_floor();
        if (!is_final_level)
        {
        	draw_triangle_carpet();
		}
    glPopMatrix();

    glutSwapBuffers();
}

void draw_boy()
{
	/*
	glPushAttrib(GL_LIGHTING_BIT);
	
	x = previous_tile_x*(-3.75);
	z = previous_tile_z*(-3.75);
	y = sin(animation_parameter * PI);
	
	if (pressed_a)
		x += animation_parameter*(-3.75);
	if (pressed_w)
		z += animation_parameter*(-3.75);
	if (pressed_d)
		x += animation_parameter*(3.75);
	if(pressed_s)
		z += animation_parameter*(3.75);

	GLfloat mat_ambient[] ={ 0.05, 0.0, 1, 1.0f };
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, mat_ambient);

	glPushMatrix();
    	glTranslatef(x,y + 1.3,z);
    	glScalef(1.3,1.2,1);
    	glutSolidCube(1);
    glPopMatrix();
    
    glPushMatrix();
    	glTranslatef(x - 0.34,y + 0.5,z);
    	glScalef(0.6,0.6,1);
    	glutSolidCube(1);
    glPopMatrix();
    
    glPushMatrix();
    	glTranslatef(x + 0.34,y + 0.5,z);
    	glScalef(0.6,0.6,1);
    	glutSolidCube(1);
    glPopMatrix();
    
    glPushMatrix();
    	glTranslatef(x,y +2.4,z);
    	glScalef(0.65,0.65,0.65);
    	glutSolidSphere(1, 16, 16);
    glPopMatrix();
    
    glPushMatrix();
    	glTranslatef(x,y +3.2, z);
    	glScalef(0.2,0.2,0.2);
    	glutSolidSphere(1, 16, 16);
    glPopMatrix();
    
    glPopAttrib();
    
    */
}

void draw_floor()
{
	glPushMatrix();
	
	float translate_by_x = 3.75;
	float translate_by_z = 3.75;

	glTranslatef(-0.3,0,0);
	
	int current_simple_index = 0;
	simple_tile current_simple;
	
	int current_special_index = 0;
	special_tile current_special;
	
	glPushAttrib(GL_LIGHTING_BIT);
	
	GLfloat mat_ambient[] ={ 1, 1, 1, 1 };
	GLfloat mat_diffuse[] ={ 1, 1, 1, 1 };
	GLfloat mat_specular[] ={ 1, 1, 1, 0.922f };
	GLfloat shine[] = { 11.264f };
	
	GLfloat mat_ambient_special[] ={ 0.5, 1, 1, 1 };
	GLfloat mat_diffuse_special[] ={ 0.5, 1, 1, 1 };
	GLfloat mat_specular_special[] ={ 0.5, 1, 1, 0.922f };
	GLfloat shine_special[] = { 11.264f };
	
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, mat_ambient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_diffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shine);
	
	while (array_simple_tiles[current_simple_index].x != INT_MAX)
	{
		current_simple = array_simple_tiles[current_simple_index];
	
		glPushMatrix();
			glTranslatef(translate_by_x * current_simple.x, 0 , translate_by_z * current_simple.z);
			glScalef(3.5,0.3,3.5);
			glutSolidCube(1);
    	glPopMatrix();
    	
    	current_simple_index++;
	}
	
	while (array_special_tiles[current_special_index].x != INT_MAX)
	{
		current_special = array_special_tiles[current_special_index];
	
		if (current_special.activated)
		{
			glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, mat_ambient);
			glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_diffuse);
			glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular);
			glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shine);
		}
		else 
		{
			glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, mat_ambient_special);
			glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_diffuse_special);
			glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular_special);
			glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shine_special);
		}
	
		glPushMatrix();
			glTranslatef(translate_by_x * current_special.x, 0 , translate_by_z * current_special.z);
			glScalef(3.5,0.3,3.5);
			glutSolidCube(1);
    	glPopMatrix();
    	
    	current_special_index++;
	}
	
    glPopMatrix();
    
    glPopAttrib();
}

void add_to_move_array(char move) 
{
	array_of_moves[current_move_index] = move;
	current_move_index++;
	array_of_moves[current_move_index] = '\0';
}

void on_keyboard(unsigned char key, int x, int y) {
    switch(key) {
        case 'w':
        case 'W': 
        case 's': 
        case 'S':
        case 'a':
        case 'A':
        case 'd':
        case 'D':
        case 'E':
        case 'e':
        	if (pressed_enter)
        		break;
        	add_to_move_array(tolower(key));
        	glutPostRedisplay();
        	break;  
        case 'C':
        case 'c':
        	// resetting input
        	if (pressed_enter)
        		break;
        	current_move_index = 0;
        	array_of_moves[0] = '\0';
        	glutPostRedisplay();
        	break;
        case 13: // code for ENTER
        	if (pressed_enter) 
        		break;
        	pressed_enter = 1;
        	current_move_index = 0;
        	current_pressed_key = array_of_moves[current_move_index];
			// change_key_pressed(array_of_moves[current_move_index]);
        	// animation_ongoing = 1;
        	// glutTimerFunc(TIMER_INTERVAL, on_timer, TIMER_ID);
        	glutTimerFunc(TIMER_INTERVAL, on_timer, TIMER_ID_CAMERA_IN);
        	break;
        case 27:
          exit(0);
          break;
    }
}
// TODO spreci da se desava on oza kamerom zoom in out

void on_timer(int id) {
    
    if (id == TIMER_ID_CAMERA_OUT)
    {
    	if (camera_parameter_in_out >= camera_parameter_in_out_max)
    		return;
    		
    	camera_parameter_in_out += 0.005;
    	glutTimerFunc(TIMER_INTERVAL, on_timer, TIMER_ID_CAMERA_OUT);
    	
    }
    else if (id == TIMER_ID_CAMERA_IN)
    {
    	if (camera_parameter_in_out > 0)
    	{
			camera_parameter_in_out -= 0.008;
			glutTimerFunc(TIMER_INTERVAL, on_timer, TIMER_ID_CAMERA_IN);
    	}
    	else 
    	{
    		animation_ongoing = 1;
    		animation_parameter = 0;
    		glutTimerFunc(TIMER_INTERVAL, on_timer, TIMER_ID);
    	}
    	
    }
    else if (animation_ongoing)
	{
		if (animation_parameter >= 1)
		{
			if (!level_failed)
			{
				animation_parameter = 0;
				animation_ongoing = 0;
				if (current_pressed_key != 'e')
					previous_pressed_key = current_pressed_key;
				switch(current_pressed_key)
				{
					case 'w':
						previous_tile_z++;
						break;
					case 'a':
						previous_tile_x++;
						break;
					case 's':
						previous_tile_z--;
						break;
					case 'd':
						previous_tile_x--;
						break;
				}
			}
			
			// TODO change
			is_current_special_activated = 0;
		}
		else {
			if (!level_failed)
			{
				switch(current_pressed_key)
				{
					case 'w':
						camera_parameter_z += 0.1;
						break;
					case 'a':
						camera_parameter_x -= 0.1;
						break;
					case 's':
						camera_parameter_z -= 0.1;
						break;
					case 'd':
						camera_parameter_x += 0.1;
						break;
				}
			}
			
			animation_parameter += 0.03;	
		}
		
		glutTimerFunc(TIMER_INTERVAL,on_timer,TIMER_ID);
	}
	else
	{
		if(check_all_specials_activated())
		{
			printf("Level Complete!\n");
			// previous_pressed_key = current_pressed_key;
			// TODO move all initializations to level set up method so that everything's reset properly
			set_arena_for_level(++current_level);
		}
		else 
		{
			current_move_index++;
			char current = array_of_moves[current_move_index];
			if (current == '\0') 
			{
				// previous_pressed_key = current_pressed_key;
				// sad animation has to play
				level_failed = 1;
				animation_ongoing = 1;
				animation_parameter = 0;
				current_pressed_key = '\0';
				glutTimerFunc(TIMER_INTERVAL,on_timer,TIMER_ID);
				
			}
			else // w a s d e
			{
				if (current == 'e')
				{
					int current_special_index = 0;
					while (array_special_tiles[current_special_index].x != INT_MAX)
					{
						special_tile current_special = array_special_tiles[current_special_index];
						if (current_special.x == -previous_tile_x && current_special.z == -previous_tile_z)
						{
							array_special_tiles[current_special_index].activated = 1;
							is_current_special_activated = 1;
							check_all_specials_activated();
						}
						current_special_index++;
					} 
				}
				
				// I don't want to change previous to e, it's supposed to keep track of the last movement thingie
				if (current_pressed_key != 'e')
					previous_pressed_key = current_pressed_key;
				
				current_pressed_key = current;
				animation_ongoing = 1;
				animation_parameter = 0;
				glutTimerFunc(TIMER_INTERVAL,on_timer,TIMER_ID);
			}
		}
	}
	
	glutPostRedisplay();
}

int check_all_specials_activated()
{
	int current_special_index = 0;
	
	while (array_special_tiles[current_special_index].x != INT_MAX)
	{
		if (array_special_tiles[current_special_index].activated == 0)
		{
			return 0;
		}
		current_special_index++;
	} 
	
	last_special_tile_activated = 1;
	return 1;
}

