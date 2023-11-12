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

void drawBox(double width, double height, double box_side, double box_height, double groundLevel) {
	glColor3d(1.0, 1.0, 1.0);
	glBegin(GL_LINE_STRIP); //閉じない多角形?を描画
	glVertex2d(box_side, box_height);
	glVertex2d(box_side, groundLevel);
	glVertex2d(width-box_side, groundLevel);
	glVertex2d(width-box_side, box_height);
	glEnd();
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

void rearrangeBalls(ball *displayBall, int *ballIndex, int collisionIndex1, int collisionIndex2){
	//ボール1とボール2を削除(無効な値で初期化)
	displayBall[collisionIndex1].r = -1.0;
	displayBall[collisionIndex2].r = -1.0;
	
	//ボール1より後ろのボールを1つ前にずらす
	for (int i = collisionIndex1; i < *ballIndex - 1; i++) {
		displayBall[i] = displayBall[i + 1];
	}
	
	//ボール2より後ろのボールを2つ前にずらす
	for (int i = collisionIndex2 - 1; i < *ballIndex - 2; i++) {
		displayBall[i] = displayBall[i + 2];
	}
	
	//新しいボールを最後に追加
	(*ballIndex)++;
	if (*ballIndex < MAX_BALLS) {
		// 新しいボールの座標は元の2つのボールの真ん中の位置
        double newX = (displayBall[collisionIndex1].x + displayBall[collisionIndex2].x) / 2.0;
        double newY = (displayBall[collisionIndex1].y + displayBall[collisionIndex2].y) / 2.0;
        
        displayBall[*ballIndex - 1] = makeNewBall(newX, newY, 0.0, 0.0, 20.0);
	}
}

// 衝突を判定し、ボール同士がぶつかった場合は新しいボールを作成して元のボールを削除する関数
void checkCollision(ball *displayBall, int *ballIndex) {
    for (int i = 0; i <= *ballIndex - 1; i++) {
    	for (int j = i + 1; j <= *ballIndex; j++) {
    		double distance = sqrt(pow(displayBall[i].x - displayBall[j].x, 2) + pow(displayBall[i].y - displayBall[j].y, 2));
    		
    		if (distance < displayBall[i].r + displayBall[j].r) {
    			rearrangeBalls(displayBall, ballIndex, i, j);
    		}
    	}
    }
}

void checkContainerCollision(ball *displayBall, double groundLevel, double width, double wallDistance, int *ballIndex) {
    for (int i = 0; i < *ballIndex; i++) {
        if (displayBall[i].r > 0.0) {  // 有効なボールのみ処理
            // 地面との接触判定
            if (displayBall[i].y - displayBall[i].r <= groundLevel) {
                // ボールが地面にぶつかった場合の処理
                // 反発係数 0.1 で反対方向に反射させる
                displayBall[i].vy = -0.1 * displayBall[i].vy;
            }

            // 左の壁との接触判定
            if (displayBall[i].x - displayBall[i].r <= wallDistance) {
                // ボールが左の壁にぶつかった場合の処理
                // 反発係数 0.1 で反対方向に反射させる
                displayBall[i].vx = -0.1 * displayBall[i].vx;
            }

            // 右の壁との接触判定
            if (displayBall[i].x + displayBall[i].r >= width-wallDistance) {
                // ボールが右の壁にぶつかった場合の処理
                // 反発係数 0.1 で反対方向に反射させる
                displayBall[i].vx = -0.1 * displayBall[i].vx;
            }
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
	double box_side = 30.0; //箱と壁からの距離を設定
	double box_height = 400.0; // 箱の高さを設定
	double groundLevel = 30.0; // グラウンドの高さを設定
	
	box_height += groundLevel; // グラウンドの高さを考慮して箱の高さを更新

	
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
		
		drawBox((double)width, (double)height, box_side, box_height, groundLevel);
		
		// スペースキーが押されているかどうかを判定する
        if (glfwGetKey(GLFW_KEY_SPACE) == GLFW_PRESS && !spaceKeyPressed) {
            spaceKeyPressed = true; // スペースキーが押されたらフラグをtrueにする
            displayBall[ballIndex].vy = 0.0; // 縦方向のスピードを初期化する

        }
        
        checkContainerCollision(displayBall, groundLevel, (double)width, box_side, &ballIndex);
        
        // ボールが地面に触れているか判定
        if (spaceKeyPressed){
		    if (displayBall[ballIndex].y - displayBall[ballIndex].r <= groundLevel) {
		        displayBall[ballIndex].y = groundLevel + displayBall[ballIndex].r; // ボールの高さを地面の高さにする
		        
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
		checkCollision(displayBall, &ballIndex);
		
		for (int i=0; i<=ballIndex; i++){ //displayBallに保存されているボールの数だけ描画する
			drawTheBall(&displayBall[i]);           
		}
			
		glfwSwapBuffers();          /* フロントバッファとバックバッファを入れ替える */
		usleep(40 * 1000);          /* 40ミリ秒くらい時間待ち */
	}
	
	glfwTerminate();              /* ウィンドウを閉じる */
	
	return 0;
}
