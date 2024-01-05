#include "include/Angel.h"
#pragma comment(lib, "glew32.lib")
#pragma warning(disable:4996)

#include <cstdlib>
#include <iostream>
#include <cstdio>
#include <fstream>
#include <time.h>
#include <string>
#include <algorithm>

using namespace std;

int starttime;			// ���Ʒ��������ƶ�ʱ��
bool gameover = false;	// ��Ϸ�������Ʊ���
bool stop = false;		// ��Ϸ��ͣ����

int xsize = 580;		// ���ڴ�С
int ysize = 725;

int clean_up_row = 0;		// ��¼ÿһ�α�����������
int score = 0;				// ��¼��Ϸ�÷�
char score_tip[100] = "current score:";	// �����������
char next_tip[100] = "next:";	// �����������
char gameover_tip[100] = "";	// �����������
char keyboard_tip[100] = "stop/continue: P   restart: R   quit: Q";	// ���������������ʾ
char score_out[100] = "0";		// ���������Ϸ�÷�
int score_rank[4] = { 0 };	// ��¼��Ϸ�÷����а�
bool al_update_rank = false;// ��¼�Ƿ�update�����а񣬷�ֹ�ظ�update

GLuint program;		// �˴�����ɫ����Ϊȫ�ֱ������Ա�ʹ��

// ���ƴ��ڵ���ɫ����
vec4 orange = vec4(1.0, 0.5, 0.0, 1.0); // ��ɫ 1 (ע�͵ı�Ŵ��������±�)
vec4 white = vec4(1.0, 1.0, 1.0, 1.0);	// ��ɫ
vec4 black = vec4(0.0, 0.0, 0.0, 1.0);	// ��ɫ
vec4 green = vec4(0.39, 0.75, 0.22, 1.0);	// ��ɫ 2.
vec4 yellow = vec4(1.0, 0.9, 0.0, 1.0);	// ��ɫ 3.
vec4 blue = vec4(0.22, 0.35, 0.75, 1.0);	// ��ɫ 4.
vec4 purple = vec4(0.45, 0.22, 0.75, 1.0);	// ��ɫ 5.
vec4 red = vec4(1.0, 0.0, 0.0, 0.8);	// ��ɫ 6.

// ����һ��������ɫ�����飬���������������ɫ��ѡ��
vec4 all_color[] = { orange, green, yellow, blue, purple, red };

float xOffset = 99.0;  // ͬ����ƫ����
int gridWidth = 33.0;  // ���̸��ӵĿ��

// ���������ʾ���̸��ĳλ���Ƿ񱻷�����䣬��board[x][y] = true��ʾ(x,y)�����ӱ���䡣
// �������̸�����½�Ϊԭ�������ϵ��
bool board[10][20];

// �����̸�ĳЩλ�ñ��������֮�󣬼�¼��Щλ���ϱ�������ɫ
vec4 boardcolours[1500];  //1200
// �����̸�ĳЩλ�ñ��������֮�󣬼�¼��Щλ���ϱ�������ɫ
vec4 next_tile_colours[50];  //48

GLuint locxsize;
GLuint locysize;

GLuint vaoIDs[3];
GLuint vboIDs[6];

GLuint borderVBO;
GLuint borderVAO;

// next ��VAO��VBO
GLuint next_vaoIDs;
GLuint next_vboIDs[2];

void showRank();	// չʾ���а�
void changecellcolour(vec2 pos, vec4 colour);
bool checkvalid(vec2 cellpos);
void update_rank();		// ���°�
void checkfullrow(int row);
// ��ǰ�����ƶ�������תʱ������VBO
void updatetile(vec2 tilepos, vec2* tile);
// ������һ������
void update_next_tile(vec2 next_tilepos, vec2* next_tile);
// Ϊ��ǰ�������һ������ɫ
void draw_color_to_tile(int color, int next_color);
// �ж���Ϸ�Ƿ����
void check_gameover();
// �ͷ�OpenGL��Դ�ĺ���
void cleanup();
// GL������
void checkGLError();


// ��ԭ���Ķ�ά��������ά���飬��ʾ7����״�Լ����ǵı��ε���״
vec2 allRotationsLshape[7][4][4] =
{
	{
		{ vec2(0, 0), vec2(-1,0), vec2(1, 0), vec2(-1,-1) },	//   "L"
		{ vec2(0, 1), vec2(0, 0), vec2(0,-1), vec2(1, -1) },
		{ vec2(1, 1), vec2(-1,0), vec2(0, 0), vec2(1,  0) },
		{ vec2(-1,1), vec2(0, 1), vec2(0, 0), vec2(0, -1) }
	},
	{
		{ vec2(0, 0), vec2(1, 0), vec2(0, -1), vec2(-1, -1) },	//   "S"
		{ vec2(0, 0), vec2(1, 0), vec2(0,  1), vec2(1,  -1) },
		{ vec2(0, 0), vec2(1, 0), vec2(0, -1), vec2(-1, -1) },
		{ vec2(0, 0), vec2(1, 0), vec2(0,  1), vec2(1,  -1) }
	},
	{
		{ vec2(0, 0), vec2(-1, 0), vec2(0, -1), vec2(1, -1) },	//   "Z"
		{ vec2(0, 0), vec2(1, 0), vec2(1,  1), vec2(0,  -1) },
		{ vec2(0, 0), vec2(-1, 0), vec2(0, -1), vec2(1, -1) },
		{ vec2(0, 0), vec2(1, 0), vec2(1,  1), vec2(0,  -1) },
	},
	{
		{ vec2(0,  0), vec2(-1,0), vec2(1, 0), vec2(1, -1) },	//   "J"
		{ vec2(0,  1), vec2(0, 0), vec2(0,-1), vec2(1,  1) },
		{ vec2(-1, 1), vec2(-1,0), vec2(0, 0), vec2(1,  0) },
		{ vec2(-1,-1), vec2(0, 1), vec2(0, 0), vec2(0, -1) }
	},
	{
		{ vec2(0,  0), vec2(0,-1), vec2(-1, 0), vec2(-1, -1) },	//   "O"
		{ vec2(0,  0), vec2(0,-1), vec2(-1, 0), vec2(-1, -1) },
		{ vec2(0,  0), vec2(0,-1), vec2(-1, 0), vec2(-1, -1) },
		{ vec2(0,  0), vec2(0,-1), vec2(-1, 0), vec2(-1, -1) },
	},
	{
		{ vec2(-2,  0), vec2(-1,0), vec2(0, 0), vec2(1, 0) },	//   "I"
		{ vec2(0,  1), vec2(0, 0), vec2(0,-1), vec2(0, -2) },
		{ vec2(-2,  0), vec2(-1,0), vec2(0, 0), vec2(1, 0) },
		{ vec2(0,  1), vec2(0, 0), vec2(0,-1), vec2(0, -2) },
	},
	{
		{ vec2(0,  0), vec2(-1,0), vec2(1, 0), vec2(0, -1) },	//   "T"
		{ vec2(0,  1), vec2(0, 0), vec2(0,-1), vec2(1,  0) },
		{ vec2(0,  1), vec2(-1,0), vec2(0, 0), vec2(1,  0) },
		{ vec2(-1, 0), vec2(0, 1), vec2(0, 0), vec2(0, -1) }
	}
};

class Tetromino {
public:
	Tetromino();
	Tetromino(int color, int shape);
	void rotate();		// ��ת�任
	void setTile(vec2 *tile);
	void settile();
	bool movetile(vec2 direction);		// �����ƶ�
	void handleSpecialKey(int key, int x, int y);		// �������ⰴ����Ӧ����
	void TimeFunc(int val);		//
	int getnext_color();
	int getnext_shape();
	
private:
	// ��ǰ������Ϣ
	int color; //������ɫ
	int shape; //������״
	vec2 tile[4];		// ��ǰ�����еķ���
	vec2 tilepos = vec2(5, 19);	// ��ǰ�����λ�ã������̸�����½�Ϊԭ�������ϵ��

	int rotation = 0;	// ���Ƶ�ǰ�����еķ�����ת
	
	// ��һ����2������Ϣ
	int next_color; // ������ɫ
	int next_shape; // ������״
	vec2 next_tile[4];	// ��һ�������еķ���
	vec2 next_tilepos = vec2(10.5, 10);		// ��һ�������λ�ã������̸�����½�Ϊԭ�������ϵ��
};

// ȫ��ָ�룬ָ��ǰTetromino����
Tetromino* currentTetromino = nullptr;

// �޲ι��캯��
Tetromino::Tetromino() {}
// �вι��캯��
Tetromino::Tetromino(int color, int shape) : color(color), shape(shape)
{
	srand(time(NULL));	// �������������
	// ����������һ���������״����ɫ�������
	next_shape = rand() % 7;
	next_color = rand() % 6;

	// ���·���������̸���������м�λ�ò�����Ĭ�ϵ���ת����
	tilepos = vec2(5, 19);
	rotation = 0;

	for (int i = 0; i < 4; i++)
	{
		// ����ĵ�һά���±�random_shape�����������״
		tile[i] = allRotationsLshape[shape][0][i];
	}
	for (int i = 0; i < 4; i++)
	{
		// ����ĵ�һά���±�random_shape�����������״
		next_tile[i] = allRotationsLshape[next_shape][0][i];
	}

	updatetile(tilepos, tile);			// ���µ�ǰ����
	update_next_tile(next_tilepos, next_tile);		// ������һ������
	// Ϊ��ǰ�������һ������ɫ
	draw_color_to_tile(color, next_color);
}

int Tetromino::getnext_color() {
	return next_color;
}
int Tetromino::getnext_shape() {
	return next_shape;
}

void Tetromino::setTile(vec2 *tile) {
	for (int i = 0; i < 4; i++)
	{
		// ����ĵ�һά���±�random_shape�����������״
		this->tile[i] = tile[i];
	}
}

//////////////////////////////////////////////////////////////////////////
// �����������㹻�ռ���������ת��ǰ����
void Tetromino::rotate() {
	// ������ת������ǰ��������Ϊ��ת֮��ķ���
	rotation = (rotation + 1) % 4;

	// ��鵱ǰ��ת֮���λ�õ���Ч��
	// �˴�����˵�һά��random_shape
	if (checkvalid((allRotationsLshape[shape][rotation][0]) + tilepos)
		&& checkvalid((allRotationsLshape[shape][rotation][1]) + tilepos)
		&& checkvalid((allRotationsLshape[shape][rotation][2]) + tilepos)
		&& checkvalid((allRotationsLshape[shape][rotation][3]) + tilepos))
	{
		for (int i = 0; i < 4; i++)
			// �˴�����˵�һά��random_shape
			tile[i] = allRotationsLshape[shape][rotation][i];

		updatetile(tilepos, tile);
	}
}

//////////////////////////////////////////////////////////////////////////
// ���õ�ǰ���飬���Ҹ������̸��Ӧλ�ö������ɫVBO

void Tetromino::settile()
{
	// ÿ������
	for (int i = 0; i < 4; i++)
	{
		// ��ȡ���������̸��ϵ�����
		int x = (tile[i] + tilepos).x;
		int y = (tile[i] + tilepos).y;
		// �����Ӷ�Ӧ�����̸��ϵ�λ������Ϊ���
		board[x][y] = true;
		// ������Ӧλ�õ���ɫ�޸�
		changecellcolour(vec2(x, y), all_color[color]);	// �˴���Ϊ random_color
	}
}

//////////////////////////////////////////////////////////////////////////
// ����λ��(x,y)���ƶ����顣��Ч���ƶ�ֵΪ(-1,0)��(1,0)��(0,-1)���ֱ��Ӧ����
// �����Һ������ƶ�������ƶ��ɹ�������ֵΪtrue����֮Ϊfalse��

bool Tetromino::movetile(vec2 direction)
{
	// �����ƶ�֮��ķ����λ������
	vec2 newtilepos[4];
	for (int i = 0; i < 4; i++)
		newtilepos[i] = tile[i] + tilepos + direction;

	// ����ƶ�֮�����Ч��
	if (checkvalid(newtilepos[0])
		&& checkvalid(newtilepos[1])
		&& checkvalid(newtilepos[2])
		&& checkvalid(newtilepos[3]))
	{
		// ��Ч���ƶ��÷���
		tilepos.x = tilepos.x + direction.x;
		tilepos.y = tilepos.y + direction.y;
		
		updatetile(tilepos, tile);
		return true;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
// ������Ӧ�¼��е����ⰴ����Ӧ
void Tetromino::handleSpecialKey(int key, int x, int y)
{
	if (!gameover && !stop)
	{
		switch (key)
		{
		case GLUT_KEY_UP:	// ���ϰ�����ת����
			rotate();
			break;
		case GLUT_KEY_DOWN: // ���°����ƶ�����
			if (!movetile(vec2(0, -1)))
			{
				settile();
				checkfullrow(0);	// ����������
				// ���ջ��ּ������������
				int plus_score = 5 * clean_up_row * (clean_up_row + 1);
				score += plus_score;
				if(plus_score > 0)
					std::cout << "plus: " << plus_score << ", current score: " << score << endl;
				sprintf(score_out, "%d", score);	// �����ָ�ֵ��buffer
				glutSwapBuffers();		// ˢ�½���
				clean_up_row = 0;		// ����clearupflag

				int next_color = currentTetromino->getnext_color();
				int next_shape = currentTetromino->getnext_shape();
				// ɾ���ɶ���(�ڴ����)
				if (currentTetromino != nullptr) {
					delete currentTetromino;
					currentTetromino = nullptr;
				}
				check_gameover();
				currentTetromino = new Tetromino(next_color, next_shape);
			}
			break;
		case GLUT_KEY_LEFT:  // ���󰴼��ƶ�����
			movetile(vec2(-1, 0));
			break;
		case GLUT_KEY_RIGHT: // ���Ұ����ƶ�����
			movetile(vec2(1, 0));
			break;
		}
	}
}

// �˺���ʵ��ÿ��500���뷽���Զ�����һ��
void Tetromino::TimeFunc(int val)
{
	if (!gameover && !stop) {
		// ����һ���ʱ����Ҫ����Ƿ񵽵ף����׵Ļ��Ƿ�������������������
		if (!movetile(vec2(0, -1)))
		{
			settile();
			checkfullrow(0);
			int plus_score = 5 * clean_up_row * (clean_up_row + 1);
			score += plus_score;
			if (plus_score > 0)
				std::cout << "plus: " << plus_score << ", current score: " << score << endl;
			sprintf(score_out, "%d", score);
			glutSwapBuffers();
			clean_up_row = 0;

			int next_color = currentTetromino->getnext_color();
			int next_shape = currentTetromino->getnext_shape();
			// ɾ���ɶ���(�ڴ����)
			if (currentTetromino != nullptr) {
				delete currentTetromino;
				currentTetromino = nullptr;
			}
			check_gameover();
			currentTetromino = new Tetromino(next_color, next_shape);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
// �޸����̸���posλ�õ���ɫΪcolour�����Ҹ��¶�Ӧ��VBO

void changecellcolour(vec2 pos, vec4 colour)
{
	// ÿ�������Ǹ������Σ��������������Σ��ܹ�6�����㣬�����ض���λ�ø����ʵ�����ɫ
	for (int i = 0; i < 6; i++)
		boardcolours[(int)(6 * (10 * pos.y + pos.x) + i)] = colour;

	vec4 newcolours[6] = { colour, colour, colour, colour, colour, colour };

	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[3]);

	// ����ƫ���������ʵ���λ�ø�����ɫ
	int offset = 6 * sizeof(vec4) * (int)(10 * pos.y + pos.x);
	glBufferSubData(GL_ARRAY_BUFFER, offset, sizeof(newcolours), newcolours);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


// �����õĺ���
bool compare(int a, int b)
{
	return a > b;
}

// �������а�
void update_rank() {
	score_rank[3] = score;
	// �Է������а��������
	sort(score_rank, score_rank + 4, compare);
	ofstream in;
	//ios::trunc��ʾ�ڴ��ļ�ǰ���ļ����,������д��,�ļ��������򴴽�
	in.open("score_rank.txt", ios::trunc);

	// д��txt�ı��м�¼
	for (int i = 0; i < 3; i++) {
		//cout << score_rank[i] << endl;
		if (i == 0) {
			in << score_rank[i];
		}
		else
			in << '\n' << score_rank[i];
	}
	// չʾ���º�����а�
	showRank();
}

//////////////////////////////////////////////////////////////////////////
// ��Ϸ��OpenGL��ʼ��
void init()
{
	std::cout << "GAME START" << endl;
	// ��ʼ�����̸񣬰���64���������꣨�ܹ�32���ߣ�������ÿ������һ����ɫֵ
	vec4 gridpoints[64];
	vec4 gridcolours[64];
	

	// ������
	for (int i = 0; i < 11; i++)
	{
		gridpoints[2*i] = vec4((33.0- xOffset + (33.0 * i)), 33.0, 0, 1);
		gridpoints[2*i + 1] = vec4((33.0- xOffset + (33.0 * i)), 693.0, 0, 1);
		
	}

	// ˮƽ��
	for (int i = 0; i < 21; i++)
	{
		gridpoints[22 + 2*i] = vec4(33.0- xOffset, (33.0 + (33.0 * i)), 0, 1);
		gridpoints[22 + 2*i + 1] = vec4(363.0- xOffset, (33.0 + (33.0 * i)), 0, 1);
	}

	// �������߸��ɺ�ɫ
	for (int i = 0; i < 64; i++)
		gridcolours[i] = black;
	
	// ��ʼ�����̸񣬲���û�б����ĸ������óɰ�ɫ
	vec4 boardpoints[1200];
	for (int i = 0; i < 1200; i++)
		boardcolours[i] = white;

	// ��ÿ�����ӣ���ʼ��6�����㣬��ʾ���������Σ�����һ�������θ���
	for (int i = 0; i < 20; i++)
		for (int j = 0; j < 10; j++)
		{
			vec4 p1 = vec4(33.0 - xOffset + (j * 33.0), 33.0 + (i * 33.0), .5, 1);
			vec4 p2 = vec4(33.0 - xOffset + (j * 33.0), 66.0 + (i * 33.0), .5, 1);
			vec4 p3 = vec4(66.0 - xOffset + (j * 33.0), 33.0 + (i * 33.0), .5, 1);
			vec4 p4 = vec4(66.0 - xOffset + (j * 33.0), 66.0 + (i * 33.0), .5, 1);

			boardpoints[6*(10*i + j)    ] = p1;
			boardpoints[6*(10*i + j) + 1] = p2;
			boardpoints[6*(10*i + j) + 2] = p3;
			boardpoints[6*(10*i + j) + 3] = p2;
			boardpoints[6*(10*i + j) + 4] = p3;
			boardpoints[6*(10*i + j) + 5] = p4;
		}

	// �����̸�����λ�õ�����������Ϊfalse��û�б���䣩
	for (int i = 0; i < 10; i++)
		for (int j = 0; j < 20; j++)
			board[i][j] = false;

	// ������ɫ�� (�˴��޸ĳ�ȫ�ֱ���)
	program = InitShader("vshader.glsl", "fshader.glsl");
	glUseProgram(program);

	locxsize = glGetUniformLocation(program, "xsize");
	locysize = glGetUniformLocation(program, "ysize");

	GLuint vPosition = glGetAttribLocation(program, "vPosition");	// ��ö������Ե���ڵ�
	GLuint vColor = glGetAttribLocation(program, "vColor");			// ���ݶ�������	

	glGenVertexArrays(3, &vaoIDs[0]);	// ���ɶ�������
	
	// ���̸񶥵�
	glBindVertexArray(vaoIDs[0]);	// VAO ��һ�����󣬰���һ���� VBO
	glGenBuffers(2, vboIDs);	// �����»������
	checkGLError();

	// ���̸񶥵�λ��
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[0]);	// �󶨻������
	glBufferData(GL_ARRAY_BUFFER, 64*sizeof(vec4), gridpoints, GL_STATIC_DRAW);; // ���������ݿ��������������
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, 0); 		// ���ݶ�������
	glEnableVertexAttribArray(vPosition);								// ���ݶ�������
	
	// ���̸񶥵���ɫ
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[1]);
	glBufferData(GL_ARRAY_BUFFER, 64*sizeof(vec4), gridcolours, GL_STATIC_DRAW);
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vColor);
	
	// ���̸�ÿ������	
	glBindVertexArray(vaoIDs[1]);
	glGenBuffers(2, &vboIDs[2]);
	checkGLError();

	// ���̸�ÿ�����Ӷ���λ��
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[2]);
	glBufferData(GL_ARRAY_BUFFER, 1200*sizeof(vec4), boardpoints, GL_STATIC_DRAW);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vPosition);

	// ���̸�ÿ�����Ӷ�����ɫ
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[3]);
	glBufferData(GL_ARRAY_BUFFER, 1200*sizeof(vec4), boardcolours, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vColor);
	
	// ��ǰ����
	glBindVertexArray(vaoIDs[2]);
	glGenBuffers(2, &vboIDs[4]);
	checkGLError();

	// ��ǰ���鶥��λ��
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[4]);
	glBufferData(GL_ARRAY_BUFFER, 24*sizeof(vec4), NULL, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vPosition);

	// ��ǰ���鶥����ɫ
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[5]);
	glBufferData(GL_ARRAY_BUFFER, 24*sizeof(vec4), NULL, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vColor);

	// ��һ������
	glBindVertexArray(next_vaoIDs);
	glGenBuffers(2, &next_vboIDs[0]);
	checkGLError();

	// ��һ�����鶥��λ��
	glBindBuffer(GL_ARRAY_BUFFER, next_vboIDs[0]);
	glBufferData(GL_ARRAY_BUFFER, 24 * sizeof(vec4), NULL, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vPosition);

	// ��һ�����鶥����ɫ
	glBindBuffer(GL_ARRAY_BUFFER, next_vboIDs[1]);
	glBufferData(GL_ARRAY_BUFFER, 24 * sizeof(vec4), NULL, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vColor);

	glBindVertexArray(0);
	glClearColor(0, 0, 0, 0);

	// ��һ������ı߿�
	glGenBuffers(1, &borderVBO);
	glGenVertexArrays(1, &borderVAO);
	glBindVertexArray(borderVAO);
	glBindBuffer(GL_ARRAY_BUFFER, borderVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec4) * 8 * 2, NULL, GL_DYNAMIC_DRAW); // 8�����㣬ÿ����������vec4��λ�ú���ɫ��
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vPosition);
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vColor);
	glBindVertexArray(0);

	// ��Ϸ��ʼ��
	int next_color = currentTetromino->getnext_color();
	int next_shape = currentTetromino->getnext_shape();
	// ɾ���ɶ���(�ڴ����)
	if (currentTetromino != nullptr) {
		delete currentTetromino;
		currentTetromino = nullptr;
	}
	check_gameover();
	currentTetromino = new Tetromino(next_color, next_shape);

	gameover = false;	// ����ж���Ϸ�����ı�־
	stop = false;	// ��ͣ�÷�
	score = 0;		// ��ʼ������Ϊ0
	strcpy(score_out, "0");	// ��ʼ����ʾ����Ϊ0
	strcpy(gameover_tip, "");	// ��ʼ����ʾgameover����ϢΪ��
	al_update_rank = false;		// ���������Ѹ������а���

	// �Ӽ�¼txt�ж�ȡ�������а�
	ifstream in("score_rank.txt");
	string s;
	int i = 0;
	while (getline(in, s)) {
		//���ж�ȡ���ݲ�����s�У�ֱ������ȫ����ȡ
		score_rank[i++] = stoi(s);
		if (i == 3)	break;
	}
	// glut����ʾ��Ϣ
	glutSwapBuffers();

	starttime = glutGet(GLUT_ELAPSED_TIME);
}

//////////////////////////////////////////////////////////////////////////
// �����cellposλ�õĸ����Ƿ��������Ƿ������̸�ı߽緶Χ�ڡ�

// �˴����һ������ʹ����������ϵ���
bool checkvalid(vec2 cellpos)
{
	// �˴����һ������ʹ����������ϵ���
	if ((cellpos.x >= 0) && (cellpos.x < 10) && (cellpos.y >= 0) && (cellpos.y < 20)
		&& (board[(int)cellpos.x][(int)cellpos.y] == false))
		return true;
	else
		return false;
}

//////////////////////////////////////////////////////////////////////////
// ������̸���row����û�б������

// �˴��Ե����ϱ���ÿһ�У����ÿһ�еķ���ı�����
// ���Ǵ��ڶ���ͬʱ�����������
// �˴�д�ɵݹ����ʽ�Ƚϼ�
void checkfullrow(int row)
{
	// ������һ�о�ֱ�ӷ���
	if (row == 19)return;
	// �Ե����ϼ��ÿһ��
	for (int i = 0; i < 10; i++) {
		if (board[i][row] == false) {
			// �����һ��
			checkfullrow(row + 1);
			return;
		}
	}
	// ������ֵı��++
	clean_up_row++;

	// ���㱻clean���������������
	// ���������Ե��������ΰ���һ�е���ɫ�ͱ����Ϣ��ֵ����һ��
	for (int i = row; i < 19; i++) {
		for (int j = 0; j < 10; j++) {
			vec4 color = boardcolours[(int)(6 * (10 * (i + 1) + j))];
			changecellcolour(vec2(j, i), color);
			board[j][i] = board[j][i + 1];
		}
	}
	// ��ʱ�ĵ�ǰ��row�ͳ�Ϊδ��������һ�У����Ǽ����ݹ�
	checkfullrow(row);
}

//////////////////////////////////////////////////////////////////////////
// ����������Ϸ

void restart()
{
	// ����Ƿ�������а����û�б����
	if (!al_update_rank) {
		update_rank();
		al_update_rank = true;
	}
	init();	// ���³�ʼ������
}

// �ж���Ϸ�Ƿ����
void check_gameover() {
	// �����̶����λ�ñ�����ռ�ţ���Ϸ�����
	if (board[5][19] || board[6][19]) {
		std::cout << "GAME OVER, finally score: " << score << endl;
		gameover = true;
		strcpy(gameover_tip, "gameover");
		glutSwapBuffers();
		// ���δ�������а񣬱�������а�
		if (!al_update_rank) {
			update_rank();
			al_update_rank = true;
		}
		return;
	}
}

//////////////////////////////////////////////////////////////////////////
// ��Ϸ��Ⱦ����

// �˺�����������Ļ�����������Ϣ
void showText(float x, float y, const char* str, float rcolor, float gcolor, float bcolor) {
	glUseProgram(0);
	glColor3f(rcolor, gcolor, bcolor);	// ������ɫ
	glRasterPos2f(x, y);			// ������ʾ[����]ʱ�����ַ�����ʼλ��
	int strLen = strlen(str);
	for (int i = 0; i<strLen; i++) {
		// ��ʾ�ַ�������-> GLUT_BITMAP_TIMES_ROMAN�������С-> 24
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, str[i]);
	}
	glFlush();
	glUseProgram(program);
}

// �˺�����������Ļ����ʾ�������а�
void showRank() {
	showText(0.40, -0.5, "Top3 Scores:", 1.0, 1.0, 1.0);
	for (int i = 0; i < 3; i++) {
		char ss[100];
		char ii[100];
		char nn[100] = "NO.";
		itoa(i+1, ii, 10);
		strcat(nn, ii);
		itoa(score_rank[i], ss, 10);
		showText(0.40, -0.6 - i*0.1, nn, 1.0, 1.0, 1.0);
		showText(0.60, -0.6 - i*0.1, ss, 1.0, 1.0, 1.0);
	}
}

void display()
{
	glClear(GL_COLOR_BUFFER_BIT);

	glUniform1i(locxsize, xsize);
	glUniform1i(locysize, ysize);

	glBindVertexArray(vaoIDs[1]);
	glDrawArrays(GL_TRIANGLES, 0, 1200); // �������̸� (10*20*2 = 400 ��������)

	glBindVertexArray(vaoIDs[2]);
	glDrawArrays(GL_TRIANGLES, 0, 24);	 // ���Ƶ�ǰ���� (8 ��������)

	glBindVertexArray(next_vaoIDs);
	glDrawArrays(GL_TRIANGLES, 0, 24);	 // ������һ������ (8 ��������)

	glBindVertexArray(borderVAO);
	glDrawArrays(GL_LINES, 0, 8); // ���Ʊ߿�
	glBindVertexArray(0);

	glBindVertexArray(vaoIDs[0]);
	glDrawArrays(GL_LINES, 0, 64);		 // �������̸����

	// �˴�showText�ѷ�����������Ϣ��ʾ�ڽ�����
	showText(0.40, 0.75, score_tip, 1.0, 1.0, 1.0);
	showText(0.55, 0.65, score_out, 0.0, 1.0, 1.0);
	showText(0.40, 0.55, gameover_tip, 0.0, 1.0, 1.0);
	showText(0.40, 0.2, next_tip, 1.0, 1.0, 1.0);
	showText(-0.95, -0.97, keyboard_tip, 1.0, 1.0, 1.0);

	// ��ʾ�������а�
	showRank();

	glutSwapBuffers();  // ÿ�ε����л�ͼ�������ں�̨�����н��У� ���������ʱ�� 
						//�ѻ��Ƶ����ս�����Ƶ���Ļ�ϣ� ������ ���ǿ�������GDIԪ��ͬʱ��������Ļ��
}

//////////////////////////////////////////////////////////////////////////
// ��ǰ�����ƶ�������תʱ������VBO
void updatetile(vec2 tilepos, vec2 *tile)
{
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[4]);

	// ÿ����������ĸ�����
	for (int i = 0; i < 4; i++)
	{
		// ������ӵ�����ֵ
		GLfloat x = tilepos.x + tile[i].x - xOffset / 33.0;
		GLfloat y = tilepos.y + tile[i].y;

		vec4 p1 = vec4(33.0 + (x * 33.0), 33.0 + (y * 33.0), .4, 1);
		vec4 p2 = vec4(33.0 + (x * 33.0), 66.0 + (y * 33.0), .4, 1);
		vec4 p3 = vec4(66.0 + (x * 33.0), 33.0 + (y * 33.0), .4, 1);
		vec4 p4 = vec4(66.0 + (x * 33.0), 66.0 + (y * 33.0), .4, 1);

		// ÿ�����Ӱ������������Σ�������6����������
		vec4 newpoints[6] = { p1, p2, p3, p2, p3, p4 };
		glBufferSubData(GL_ARRAY_BUFFER, i * 6 * sizeof(vec4), 6 * sizeof(vec4), newpoints);
	}
	glBindVertexArray(0);
}

// ������һ������
void update_next_tile(vec2 next_tilepos, vec2 *next_tile) {

	glBindBuffer(GL_ARRAY_BUFFER, next_vboIDs[0]);

	// ÿ����������ĸ�����
	for (int i = 0; i < 4; i++)
	{

		// ������ӵ�����ֵ
		GLfloat x = next_tilepos.x + next_tile[i].x;
		GLfloat y = next_tilepos.y + next_tile[i].y;

		vec4 p1 = vec4(33.0 + (x * 33.0), 33.0 + (y * 33.0), .4, 1);
		vec4 p2 = vec4(33.0 + (x * 33.0), 66.0 + (y * 33.0), .4, 1);
		vec4 p3 = vec4(66.0 + (x * 33.0), 33.0 + (y * 33.0), .4, 1);
		vec4 p4 = vec4(66.0 + (x * 33.0), 66.0 + (y * 33.0), .4, 1);

		// ÿ�����Ӱ������������Σ�������6����������
		vec4 newpoints[6] = { p1, p2, p3, p2, p3, p4 };
		glBufferSubData(GL_ARRAY_BUFFER, i * 6 * sizeof(vec4), 6 * sizeof(vec4), newpoints);
	}
	glBindVertexArray(0);

	// ���Ʊ߿�
	for (int i = 0; i < 4; i++) {
		// ����ÿ��С����ı߽�
		GLfloat x = next_tilepos.x + next_tile[i].x;
		GLfloat y = next_tilepos.y + next_tile[i].y;

		vec4 p1 = vec4(33.0 + (x * 33.0), 33.0 + (y * 33.0), 0.5, 1);
		vec4 p2 = vec4(66.0 + (x * 33.0), 33.0 + (y * 33.0), 0.5, 1);
		vec4 p3 = vec4(66.0 + (x * 33.0), 66.0 + (y * 33.0), 0.5, 1);
		vec4 p4 = vec4(33.0 + (x * 33.0), 66.0 + (y * 33.0), 0.5, 1);

		vec4 borderPoints[8] = { p1, p2, p2, p3, p3, p4, p4, p1 };
		vec4 borderColors[8] = { 2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0 };
	}
}

// Ϊ��ǰ�������һ������ɫ
void draw_color_to_tile(int color, int next_color) {
	// ���·��鸳����ɫ
	vec4 newcolours[24];
	for (int i = 0; i < 24; i++)
		// ������±�random_color�����������ɫ
		newcolours[i] = all_color[color];

	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[5]);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(newcolours), newcolours);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	// ����һ���·��鸳����ɫ
	for (int i = 0; i < 24; i++)
		// ������±�random_color�����������ɫ
		newcolours[i] = all_color[next_color];

	glBindBuffer(GL_ARRAY_BUFFER, next_vboIDs[1]);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(newcolours), newcolours);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

//////////////////////////////////////////////////////////////////////////
// �ڴ��ڱ������ʱ�򣬿������̸�Ĵ�С��ʹ֮���̶ֹ��ı�����

void reshape(GLsizei w, GLsizei h)
{
	xsize = w;
	ysize = h;
	glViewport(0, 0, w, h);
}


//////////////////////////////////////////////////////////////////////////
// ������Ӧʱ���е���ͨ������Ӧ

void keyboard(unsigned char key, int x, int y)
{
	switch(key) 
	{
		case 033: // ESC�� �� 'q' ���˳���Ϸ���˳�ǰ���°񵥣�
		case 'q':
			// ���δ�������а񣬱�������а�
			if (!al_update_rank) {
				update_rank();
				al_update_rank = true;
			}
			// ɾ�������˳�
			if (currentTetromino != nullptr) {
				delete currentTetromino;
				currentTetromino = nullptr;
			}
			exit (EXIT_SUCCESS);
			break;
		case 'p':
			if(!stop)
				strcpy(gameover_tip, "game stop");
			else
				strcpy(gameover_tip, "");
			stop = !stop;
			glutSwapBuffers();
			break;
		case 'r': // 'r' ��������Ϸ
			restart();
			break;
	}
	glutPostRedisplay();
}

void idle(void)
{
	glutPostRedisplay();
}

// �ͷ�OpenGL��Դ�ĺ���
void cleanup() {
	glDeleteBuffers(6, vboIDs);
	glDeleteVertexArrays(3, vaoIDs);
	glDeleteProgram(program);
	glDeleteBuffers(1, &borderVBO);
	glDeleteVertexArrays(1, &borderVAO);

	if (currentTetromino != nullptr) {
		delete currentTetromino;
		currentTetromino = nullptr;
	}
}
// GL������
void checkGLError() {
	GLenum err;
	while ((err = glGetError()) != GL_NO_ERROR) {
		std::cerr << "OpenGL error: " << gluErrorString(err) << std::endl;
	}
}

//////////////////////////////////////////////////////////////////////////
// �˴�����opengl�����к���ͨ�ţ���֤���ݵķ�װ
// ��װ�������ⰴ����Ӧ����
void specialKeyWrapper(int key, int x, int y) {
	if (currentTetromino != nullptr) {
		currentTetromino->handleSpecialKey(key, x, y);
	}
}
// ��װʱ�䴦��
void TimeFuncWrapper(int val) {
	if (currentTetromino != nullptr) {
		currentTetromino->TimeFunc(val);
	}
	// �����������100�Ļ������ӷ��������ٶ�
	// �Դ�������Ϸ�Ѷ�
	if (score >= 100) {
		glutTimerFunc(300, TimeFuncWrapper, 1);
	}
	else {
		// �ڽ�β�ظ����ü�ʱ������
		glutTimerFunc(500, TimeFuncWrapper, 1);
	}
}
//////////////////////////////////////////////////////////////////////////

int main(int argc, char **argv)
{
	std::ofstream logFile("log.txt");
	logFile << "Program started" << std::endl;

	try {
		glutInit(&argc, argv);
		glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
		glutInitWindowSize(xsize, ysize);
		glutInitWindowPosition(680, 178);
		glutCreateWindow("Tetromino");
		glewInit();

		srand(time(NULL));	// �������������
		// ����0-6������������������״
		int shape = rand() % 7;
		// ����0-5������������������ɫ
		int color = rand() % 6;
		currentTetromino = new Tetromino(shape, color); // ʵ�������󲢸�ֵ��ȫ��ָ��
		init();

		glutDisplayFunc(display);	// display
		glutReshapeFunc(reshape);	// ���ڴ�С�仯
		glutSpecialFunc(specialKeyWrapper);	// ��Ӧ�������
		glutKeyboardFunc(keyboard);	// ��Ӧr��q
		glutIdleFunc(idle);			// ����ȫ�ֵĻص�����

		glutTimerFunc(500, TimeFuncWrapper, 1);// �˴������˵��ü�ʱ������

		glutMainLoop();			// ����GLUT�¼�����ѭ��
	}
	catch (const std::exception& e) {
		std::cerr << "Exception caught: " << e.what() << std::endl;
		logFile << "Exception caught: " << e.what() << std::endl;
		// ���ܵ�������
		cleanup();
		exit(EXIT_FAILURE);
	}

	// ���˳�ǰ������Դ(openGL��Դ)
	cleanup();

	// ���˳�ǰ�ر���־�ļ�
	logFile << "Program finished" << std::endl;
	logFile.close();

	return 0;
}