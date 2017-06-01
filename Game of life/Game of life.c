#include <stdio.h>
#include <locale.h>
#include <string.h>
#include <time.h>
#include <glut.h>

#define live 1
#define dead 0
#define world_height 20
#define world_width 20

//структура для сохранения миров и последующей проверки периода
struct worlds {
	int game_world[world_width][world_height];
	struct worlds *next;
};

//корень линейного списка
struct wolrds *root;
//для добавления элементов
struct worlds *ptr;

//для генерации мира
int world[world_width][world_height];
int prev_world[world_width][world_height];
int first_world[world_width][world_height];

//для цветов
int worldR[world_width][world_height];
int worldG[world_width][world_height];
int worldB[world_width][world_height];

int prev_worldR[world_width][world_height];
int prev_worldG[world_width][world_height];
int prev_worldB[world_width][world_height];

//переменные для состояния мира
int liveCondition = 1, condition, periodCondition, new_period_condition;
//для определения периода фигуры
int period = 0;
//для определения периода получившийся фигуры
int received_period = 0;
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

void world_copy_color(int destination[world_width][world_height], int source[world_width][world_height]) {
	for (int i = 0; i < world_width; i++) {
		for (int j = 0; j < world_height; j++) {
			destination[i][j] = source[i][j];
		}
	}
}

struct worlds *create_root(int world[world_width][world_height]) {
	struct worlds *ptr;
	ptr = (struct worlds*)malloc(sizeof(struct worlds));
	world_copy(ptr->game_world, world);
	ptr->next = NULL;
	return ptr;
}

struct worlds *create_elem(struct worlds *ptr, int world[world_width][world_height]) {
	struct worlds *temp, *p;
	temp = (struct worlds*)malloc(sizeof(struct worlds));
	p = ptr->next;
	world_copy(temp->game_world, world);
	ptr->next = temp;
	temp->next = p;
	return temp;
}

void delete_worlds(struct worlds *root) {
	struct worlds *temp;
	temp = root;
	while (temp != NULL) {
		temp = root->next;
		free(root);
		root = temp;
	}
}

int check_period(struct worlds *root, int world[world_width][world_height]) {
	struct worlds *temp;
	temp = root;
	int period = 0;

	while (temp != NULL) {
		if (world_cmp(world, temp->game_world) == 1) {
			
			do {
				temp = temp->next;
				period++;
				if (temp == NULL) return -1;
			} while (world_cmp(world, temp->game_world) != 1);

			return period;
		}
		temp = temp->next;
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

float live_neighbours_color(int world_color[world_width][world_height], int x, int y) {
	int neighbours[8][2];
	int color = 0;
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

		if (world_color[dx][dy] != 0) color += world_color[dx][dy];
	}
	color /= 3;
	
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

					worldR[i][j] = live_neighbours_color(prev_worldR, i, j);
					worldG[i][j] = live_neighbours_color(prev_worldG, i, j);
					worldB[i][j] = live_neighbours_color(prev_worldB, i, j);
				}
				else {
					world[i][j] = dead;

					worldR[i][j] = 0;
					worldG[i][j] = 0;
					worldB[i][j] = 0;
				}
			}
			 
			else {
				if (lv < 2 || lv > 3) {
					world[i][j] = dead;

					worldR[i][j] = 0;
					worldG[i][j] = 0;
					worldB[i][j] = 0;
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

void random_color(int worldR[world_width][world_height], int worldG[world_width][world_height], int worldB[world_width][world_height], int world[world_width][world_height]) {
	srand(time(NULL));

	int value, colors;

	for (int i = 0; i < world_width; i++) {
		for (int j = 0; j < world_height; j++) {
			if (world[i][j] == live) {
				colors = rand() % 7 + 1;

				switch (colors) {
				case 1:
					value = rand() % 256;
					worldR[i][j] = value;
					worldG[i][j] = 0;
					worldB[i][j] = 0;
					break;

				case 2:
					value = rand() % 256;
					worldG[i][j] = value;
					worldR[i][j] = 0;
					worldB[i][j] = 0;
					break;

				case 3:
					value = rand() % 256;
					worldB[i][j] = value;
					worldG[i][j] = 0;
					worldR[i][j] = 0;
					break;

				case 4:
					value = rand() % 256;
					worldR[i][j] = value;
					value = rand() % 256;
					worldG[i][j] = value;
					worldB[i][j] = 0;
					break;

				case 5:
					value = rand() % 256;
					worldG[i][j] = value;
					value = rand() % 256;
					worldB[i][j] = value;
					worldR[i][j] = 0;
					break;

				case 6:
					value = rand() % 256;
					worldR[i][j] = value;
					value = rand() % 256;
					worldB[i][j] = value ;
					worldG[i][j] = 0;
					break;

				case 7:
					value = rand() % 256;
					worldR[i][j] = value;
					value = rand() % 256;
					worldB[i][j] = value; 
					value = rand() % 256;
					worldG[i][j] = value;
					break;
				}
			}
			else {
				worldR[i][j] = 0;
				worldG[i][j] = 0;
				worldB[i][j] = 0;
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

		worldR[lDownY / 20][lDownX / 20] = 0;
		worldG[lDownY / 20][lDownX / 20] = 0;
		worldB[lDownY / 20][lDownX / 20] = 0;
	}
	else {
		world[lDownY / 20][lDownX / 20] = live;

		colors = rand() % 7 + 1;

		switch (colors) {
		case 1:
			value = rand() % 256;
			worldR[lDownY / 20][lDownX / 20] = value;
			worldG[lDownY / 20][lDownX / 20] = 0;
			worldB[lDownY / 20][lDownX / 20] = 0;
			break;

		case 2:
			value = rand() % 256;
			worldG[lDownY / 20][lDownX / 20] = value ;
			worldR[lDownY / 20][lDownX / 20] = 0;
			worldB[lDownY / 20][lDownX / 20] = 0;
			break;

		case 3:
			value = rand() % 256;
			worldB[lDownY / 20][lDownX / 20] = value;
			worldG[lDownY / 20][lDownX / 20] = 0;
			worldR[lDownY / 20][lDownX / 20] = 0;
			break;

		case 4:
			value = rand() % 256;
			worldR[lDownY / 20][lDownX / 20] = value;
			value = rand() % 256;
			worldG[lDownY / 20][lDownX / 20] = value;
			worldB[lDownY / 20][lDownX / 20] = 0;
			break;

		case 5:
			value = rand() % 256;
			worldG[lDownY / 20][lDownX / 20] = value;
			value = rand() % 256;
			worldB[lDownY / 20][lDownX / 20] = value;
			worldR[lDownY / 20][lDownX / 20] = 0;
			break;

		case 6:
			value = rand() % 256;
			worldR[lDownY / 20][lDownX / 20] = value;
			value = rand() % 256;
			worldB[lDownY / 20][lDownX / 20] = value;
			worldG[lDownY / 20][lDownX / 20] = 0;
			break;

		case 7:
			value = rand() % 256;
			worldR[lDownY / 20][lDownX / 20] = value;
			value = rand() % 256;
			worldB[lDownY / 20][lDownX / 20] = value;
			value = rand() % 256;
			worldG[lDownY / 20][lDownX / 20] = value;
			break;
		}
	}
	world_copy(first_world, world);

	world_copy_color(prev_worldR, worldR);
	world_copy_color(prev_worldG, worldG);
	world_copy_color(prev_worldB, worldB);
}

void mouseButton(int button, int state, int x, int y) {

	if (button == GLUT_LEFT_BUTTON) {
		if (state == GLUT_DOWN) {

			printf("%d %d", x, y);
			printf("\n");

			//нажатие кнопки очистки поля
			if (x > 600 && x < 700 && y > 50 && y < 100) {
				null_world(world);

				delete_worlds(root);
				root = NULL;

				start = 0;
				period = 0;
				received_period = 0;
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
				period++;

				if (root == NULL)
				{
					root = create_root(world);
					ptr = root;
				}
				else ptr = create_elem(ptr, world);

				//ptr = create_elem(ptr, world);
			}

			//нажатие кнопки старт
			if (x > 450 && x < 550 && y > 50 && y < 100) {
				start = 1;

				root = create_root(world);
				ptr = root;

				if (live_cells(world) == 0) {
					liveCondition = 1;
					condition = 0;
					periodCondition = 0;
					start = 0;
					new_period_condition = 0;
				}
			}

			//нажатие кнопки стоп
			if (x > 450 && x < 550 && y > 250 && y < 300) {
				stop = 1;
				start = 0;
			}

			//нажатие кнопки рандомной генерации
			if (x > 450 && x < 550 && y > 150 && y < 200) {
				null_world(world);

				delete_worlds(root);
				root = NULL;

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

void create_text(char *text, float x, float y) {
	char *c;

	glColor3f(1.0, 1.0, 1.0);
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
						glColor3ub(worldR[i][j], worldG[i][j], worldB[i][j]);
						//glColor3f(worldR[i][j], worldG[i][j], worldB[i][j]);
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
	for (float i = 0 + x; i <= WinWid + x; i += 20.0){
		glVertex2f(i, 0 + y);
		glVertex2f(i, WinHight + y);
	}
	for (float i = 0 + y; i <= WinHight + y; i += 20.0){
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

	/*create_button(600, 700, 250, 300);

	if (condition == 1) {
		create_button_text("Stable figure", 600, 280);
		periodCondition = 0;
	}
	if (periodCondition == 1) {
		char per[100] = "Period is ";
		char digit[6];

		itoa(period, digit, 10);
		strcat(per, digit);
		create_button_text(per, 600, 280);
	}
	if (liveCondition == 0) create_button_text ("World dead", 600, 280);*/

	if (condition == 1) {
		create_text("Stable figure", 450, 350);
		periodCondition = 0;
	}
	if (liveCondition == 0) create_text("All cells dead", 450, 350);

      received_period = check_period(root, world);
	if (received_period > 0) {
		char per[100] = "Period is ";
		char digit[6];

		new_period_condition = 1;
		itoa(received_period, digit, 10);
		strcat(per, digit);
		create_text(per, 450, 350);
	}
	else new_period_condition = 0;
	
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
		period++;

		ptr = create_elem(ptr, world);

		if (liveCondition == 0 || condition == 1 || periodCondition == 1 || new_period_condition == 1) start = 0;
		

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

	Initialize();

	glutMainLoop();

	return 0;
}