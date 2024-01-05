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

int starttime;			// 控制方块向下移动时间
bool gameover = false;	// 游戏结束控制变量
bool stop = false;		// 游戏暂停变量

int xsize = 580;		// 窗口大小
int ysize = 725;

int clean_up_row = 0;		// 记录每一次被消除的行数
int score = 0;				// 记录游戏得分
char score_tip[100] = "current score:";	// 界面输出所需
char next_tip[100] = "next:";	// 界面输出所需
char gameover_tip[100] = "";	// 界面输出所需
char keyboard_tip[100] = "stop/continue: P   restart: R   quit: Q";	// 界面输出：按键提示
char score_out[100] = "0";		// 界面输出游戏得分
int score_rank[4] = { 0 };	// 记录游戏得分排行榜
bool al_update_rank = false;// 记录是否update了排行榜，防止重复update

GLuint program;		// 此处将着色器变为全局变量，以便使用

// 绘制窗口的颜色变量
vec4 orange = vec4(1.0, 0.5, 0.0, 1.0); // 橙色 1 (注释的编号代表数组下标)
vec4 white = vec4(1.0, 1.0, 1.0, 1.0);	// 白色
vec4 black = vec4(0.0, 0.0, 0.0, 1.0);	// 黑色
vec4 green = vec4(0.39, 0.75, 0.22, 1.0);	// 绿色 2.
vec4 yellow = vec4(1.0, 0.9, 0.0, 1.0);	// 黄色 3.
vec4 blue = vec4(0.22, 0.35, 0.75, 1.0);	// 蓝色 4.
vec4 purple = vec4(0.45, 0.22, 0.75, 1.0);	// 紫色 5.
vec4 red = vec4(1.0, 0.0, 0.0, 0.8);	// 红色 6.

// 定义一个所有颜色的数组，方便下面做随机颜色的选择
vec4 all_color[] = { orange, green, yellow, blue, purple, red };

float xOffset = 99.0;  // 同样的偏移量
int gridWidth = 33.0;  // 棋盘格子的宽度

// 布尔数组表示棋盘格的某位置是否被方块填充，即board[x][y] = true表示(x,y)处格子被填充。
// （以棋盘格的左下角为原点的坐标系）
bool board[10][20];

// 当棋盘格某些位置被方块填充之后，记录这些位置上被填充的颜色
vec4 boardcolours[1500];  //1200
// 当棋盘格某些位置被方块填充之后，记录这些位置上被填充的颜色
vec4 next_tile_colours[50];  //48

GLuint locxsize;
GLuint locysize;

GLuint vaoIDs[3];
GLuint vboIDs[6];

GLuint borderVBO;
GLuint borderVAO;

// next 的VAO、VBO
GLuint next_vaoIDs;
GLuint next_vboIDs[2];

void showRank();	// 展示排行榜
void changecellcolour(vec2 pos, vec4 colour);
bool checkvalid(vec2 cellpos);
void update_rank();		// 更新榜单
void checkfullrow(int row);
// 当前方块移动或者旋转时，更新VBO
void updatetile(vec2 tilepos, vec2* tile);
// 更新下一个方块
void update_next_tile(vec2 next_tilepos, vec2* next_tile);
// 为当前方块和下一方块着色
void draw_color_to_tile(int color, int next_color);
// 判断游戏是否结束
void check_gameover();
// 释放OpenGL资源的函数
void cleanup();
// GL错误检查
void checkGLError();


// 将原本的二维数组变成三维数组，表示7种形状以及它们的变形的形状
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
	void rotate();		// 旋转变换
	void setTile(vec2 *tile);
	void settile();
	bool movetile(vec2 direction);		// 方块移动
	void handleSpecialKey(int key, int x, int y);		// 处理特殊按键响应函数
	void TimeFunc(int val);		//
	int getnext_color();
	int getnext_shape();
	
private:
	// 当前窗口信息
	int color; //方块颜色
	int shape; //方块形状
	vec2 tile[4];		// 当前窗口中的方块
	vec2 tilepos = vec2(5, 19);	// 当前方块的位置（以棋盘格的左下角为原点的坐标系）

	int rotation = 0;	// 控制当前窗口中的方块旋转
	
	// 下一窗口2方块信息
	int next_color; // 方块颜色
	int next_shape; // 方块形状
	vec2 next_tile[4];	// 下一个窗口中的方块
	vec2 next_tilepos = vec2(10.5, 10);		// 下一个方块的位置（以棋盘格的左下角为原点的坐标系）
};

// 全局指针，指向当前Tetromino对象
Tetromino* currentTetromino = nullptr;

// 无参构造函数
Tetromino::Tetromino() {}
// 有参构造函数
Tetromino::Tetromino(int color, int shape) : color(color), shape(shape)
{
	srand(time(NULL));	// 设置随机数种子
	// 产生控制下一个方块的形状和颜色的随机数
	next_shape = rand() % 7;
	next_color = rand() % 6;

	// 将新方块放于棋盘格的最上行中间位置并设置默认的旋转方向
	tilepos = vec2(5, 19);
	rotation = 0;

	for (int i = 0; i < 4; i++)
	{
		// 这里的第一维的下标random_shape控制着随机形状
		tile[i] = allRotationsLshape[shape][0][i];
	}
	for (int i = 0; i < 4; i++)
	{
		// 这里的第一维的下标random_shape控制着随机形状
		next_tile[i] = allRotationsLshape[next_shape][0][i];
	}

	updatetile(tilepos, tile);			// 更新当前方块
	update_next_tile(next_tilepos, next_tile);		// 更新下一个方块
	// 为当前方块和下一方块着色
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
		// 这里的第一维的下标random_shape控制着随机形状
		this->tile[i] = tile[i];
	}
}

//////////////////////////////////////////////////////////////////////////
// 在棋盘上有足够空间的情况下旋转当前方块
void Tetromino::rotate() {
	// 更新旋转，将当前方块设置为旋转之后的方块
	rotation = (rotation + 1) % 4;

	// 检查当前旋转之后的位置的有效性
	// 此处添加了第一维的random_shape
	if (checkvalid((allRotationsLshape[shape][rotation][0]) + tilepos)
		&& checkvalid((allRotationsLshape[shape][rotation][1]) + tilepos)
		&& checkvalid((allRotationsLshape[shape][rotation][2]) + tilepos)
		&& checkvalid((allRotationsLshape[shape][rotation][3]) + tilepos))
	{
		for (int i = 0; i < 4; i++)
			// 此处添加了第一维的random_shape
			tile[i] = allRotationsLshape[shape][rotation][i];

		updatetile(tilepos, tile);
	}
}

//////////////////////////////////////////////////////////////////////////
// 放置当前方块，并且更新棋盘格对应位置顶点的颜色VBO

void Tetromino::settile()
{
	// 每个格子
	for (int i = 0; i < 4; i++)
	{
		// 获取格子在棋盘格上的坐标
		int x = (tile[i] + tilepos).x;
		int y = (tile[i] + tilepos).y;
		// 将格子对应在棋盘格上的位置设置为填充
		board[x][y] = true;
		// 并将相应位置的颜色修改
		changecellcolour(vec2(x, y), all_color[color]);	// 此处改为 random_color
	}
}

//////////////////////////////////////////////////////////////////////////
// 给定位置(x,y)，移动方块。有效的移动值为(-1,0)，(1,0)，(0,-1)，分别对应于向
// 左，向右和向下移动。如果移动成功，返回值为true，反之为false。

bool Tetromino::movetile(vec2 direction)
{
	// 计算移动之后的方块的位置坐标
	vec2 newtilepos[4];
	for (int i = 0; i < 4; i++)
		newtilepos[i] = tile[i] + tilepos + direction;

	// 检查移动之后的有效性
	if (checkvalid(newtilepos[0])
		&& checkvalid(newtilepos[1])
		&& checkvalid(newtilepos[2])
		&& checkvalid(newtilepos[3]))
	{
		// 有效：移动该方块
		tilepos.x = tilepos.x + direction.x;
		tilepos.y = tilepos.y + direction.y;
		
		updatetile(tilepos, tile);
		return true;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
// 键盘响应事件中的特殊按键响应
void Tetromino::handleSpecialKey(int key, int x, int y)
{
	if (!gameover && !stop)
	{
		switch (key)
		{
		case GLUT_KEY_UP:	// 向上按键旋转方块
			rotate();
			break;
		case GLUT_KEY_DOWN: // 向下按键移动方块
			if (!movetile(vec2(0, -1)))
			{
				settile();
				checkfullrow(0);	// 检查消除情况
				// 按照积分计算规则计算积分
				int plus_score = 5 * clean_up_row * (clean_up_row + 1);
				score += plus_score;
				if(plus_score > 0)
					std::cout << "plus: " << plus_score << ", current score: " << score << endl;
				sprintf(score_out, "%d", score);	// 将积分赋值给buffer
				glutSwapBuffers();		// 刷新界面
				clean_up_row = 0;		// 重置clearupflag

				int next_color = currentTetromino->getnext_color();
				int next_shape = currentTetromino->getnext_shape();
				// 删除旧对象(内存管理)
				if (currentTetromino != nullptr) {
					delete currentTetromino;
					currentTetromino = nullptr;
				}
				check_gameover();
				currentTetromino = new Tetromino(next_color, next_shape);
			}
			break;
		case GLUT_KEY_LEFT:  // 向左按键移动方块
			movetile(vec2(-1, 0));
			break;
		case GLUT_KEY_RIGHT: // 向右按键移动方块
			movetile(vec2(1, 0));
			break;
		}
	}
}

// 此函数实现每隔500毫秒方块自动下落一格
void Tetromino::TimeFunc(int val)
{
	if (!gameover && !stop) {
		// 下落一格的时候，需要检测是否到底，到底的话是否消除，消除便计算分数
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
			// 删除旧对象(内存管理)
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
// 修改棋盘格在pos位置的颜色为colour，并且更新对应的VBO

void changecellcolour(vec2 pos, vec4 colour)
{
	// 每个格子是个正方形，包含两个三角形，总共6个顶点，并在特定的位置赋上适当的颜色
	for (int i = 0; i < 6; i++)
		boardcolours[(int)(6 * (10 * pos.y + pos.x) + i)] = colour;

	vec4 newcolours[6] = { colour, colour, colour, colour, colour, colour };

	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[3]);

	// 计算偏移量，在适当的位置赋上颜色
	int offset = 6 * sizeof(vec4) * (int)(10 * pos.y + pos.x);
	glBufferSubData(GL_ARRAY_BUFFER, offset, sizeof(newcolours), newcolours);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


// 排序用的函数
bool compare(int a, int b)
{
	return a > b;
}

// 更新排行榜
void update_rank() {
	score_rank[3] = score;
	// 对分数排行榜进行排序
	sort(score_rank, score_rank + 4, compare);
	ofstream in;
	//ios::trunc表示在打开文件前将文件清空,由于是写入,文件不存在则创建
	in.open("score_rank.txt", ios::trunc);

	// 写入txt文本中记录
	for (int i = 0; i < 3; i++) {
		//cout << score_rank[i] << endl;
		if (i == 0) {
			in << score_rank[i];
		}
		else
			in << '\n' << score_rank[i];
	}
	// 展示更新后的排行榜
	showRank();
}

//////////////////////////////////////////////////////////////////////////
// 游戏和OpenGL初始化
void init()
{
	std::cout << "GAME START" << endl;
	// 初始化棋盘格，包含64个顶点坐标（总共32条线），并且每个顶点一个颜色值
	vec4 gridpoints[64];
	vec4 gridcolours[64];
	

	// 纵向线
	for (int i = 0; i < 11; i++)
	{
		gridpoints[2*i] = vec4((33.0- xOffset + (33.0 * i)), 33.0, 0, 1);
		gridpoints[2*i + 1] = vec4((33.0- xOffset + (33.0 * i)), 693.0, 0, 1);
		
	}

	// 水平线
	for (int i = 0; i < 21; i++)
	{
		gridpoints[22 + 2*i] = vec4(33.0- xOffset, (33.0 + (33.0 * i)), 0, 1);
		gridpoints[22 + 2*i + 1] = vec4(363.0- xOffset, (33.0 + (33.0 * i)), 0, 1);
	}

	// 将所有线赋成黑色
	for (int i = 0; i < 64; i++)
		gridcolours[i] = black;
	
	// 初始化棋盘格，并将没有被填充的格子设置成白色
	vec4 boardpoints[1200];
	for (int i = 0; i < 1200; i++)
		boardcolours[i] = white;

	// 对每个格子，初始化6个顶点，表示两个三角形，绘制一个正方形格子
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

	// 将棋盘格所有位置的填充与否都设置为false（没有被填充）
	for (int i = 0; i < 10; i++)
		for (int j = 0; j < 20; j++)
			board[i][j] = false;

	// 载入着色器 (此处修改成全局变量)
	program = InitShader("vshader.glsl", "fshader.glsl");
	glUseProgram(program);

	locxsize = glGetUniformLocation(program, "xsize");
	locysize = glGetUniformLocation(program, "ysize");

	GLuint vPosition = glGetAttribLocation(program, "vPosition");	// 获得顶点属性的入口的
	GLuint vColor = glGetAttribLocation(program, "vColor");			// 传递顶点数据	

	glGenVertexArrays(3, &vaoIDs[0]);	// 生成顶点数组
	
	// 棋盘格顶点
	glBindVertexArray(vaoIDs[0]);	// VAO 是一个对象，包含一或多个 VBO
	glGenBuffers(2, vboIDs);	// 生成新缓存对象
	checkGLError();

	// 棋盘格顶点位置
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[0]);	// 绑定缓存对象
	glBufferData(GL_ARRAY_BUFFER, 64*sizeof(vec4), gridpoints, GL_STATIC_DRAW);; // 将顶点数据拷贝到缓存对象中
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, 0); 		// 传递顶点数据
	glEnableVertexAttribArray(vPosition);								// 传递顶点数据
	
	// 棋盘格顶点颜色
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[1]);
	glBufferData(GL_ARRAY_BUFFER, 64*sizeof(vec4), gridcolours, GL_STATIC_DRAW);
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vColor);
	
	// 棋盘格每个格子	
	glBindVertexArray(vaoIDs[1]);
	glGenBuffers(2, &vboIDs[2]);
	checkGLError();

	// 棋盘格每个格子顶点位置
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[2]);
	glBufferData(GL_ARRAY_BUFFER, 1200*sizeof(vec4), boardpoints, GL_STATIC_DRAW);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vPosition);

	// 棋盘格每个格子顶点颜色
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[3]);
	glBufferData(GL_ARRAY_BUFFER, 1200*sizeof(vec4), boardcolours, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vColor);
	
	// 当前方块
	glBindVertexArray(vaoIDs[2]);
	glGenBuffers(2, &vboIDs[4]);
	checkGLError();

	// 当前方块顶点位置
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[4]);
	glBufferData(GL_ARRAY_BUFFER, 24*sizeof(vec4), NULL, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vPosition);

	// 当前方块顶点颜色
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[5]);
	glBufferData(GL_ARRAY_BUFFER, 24*sizeof(vec4), NULL, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vColor);

	// 下一个方块
	glBindVertexArray(next_vaoIDs);
	glGenBuffers(2, &next_vboIDs[0]);
	checkGLError();

	// 下一个方块顶点位置
	glBindBuffer(GL_ARRAY_BUFFER, next_vboIDs[0]);
	glBufferData(GL_ARRAY_BUFFER, 24 * sizeof(vec4), NULL, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vPosition);

	// 下一个方块顶点颜色
	glBindBuffer(GL_ARRAY_BUFFER, next_vboIDs[1]);
	glBufferData(GL_ARRAY_BUFFER, 24 * sizeof(vec4), NULL, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vColor);

	glBindVertexArray(0);
	glClearColor(0, 0, 0, 0);

	// 下一个方块的边框
	glGenBuffers(1, &borderVBO);
	glGenVertexArrays(1, &borderVAO);
	glBindVertexArray(borderVAO);
	glBindBuffer(GL_ARRAY_BUFFER, borderVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec4) * 8 * 2, NULL, GL_DYNAMIC_DRAW); // 8个顶点，每个顶点两个vec4（位置和颜色）
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vPosition);
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vColor);
	glBindVertexArray(0);

	// 游戏初始化
	int next_color = currentTetromino->getnext_color();
	int next_shape = currentTetromino->getnext_shape();
	// 删除旧对象(内存管理)
	if (currentTetromino != nullptr) {
		delete currentTetromino;
		currentTetromino = nullptr;
	}
	check_gameover();
	currentTetromino = new Tetromino(next_color, next_shape);

	gameover = false;	// 添加判断游戏结束的标志
	stop = false;	// 暂停置否
	score = 0;		// 初始化分数为0
	strcpy(score_out, "0");	// 初始化显示分数为0
	strcpy(gameover_tip, "");	// 初始化显示gameover的信息为空
	al_update_rank = false;		// 重新设置已更新排行榜标记

	// 从记录txt中读取分数排行榜
	ifstream in("score_rank.txt");
	string s;
	int i = 0;
	while (getline(in, s)) {
		//着行读取数据并存于s中，直至数据全部读取
		score_rank[i++] = stoi(s);
		if (i == 3)	break;
	}
	// glut的显示信息
	glutSwapBuffers();

	starttime = glutGet(GLUT_ELAPSED_TIME);
}

//////////////////////////////////////////////////////////////////////////
// 检查在cellpos位置的格子是否被填充或者是否在棋盘格的边界范围内。

// 此处添加一个条件使方块可以向上叠加
bool checkvalid(vec2 cellpos)
{
	// 此处添加一个条件使方块可以向上叠加
	if ((cellpos.x >= 0) && (cellpos.x < 10) && (cellpos.y >= 0) && (cellpos.y < 20)
		&& (board[(int)cellpos.x][(int)cellpos.y] == false))
		return true;
	else
		return false;
}

//////////////////////////////////////////////////////////////////////////
// 检查棋盘格在row行有没有被填充满

// 此处自底向上遍历每一行，检查每一行的方格的标记情况
// 考虑存在多行同时被消除的情况
// 此处写成递归的形式比较简单
void checkfullrow(int row)
{
	// 如果最后一行就直接返回
	if (row == 19)return;
	// 自底向上检查每一行
	for (int i = 0; i < 10; i++) {
		if (board[i][row] == false) {
			// 检查上一行
			checkfullrow(row + 1);
			return;
		}
	}
	// 计算积分的标记++
	clean_up_row++;

	// 满足被clean的情况，进行消除
	// 消除就是自底向上依次把上一行的颜色和标记信息赋值给下一行
	for (int i = row; i < 19; i++) {
		for (int j = 0; j < 10; j++) {
			vec4 color = boardcolours[(int)(6 * (10 * (i + 1) + j))];
			changecellcolour(vec2(j, i), color);
			board[j][i] = board[j][i + 1];
		}
	}
	// 此时的当前行row就成为未被检查的那一行，于是继续递归
	checkfullrow(row);
}

//////////////////////////////////////////////////////////////////////////
// 重新启动游戏

void restart()
{
	// 检查是否更新排行榜，如果没有便更新
	if (!al_update_rank) {
		update_rank();
		al_update_rank = true;
	}
	init();	// 重新初始化即可
}

// 判断游戏是否结束
void check_gameover() {
	// 当棋盘顶格的位置被方块占着，游戏便结束
	if (board[5][19] || board[6][19]) {
		std::cout << "GAME OVER, finally score: " << score << endl;
		gameover = true;
		strcpy(gameover_tip, "gameover");
		glutSwapBuffers();
		// 如果未更新排行榜，便更新排行榜
		if (!al_update_rank) {
			update_rank();
			al_update_rank = true;
		}
		return;
	}
}

//////////////////////////////////////////////////////////////////////////
// 游戏渲染部分

// 此函数用于在屏幕上输出文字信息
void showText(float x, float y, const char* str, float rcolor, float gcolor, float bcolor) {
	glUseProgram(0);
	glColor3f(rcolor, gcolor, bcolor);	// 设置颜色
	glRasterPos2f(x, y);			// 用于显示[字体]时设置字符的起始位置
	int strLen = strlen(str);
	for (int i = 0; i<strLen; i++) {
		// 显示字符，字体-> GLUT_BITMAP_TIMES_ROMAN，字体大小-> 24
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, str[i]);
	}
	glFlush();
	glUseProgram(program);
}

// 此函数用于在屏幕上显示分数排行榜
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
	glDrawArrays(GL_TRIANGLES, 0, 1200); // 绘制棋盘格 (10*20*2 = 400 个三角形)

	glBindVertexArray(vaoIDs[2]);
	glDrawArrays(GL_TRIANGLES, 0, 24);	 // 绘制当前方块 (8 个三角形)

	glBindVertexArray(next_vaoIDs);
	glDrawArrays(GL_TRIANGLES, 0, 24);	 // 绘制下一个方块 (8 个三角形)

	glBindVertexArray(borderVAO);
	glDrawArrays(GL_LINES, 0, 8); // 绘制边框
	glBindVertexArray(0);

	glBindVertexArray(vaoIDs[0]);
	glDrawArrays(GL_LINES, 0, 64);		 // 绘制棋盘格的线

	// 此处showText把分数等文字信息显示在界面上
	showText(0.40, 0.75, score_tip, 1.0, 1.0, 1.0);
	showText(0.55, 0.65, score_out, 0.0, 1.0, 1.0);
	showText(0.40, 0.55, gameover_tip, 0.0, 1.0, 1.0);
	showText(0.40, 0.2, next_tip, 1.0, 1.0, 1.0);
	showText(-0.95, -0.97, keyboard_tip, 1.0, 1.0, 1.0);

	// 显示分数排行榜
	showRank();

	glutSwapBuffers();  // 每次的所有绘图操作都在后台缓冲中进行， 当绘制完成时， 
						//把绘制的最终结果复制到屏幕上， 这样， 我们看到所有GDI元素同时出现在屏幕上
}

//////////////////////////////////////////////////////////////////////////
// 当前方块移动或者旋转时，更新VBO
void updatetile(vec2 tilepos, vec2 *tile)
{
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[4]);

	// 每个方块包含四个格子
	for (int i = 0; i < 4; i++)
	{
		// 计算格子的坐标值
		GLfloat x = tilepos.x + tile[i].x - xOffset / 33.0;
		GLfloat y = tilepos.y + tile[i].y;

		vec4 p1 = vec4(33.0 + (x * 33.0), 33.0 + (y * 33.0), .4, 1);
		vec4 p2 = vec4(33.0 + (x * 33.0), 66.0 + (y * 33.0), .4, 1);
		vec4 p3 = vec4(66.0 + (x * 33.0), 33.0 + (y * 33.0), .4, 1);
		vec4 p4 = vec4(66.0 + (x * 33.0), 66.0 + (y * 33.0), .4, 1);

		// 每个格子包含两个三角形，所以有6个顶点坐标
		vec4 newpoints[6] = { p1, p2, p3, p2, p3, p4 };
		glBufferSubData(GL_ARRAY_BUFFER, i * 6 * sizeof(vec4), 6 * sizeof(vec4), newpoints);
	}
	glBindVertexArray(0);
}

// 更新下一个方块
void update_next_tile(vec2 next_tilepos, vec2 *next_tile) {

	glBindBuffer(GL_ARRAY_BUFFER, next_vboIDs[0]);

	// 每个方块包含四个格子
	for (int i = 0; i < 4; i++)
	{

		// 计算格子的坐标值
		GLfloat x = next_tilepos.x + next_tile[i].x;
		GLfloat y = next_tilepos.y + next_tile[i].y;

		vec4 p1 = vec4(33.0 + (x * 33.0), 33.0 + (y * 33.0), .4, 1);
		vec4 p2 = vec4(33.0 + (x * 33.0), 66.0 + (y * 33.0), .4, 1);
		vec4 p3 = vec4(66.0 + (x * 33.0), 33.0 + (y * 33.0), .4, 1);
		vec4 p4 = vec4(66.0 + (x * 33.0), 66.0 + (y * 33.0), .4, 1);

		// 每个格子包含两个三角形，所以有6个顶点坐标
		vec4 newpoints[6] = { p1, p2, p3, p2, p3, p4 };
		glBufferSubData(GL_ARRAY_BUFFER, i * 6 * sizeof(vec4), 6 * sizeof(vec4), newpoints);
	}
	glBindVertexArray(0);

	// 绘制边框
	for (int i = 0; i < 4; i++) {
		// 计算每个小方块的边界
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

// 为当前方块和下一方块着色
void draw_color_to_tile(int color, int next_color) {
	// 给新方块赋上颜色
	vec4 newcolours[24];
	for (int i = 0; i < 24; i++)
		// 这里的下标random_color控制着随机颜色
		newcolours[i] = all_color[color];

	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[5]);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(newcolours), newcolours);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	// 给下一个新方块赋上颜色
	for (int i = 0; i < 24; i++)
		// 这里的下标random_color控制着随机颜色
		newcolours[i] = all_color[next_color];

	glBindBuffer(GL_ARRAY_BUFFER, next_vboIDs[1]);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(newcolours), newcolours);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

//////////////////////////////////////////////////////////////////////////
// 在窗口被拉伸的时候，控制棋盘格的大小，使之保持固定的比例。

void reshape(GLsizei w, GLsizei h)
{
	xsize = w;
	ysize = h;
	glViewport(0, 0, w, h);
}


//////////////////////////////////////////////////////////////////////////
// 键盘响应时间中的普通按键响应

void keyboard(unsigned char key, int x, int y)
{
	switch(key) 
	{
		case 033: // ESC键 和 'q' 键退出游戏（退出前更新榜单）
		case 'q':
			// 如果未更新排行榜，便更新排行榜
			if (!al_update_rank) {
				update_rank();
				al_update_rank = true;
			}
			// 删除对象并退出
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
		case 'r': // 'r' 键重启游戏
			restart();
			break;
	}
	glutPostRedisplay();
}

void idle(void)
{
	glutPostRedisplay();
}

// 释放OpenGL资源的函数
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
// GL错误检查
void checkGLError() {
	GLenum err;
	while ((err = glGetError()) != GL_NO_ERROR) {
		std::cerr << "OpenGL error: " << gluErrorString(err) << std::endl;
	}
}

//////////////////////////////////////////////////////////////////////////
// 此处用于opengl与类中函数通信，保证数据的封装
// 包装处理特殊按键响应函数
void specialKeyWrapper(int key, int x, int y) {
	if (currentTetromino != nullptr) {
		currentTetromino->handleSpecialKey(key, x, y);
	}
}
// 包装时间处理
void TimeFuncWrapper(int val) {
	if (currentTetromino != nullptr) {
		currentTetromino->TimeFunc(val);
	}
	// 如果分数大于100的话，增加方块下落速度
	// 以此提升游戏难度
	if (score >= 100) {
		glutTimerFunc(300, TimeFuncWrapper, 1);
	}
	else {
		// 在结尾重复调用计时器函数
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

		srand(time(NULL));	// 设置随机数种子
		// 产生0-6的随机数控制随机的形状
		int shape = rand() % 7;
		// 产生0-5的随机数控制随机的颜色
		int color = rand() % 6;
		currentTetromino = new Tetromino(shape, color); // 实例化对象并赋值给全局指针
		init();

		glutDisplayFunc(display);	// display
		glutReshapeFunc(reshape);	// 窗口大小变化
		glutSpecialFunc(specialKeyWrapper);	// 响应方向键盘
		glutKeyboardFunc(keyboard);	// 响应r和q
		glutIdleFunc(idle);			// 设置全局的回调函数

		glutTimerFunc(500, TimeFuncWrapper, 1);// 此处增加了调用计时器函数

		glutMainLoop();			// 进入GLUT事件处理循环
	}
	catch (const std::exception& e) {
		std::cerr << "Exception caught: " << e.what() << std::endl;
		logFile << "Exception caught: " << e.what() << std::endl;
		// 可能的清理工作
		cleanup();
		exit(EXIT_FAILURE);
	}

	// 在退出前清理资源(openGL资源)
	cleanup();

	// 在退出前关闭日志文件
	logFile << "Program finished" << std::endl;
	logFile.close();

	return 0;
}