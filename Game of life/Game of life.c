#include <stdio.h>
#include <locale.h>
#include <time.h>
#include <glut.h>

#define live 1
#define dead 0
#define world_height 20
#define world_width 20

//для генерации мира
int world[world_width][world_height];
int prev_world[world_width][world_height];
int first_world[world_width][world_height];

//для цветов
float worldR[world_width][world_height];
float worldG[world_width][world_height];
float worldB[world_width][world_height];

float prev_worldR[world_width][world_height];
float prev_worldG[world_width][world_height];
float prev_worldB[world_width][world_height];

//переменные для состояния мира
int liveCondition, condition, periodCondition;
//размер поля для "игры жизнь"
float WinWid = 400.0;
float WinHight = 400.0;
//переменная для состояния кнопки старт
int start = 0;
//переменная для состояния кнопки стоп
int stop = 0;
//переменные для ручного заполнения 
float paintX, paintY;
// шрифт
void *font = GLUT_BITMAP_HELVETICA_18;

//Функции самого алгоритма "игры жизнь"
//------------------------------------------------------------------------------------------------------------

int world_cmp(int world1[world_width][world_height], int world2[world_width][world_height]) {
	for (int i = 0; i < world_width; i++) {
		for (int j = 0; j < world_height; j++) {
			if (world1[i][j] != world2[i][j]) return 0;
		}
	}
	return 1;
}

void world_copy(int destination[world_width][world_height], int source[world_width][world_height]) {
	for (int i = 0; i < world_width; i++) {
		for (int j = 0; j < world_height; j++) {
			destination[i][j] = source[i][j];
		}
	}
}

void world_copy_color(float destination[world_width][world_height], float source[world_width][world_height]) {
	for (int i = 0; i < world_width; i++) {
		for (int j = 0; j < world_height; j++) {
			destination[i][j] = source[i][j];
		}
	}
}

void read_neighbours(int neighbours[8][2], int x, int y) {
	int count = 0;
	int i, j;
	int k = 0;
	for (i = x - 1; i <= x + 1; i++) {
		for (j = y - 1; j <= y + 1; j++) {
			if (i == x && j == y) continue;

			neighbours[k][0] = i;
			neighbours[k][1] = j;

			k++;
		}
	}
}

int live_neighbours(int world[world_width][world_height], int x, int y) {
	int count = 0;
	int neighbours[8][2];
	int dx, dy;

	read_neighbours(neighbours, x, y);

	for (int i = 0; i < 8; i++) {
		dx = neighbours[i][0];
		dy = neighbours[i][1];

		//еще раз потом проверить 
		if (dx == world_width) neighbours[i][0] = world_width - dx;
		if (dx < 0) neighbours[i][0] = world_width + dx;
		if (dy == world_height) neighbours[i][1] = world_height - dy;
		if (dy < 0) neighbours[i][1] = world_height + dy;
	}

	for (int i = 0; i < 8; i++) {
		dx = neighbours[i][0];
		dy = neighbours[i][1];

		if (world[dx][dy] == live) count++;
	}
	return count;
}

float live_neighbours_color(float world_color[world_width][world_height], int x, int y) {
	int neighbours[8][2];
	float color = 0;
	int dx, dy;

	read_neighbours(neighbours, x, y);

	for (int i = 0; i < 8; i++) {
		dx = neighbours[i][0];
		dy = neighbours[i][1];

		//еще раз потом проверить 
		if (dx == world_width) neighbours[i][0] = world_width - dx;
		if (dx < 0) neighbours[i][0] = world_width + dx;
		if (dy == world_height) neighbours[i][1] = world_height - dy;
		if (dy < 0) neighbours[i][1] = world_height + dy;
	}

	for (int i = 0; i < 8; i++) {
		dx = neighbours[i][0];
		dy = neighbours[i][1];

		if (world_color[dx][dy] != 0.0) color += world_color[dx][dy];
	}
	color /= 3.0;
	color = 1.0 - color;
	
	return color;
}

void next_generation(int world[world_width][world_height], int prev_world[world_width][world_height]) {
	int i, j;
	int lv;

	for (i = 0; i < world_width; i++){
		for (int j = 0; j < world_height; j++) {
			lv = live_neighbours(prev_world, i, j);

			if (prev_world[i][j] == dead) {
				if (lv == 3) {
					world[i][j] = live;

					worldR[i][j] = live_neighbours_color(worldR, i, j);
					worldG[i][j] = live_neighbours_color(worldG, i, j);
					worldB[i][j] = live_neighbours_color(worldB, i, j);
				}
				else {
					world[i][j] = dead;

					worldR[i][j] = 0.0;
					worldG[i][j] = 0.0;
					worldB[i][j] = 0.0;
				}
			}
			 
			else {
				if (lv < 2 || lv > 3) {
					world[i][j] = dead;

					worldR[i][j] = 0.0;
					worldG[i][j] = 0.0;
					worldB[i][j] = 0.0;
				}
				//else world[i][j] = live;
			}

		}
	}
}

int live_cells(int world[world_width][world_height]) {
	int count = 0;
	for (int i = 0; i < world_width; i++) {
		for (int j = 0; j < world_height; j++) {
			if (world[i][j] == live) count++;
		}
	}
	return count;
}

void random_world(int world[world_width][world_height]) {
	srand(time(NULL));
	int digit;
	for (int i = 0; i < world_width; i++) {
		for (int j = 0; j < world_height; j++) {
			digit = rand();
			if (digit % 2 == 0) world[i][j] = live;
			else world[i][j] = dead;
		}
	}
}

void configur_world(int world[world_width][world_height]) {
	for (int i = 0; i < world_width; i++) {
		for (int j = 0; j < world_height; j++) {
			world[i][j] = dead;
		}
	}

	int x, y;
	int value;

	while (1) {
		printf("Введите координаты \n");
		scanf_s("%d", &x);
		scanf_s("%d", &y);
		world[x][y] = live;
		printf("Введите '1', чтобы продолжить или '2', чтобы закончить заполнение\n");
		scanf_s("%d", &value);
		if (value == 2) break;
	}
}

void null_world(int world[world_width][world_height]) {
	for (int i = 0; i < world_width; i++) {
		for (int j = 0; j < world_height; j++) {
			world[i][j] = dead;
		}
	}
}

void random_color(float worldR[world_width][world_height], float worldG[world_width][world_height], float worldB[world_width][world_height], int world[world_width][world_height]) {
	srand(time(NULL));

	int value, colors;

	for (int i = 0; i < world_width; i++) {
		for (int j = 0; j < world_height; j++) {
			if (world[i][j] == live) {
				colors = rand() % 6 + 1;

				switch (colors) {
				case 1:
					value = rand() % 256;
					worldR[i][j] = (float)value / 255;
					worldG[i][j] = 0.0;
					worldB[i][j] = 0.0;
					break;

				case 2:
					value = rand() % 256;
					worldG[i][j] = (float)value / 255;
					worldR[i][j] = 0.0;
					worldB[i][j] = 0.0;
					break;

				case 3:
					value = rand() % 256;
					worldB[i][j] = (float)value / 255;
					worldG[i][j] = 0.0;
					worldR[i][j] = 0.0;
					break;

				case 4:
					value = rand() % 256;
					worldR[i][j] = (float)value / 255;
					value = rand() % 256;
					worldG[i][j] = (float)value / 255;
					worldB[i][j] = 0.0;
					break;

				case 5:
					value = rand() % 256;
					worldG[i][j] = (float)value / 255;
					value = rand() % 256;
					worldB[i][j] = (float)value / 255;
					worldR[i][j] = 0.0;
					break;

				case 6:
					value = rand() % 256;
					worldR[i][j] = (float)value / 255;
					value = rand() % 256;
					worldB[i][j] = (float)value / 255;
					worldG[i][j] = 0.0;
					break;
				}
			}
			else {
				worldR[i][j] = 0.0;
				worldG[i][j] = 0.0;
				worldB[i][j] = 0.0;
			}
		}
	}
}

//------------------------------------------------------------------------------------------------------------

//Функции OpenGL
//------------------------------------------------------------------------------------------------------------

void Timer(int value) {
	glColor3f(1.0, 1.0, 1.0);
	glutPostRedisplay();
	glutTimerFunc(200, Timer, 0);
}

void Initialize() {
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, 800, 400, 0, -200.0, 200.0);
	glMatrixMode(GL_MODELVIEW);
}

void paint_cell(float x, float y) {
	int d = 20;
	int lDownX = 0, lUpX = 400, lDownY = 0, lUpY = 400;
	int value, colors;

	//поиск координат для закрашивания
	while (lDownX < x) lDownX += d;
	lDownX -= d;

	while (lDownY < y) lDownY += d;
	lDownY -= d;

	if (world[lDownY / 20][lDownX / 20] == live) {
		world[lDownY / 20][lDownX / 20] = dead;

		worldR[lDownY / 20][lDownX / 20] = 0.0;
		worldG[lDownY / 20][lDownX / 20] = 0.0;
		worldB[lDownY / 20][lDownX / 20] = 0.0;
	}
	else {
		world[lDownY / 20][lDownX / 20] = live;

		colors = rand() % 6 + 1;

		switch (colors) {
		case 1:
			value = rand() % 256;
			worldR[lDownY / 20][lDownX / 20] = (float)value / 255;
			worldG[lDownY / 20][lDownX / 20] = 0.0;
			worldB[lDownY / 20][lDownX / 20] = 0.0;
			break;

		case 2:
			value = rand() % 256;
			worldG[lDownY / 20][lDownX / 20] = (float)value / 255;
			worldR[lDownY / 20][lDownX / 20] = 0.0;
			worldB[lDownY / 20][lDownX / 20] = 0.0;
			break;

		case 3:
			value = rand() % 256;
			worldB[lDownY / 20][lDownX / 20] = (float)value / 255;
			worldG[lDownY / 20][lDownX / 20] = 0.0;
			worldR[lDownY / 20][lDownX / 20] = 0.0;
			break;

		case 4:
			value = rand() % 256;
			worldR[lDownY / 20][lDownX / 20] = (float)value / 255;
			value = rand() % 256;
			worldG[lDownY / 20][lDownX / 20] = (float)value / 255;
			worldB[lDownY / 20][lDownX / 20] = 0.0;
			break;

		case 5:
			value = rand() % 256;
			worldG[lDownY / 20][lDownX / 20] = (float)value / 255;
			value = rand() % 256;
			worldB[lDownY / 20][lDownX / 20] = (float)value / 255;
			worldR[lDownY / 20][lDownX / 20] = 0.0;
			break;

		case 6:
			value = rand() % 256;
			worldR[lDownY / 20][lDownX / 20] = (float)value / 255;
			value = rand() % 256;
			worldB[lDownY / 20][lDownX / 20] = (float)value / 255;
			worldG[lDownY / 20][lDownX / 20] = 0.0;
			break;
		}
	}
	world_copy(first_world, world);

	world_copy(prev_worldR, worldR);
	world_copy(prev_worldG, worldG);
	world_copy(prev_worldB, worldB);
}

void mouseButton(int button, int state, int x, int y) {

	if (button == GLUT_LEFT_BUTTON) {
		if (state == GLUT_DOWN) {

			printf("%d %d", x, y);
			printf("\n");

			//нажатие кнопки очистки поля
			if (x > 600 && x < 700 && y > 50 && y < 100) {
				null_world(world);
				start = 0;
			}

			//нажатие кнопки следующего поколения
			if (x > 600 && x < 700 && y > 150 && y < 200) {
				start = 0;

				world_copy(prev_world, world);

				world_copy_color(prev_worldR, worldR);
				world_copy_color(prev_worldG, worldG);
				world_copy_color(prev_worldB, worldB);

				next_generation(world, prev_world);
				condition = world_cmp(world, prev_world);
				liveCondition = live_cells(world);
				periodCondition = world_cmp(world, first_world);
			}

			//нажатие кнопки старт
			if (x > 450 && x < 550 && y > 50 && y < 100) start = 1;

			//нажатие кнопки стоп
			if (x > 450 && x < 550 && y > 250 && y < 300) {
				stop = 1;
				start = 0;
			}

			//нажатие кнопки рандомной генерации
			if (x > 450 && x < 550 && y > 150 && y < 200) {
				start = 0;
				random_world(world);

				random_color(worldR, worldG, worldB, world);

				world_copy_color(prev_worldR, worldR);
				world_copy_color(prev_worldB, worldB);
				world_copy_color(prev_worldG, worldG);
				
				world_copy(first_world, world);
				printf("Hello world!\n");
			}

			//ручное заполнение
			if (x > 0 && x < 400 && y > 0 && y < 400) paint_cell(x, y);

		}
	}
}

void create_button(float xSour, float xDest, float ySour, float yDest) {
	glBegin(GL_POINTS);
	glColor3f(1.0, 1.0, 1.0);
	for (float i = xSour; i <= xDest; i++) {
		for (float j = ySour; j <= yDest; j++) {
			glVertex2f(i, j);
		}
	}
	glEnd();
}

void create_button_text(char *text, float x, float y) {
	char *c;

	glColor3f(0.0, 0.0, 0.0);
	glRasterPos2f(x, y);
	for (c = text; *c != '\0'; c++) {
		glutBitmapCharacter(font, *c);
	}
}

void paint_world() {
	float x = 0.0, y = 0.0;

	glBegin(GL_POINTS);
	for (int i = 0; i < world_width; i++) {
		for (int j = 0; j < world_height; j++) {

			if (world[i][j] == live) {
				x = i * 20; y = j * 20;
				float t = x + 20, t1 = y + 20;

				for (y = (j * 20) + 1; y < t1 - 1; y++) {
					for(x = (i * 20 + 1); x < t - 1; x++) {
						//glColor3f(1.0, 0.0, 0.0);
						glColor3f(worldR[i][j], worldG[i][j], worldB[i][j]);
						glVertex2f(y, x);
					}
				}

			}

		}
	}
	glEnd();
}

void create_grid() {
	float x = 0.0, y = 0.0;

	glBegin(GL_LINES);
	glColor3d(1, 1, 1);
	for (float i = 0 + x; i <= WinWid + x; i += 20.0)
	{
		glVertex2f(i, 0 + y);
		glVertex2f(i, WinHight + y);
	}
	for (float i = 0 + y; i <= WinHight + y; i += 20.0)
	{
		glVertex2f(0 + x, i);
		glVertex2f(WinWid + x, i);
	}
	glEnd();
}

void Draw() {
	glClear(GL_COLOR_BUFFER_BIT);

	glPushMatrix();

	create_button(450, 550, 50, 100);      //кнопка старт
	create_button(450, 550, 150, 200);     //конпка рандомной генерации 
	create_button(450, 550, 250, 300);     //кнопка стоп
	create_button(600, 700, 50, 100);      //кнопка очистки поля
	create_button(600, 700, 150, 200);     //кнопка для остановки и просмотра следующего поколения

	//текст кнопок
	create_button_text("Start", 480, 80);
	create_button_text("Random", 470, 180);
	create_button_text("Stop", 480, 280);
	create_button_text("Clear", 630, 80);
	create_button_text("Next", 630, 180);
	
	create_grid();  //рисовка сетки
	paint_world();  //рисовка мира

	glPopMatrix();

	glutSwapBuffers();

	if (start == 1) {
		world_copy(prev_world, world);

		world_copy_color(prev_worldR, worldR);
		world_copy_color(prev_worldG, worldG);
		world_copy_color(prev_worldB, worldB);

		next_generation(world, prev_world);
		condition = world_cmp(world, prev_world);
		liveCondition = live_cells(world);
		periodCondition = world_cmp(world, first_world);
	}
}

//------------------------------------------------------------------------------------------------------------

int main(int argc, char **argv) {

	setlocale(LC_ALL, "Rus");

	null_world(world);

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(800, 400);
	glutInitWindowPosition(100, 200);
	glutCreateWindow("The Game of Life");

	//int operation;
	//printf("Введите '1', если хотите сами задать мир, '2', чтобы мир сгенерировался\n");
	//scanf_s("%d", &operation);
	//if (operation == 1) configur_world(world);
	//if (operation == 2) random_world(world);
	//world_copy(first_world, world);

	glutDisplayFunc(Draw);
	glutMouseFunc(mouseButton);
	glutTimerFunc(200, Timer, 0);

	if (liveCondition == 0 || condition == 1 || periodCondition == 1) start = 0;

	Initialize();

	glutMainLoop();

	return 0;
}