/* exercise07.c  */
/* compile: cc -o exercise07 exercise07.c -g -O2 -Wall -lglfw -lGLU -lGL -lX11 -lXrandr -lm */

#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <unistd.h>
#include <GL/glfw.h>

#define MAX_BALLS 100

typedef struct Ball{
	long double x, y;   /* ボールの中心位置 */
	long double vx, vy;     /* ボールの移動速度 */
	long double r;               /* ボールの半径 */
}ball;

/* 円を描画する */
void circle(double x, double y, double r)
{
	int const N = 24;             /* 円周を 24分割して線分で描画することにする */
	int i;
	
	glBegin(GL_LINE_LOOP);
	for (i = 0; i < N; i++)
		glVertex2d(x + cos(2 * M_PI * i / N) * r, y + sin(2 * M_PI * i / N) * r);
	glEnd();
}

void drawTheBall(ball *Ball) {
	glColor3d(1.0, 1.0, 1.0);
	circle(Ball->x, Ball->y, Ball->r); 
}

// 新しいボールを生成する関数
ball makeNewBall(double x, double y, double vx, double vy, double r) {
    ball newBall;
    newBall.x = x;
    newBall.y = y;
    newBall.vx = vx;
    newBall.vy = vy;
    newBall.r = r;
    return newBall;
}


// 矢印キーが押された時にボールを移動する関数
void moveBall(ball *Ball, int width, int height, double dt) {
    if (glfwGetKey(GLFW_KEY_LEFT) == GLFW_PRESS) {
        Ball->x -= 10.0; // ボールを左に10px移動
    }
    if (glfwGetKey(GLFW_KEY_RIGHT) == GLFW_PRESS) {
        Ball->x += 10.0; // ボールを右に10px移動
    }

    // 画面外に出ないようにする
    if (Ball->x - Ball->r < 0) {
        Ball->x = Ball->r; // 左端の境界内に調整
    }
    else if (Ball->x + Ball->r > width) {
        Ball->x = width - Ball->r; // 右端の境界内に調整
    }
}


// 衝突を判定し、ボール同士がぶつかった場合は新しいボールを作成して元のボールを削除する関数
void checkCollision(ball *ball1, ball *ball2, ball *displayBall, int *ballIndex) {
    double distance = sqrt(pow(ball1->x - ball2->x, 2) + pow(ball1->y - ball2->y, 2));

    if (distance < ball1->r + ball2->r) {
        // ボール同士がぶつかった場合の処理
        printf("Collision detected! Deleting balls and creating a new one.\n");

        // 新しいボールの座標は元の2つのボールの真ん中の位置
        double newX = (ball1->x + ball2->x) / 2.0;
        double newY = (ball1->y + ball2->y) / 2.0;

        // ボールを削除して新しいボールを作成
        for (int i = 0; i <= *ballIndex; i++) {
            if (&displayBall[i] == ball1 || &displayBall[i] == ball2) {
                // 削除するボールは無効な値で初期化
                displayBall[i].r = -1.0;
            }
        }

        // 新しいボールの作成
        (*ballIndex)++;
        if (*ballIndex < MAX_BALLS) {  // 配列の範囲を超えないようにする
            displayBall[*ballIndex] = makeNewBall(newX, newY, 0.0, 0.0, 20.0);
        }
    }
}

int main(void)
{
	ball displayBall[MAX_BALLS]; //画面に表示されているボールを管理する配列
	int ballIndex = 0; //displayBallの現在のインデックス
	int width = 400, height = 640; /* ウィンドウのサイズ */
	double g = 9.8; /* 重力加速度 */
	double dt = 0.2; /* 時間刻み */
	bool spaceKeyPressed = false; 
	double groundLevel = 0.0; // グラウンドの高さを設定

	
	displayBall[ballIndex].x = width/2.0; displayBall[ballIndex].y = height/1.5; displayBall[ballIndex].vx = 0.0; displayBall[ballIndex].vy = 0.0; displayBall[ballIndex].r = 10.0;
	
	/* グラフィック環境を初期化して、ウィンドウを開く */
	glfwInit();
	glfwOpenWindow(width, height, 0, 0, 0, 0, 0, 0, GLFW_WINDOW);
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();             /* 投影以外の座標変換は行わない */
	
	for (;;) {
		/* Esc が押されるかウィンドウが閉じられたらおしまい */
		if (glfwGetKey(GLFW_KEY_ESC) || !glfwGetWindowParam(GLFW_OPENED))
			break;
		
		glfwGetWindowSize(&width, &height); /* 現在のウィンドウサイズを取得する */
		glViewport(0, 0, width, height); /* ウィンドウ全面をビューポートにする */
		
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(0, width, 0, height, -1.0, 1.0); /* この範囲の空間をビューポートに投影する */
		
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT); /* バックバッファを黒で塗り潰す */
		
		// スペースキーが押されているかどうかを判定する
        if (glfwGetKey(GLFW_KEY_SPACE) == GLFW_PRESS && !spaceKeyPressed) {
            spaceKeyPressed = true; // スペースキーが押されたらフラグをtrueにする
            displayBall[ballIndex].vy = 0.0; // 縦方向のスピードを初期化する

        }
        
        // ボールが地面に触れているか判定
        if (spaceKeyPressed){
		    if (displayBall[ballIndex].y - displayBall[ballIndex].r <= groundLevel) {
		        displayBall[ballIndex].y = groundLevel + displayBall[ballIndex].r; // ボールの高さを地面の高さにする
		        displayBall[ballIndex].vy = 0.0; // 縦方向の速度を0にする
		        spaceKeyPressed = false;
		        ballIndex++; //つぎのボールを生成する
		        displayBall[ballIndex] = makeNewBall(width / 2.0, height / 1.5, 0.0, 0.0, 10.0);
		    }
        }

        // スペースキーが押されていたらボールの位置を更新する
        if (spaceKeyPressed) {
            displayBall[ballIndex].vy += g * dt; // 重力加速度を用いて縦方向の速度を更新する
            displayBall[ballIndex].y -= displayBall[ballIndex].vy * dt; // 縦の位置を更新する
        }
        
        // 横方向にも飛び出さないようにする
        if (displayBall[ballIndex].x - displayBall[ballIndex].r < 0) {
            displayBall[ballIndex].x = displayBall[ballIndex].r; // 左側から飛び出さないようにする
        }
        else if (displayBall[ballIndex].x + displayBall[ballIndex].r > width) {
            displayBall[ballIndex].x = width - displayBall[ballIndex].r; // 右側から飛び出さないようにする
        }
        
        //ボールが落下し始める前に矢印キーが押されたら左右に移動する
        if (!spaceKeyPressed) {
            moveBall(&displayBall[ballIndex], width, height, dt);
        }
        
        // ボール同士の衝突をチェック
		for (int i = 0; i <= ballIndex; i++) {
			if (i != ballIndex && displayBall[i].r > 0.0) {  // 無効なボールは衝突判定を行わない
				checkCollision(&displayBall[i], &displayBall[ballIndex], displayBall, &ballIndex);
			}
		}
		
		for (int i=0; i<=ballIndex; i++){ //displayBallに保存されているボールの数だけ描画する
			drawTheBall(&displayBall[i]);           
		}
			
		glfwSwapBuffers();          /* フロントバッファとバックバッファを入れ替える */
		usleep(40 * 1000);          /* 40ミリ秒くらい時間待ち */
	}
	
	glfwTerminate();              /* ウィンドウを閉じる */
	
	return 0;
}
