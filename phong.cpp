#include <unistd.h>
#include <GL/glut.h>
#include <deque>
#include <ctime>
#include <iostream>
#include <string>
#include <iostream>
#include <sstream>
#include <math.h>
#include "GL/freeglut.h"
#pragma comment(lib, "OpenGL32.lib")
//#include <SFML/Audio.hpp>

//#include <include/irrKlang.h>
//using namespace irrklang;

//ISoundEngine *SoundEngine = createIrrKlangDevice();

// window size and update rate (60 fps)
GLint width = 500;
GLint height = 500;
GLint interval = 1000 / 60;

// score
GLint score_left = 0;
GLint score_right = 0;
GLint point = -1;

// rackets in general
GLint racket_width = 10;
GLint racket_height = 80;
GLint racket_speed = 6;

// left racket position
GLfloat racket_left_x = 10.0f;
GLfloat racket_left_y = height/2;

// right racket position
GLfloat racket_right_x = width - racket_width - 10;
GLfloat racket_right_y = height/2;

// ball
GLfloat ball_pos_x = width / 2;
GLfloat ball_pos_y = height / 2;
GLfloat ball_dir_x = -1.0f;
GLfloat ball_dir_y = 0.0f;
GLfloat ball_rot_x = 0.0f;
GLfloat ball_rot_y = 0.0f;
GLint ball_size = 20;
GLfloat ball_speed = 2.0f;
GLfloat ball_rot_speed = 10.0f;
GLfloat t = 0;//angulacao da bola

//sistema
GLboolean paused = false;
GLboolean gameOver = false;

// controls
GLboolean but_up = false;
GLboolean but_down = false;
GLint theta = 180;
bool keys[256] = { false };

void drawRect(float x, float y, float width, float height) {
    glBegin(GL_QUADS);
        glVertex2f(x, y);
        glVertex2f(x + width, y);
        glVertex2f(x + width, y + height);
        glVertex2f(x, y + height);
    glEnd();
}

void drawLine(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2){
    glPointSize(3.0);
    for(int i = 0; i <= y2; i+=60){
        glBegin(GL_LINES);
            glVertex2f(x1, i);
            glVertex2f(x2, i+30);
        glEnd();
    }
    glFlush();
}


void rotacao2d(GLfloat x, GLfloat y, GLfloat xct, GLfloat yct){
    //vertice v_rotacao;
    
    double s = sin(theta*3.14/180.0);
    double c = cos(theta*3.14/180.0);
    
    //variáveis de reposicionamento
    double repx = xct - xct*c + yct*s;
    double repy = yct - yct*c - xct*s;
    

    double matrix_rotacao[3][3] = {
        c, -s, repx,
        s, c, repy,
        0, 0, 1
    };

    float coord_homogeneas[3] = {x, y , 1};
    float coord_finais[3] = {0,0,0};
    for(int i = 0; i < 3; i++){
        for(int j = 0; j < 3; j++){
            coord_finais[i] += matrix_rotacao[i][j]*coord_homogeneas[j];
        }
    }
    ball_rot_x = coord_finais[0];
    ball_rot_y = coord_finais[1];
}


void drawCircle(GLfloat x1, GLfloat x, GLfloat y1, GLfloat y){
    GLfloat radius;
    radius = (sqrt(pow(x,2) + pow(y,2)));
    int trianguleAmount = 100;
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(x1,y1);
    for(int i = 0; i<=trianguleAmount; i++){
        
        rotacao2d(x1+(radius*cos(i*2*3.14/trianguleAmount)),
        y1+(radius*sin(i*2*3.14/trianguleAmount)), x1, y1);
        glVertex2f(ball_rot_x, ball_rot_y);
        if(i <= trianguleAmount/4 || i <= trianguleAmount*3/4 && i >= trianguleAmount/2)
            glColor3f(1.0f, 0.0f, 0.0f);
        else    
            glColor3f(0.0f, 0.0f, 1.0f);
    }
    glEnd();
}

std::string int2str(int x) {
    // converts int to string
    std::stringstream ss;
    ss << x;
    return ss.str( );
}

void drawText(float x, float y, std::string text) {
    glRasterPos2f(x, y);
    glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, (const unsigned char*)text.c_str());
}

void draw() {
    // clear (has to be done at the beginning)
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    // draw rackets
    drawRect(racket_left_x, racket_left_y, racket_width, racket_height);
    drawRect(racket_right_x, racket_right_y, racket_width, racket_height);

    // draw ball
    drawCircle(ball_pos_x, ball_size / 2, ball_pos_y, ball_size / 2);

    // desenha linha do meio
    drawLine(width/2, 0, width/2, height);

    // draw score
    drawText(width / 4, height - 30, int2str(score_left));
    drawText(width* 3 / 4, height - 30, int2str(score_right));
    if(gameOver){
        if(score_left > 2)
            drawText(width/2 - 200, height/2, "Jogador 1 venceu");
        else
            drawText(width/2 + 25, height/2, "Jogador 2 venceu");
        paused = true;
    }

    // swap buffers (has to be done at the end)
    glutSwapBuffers();
}

void enable2D(int width, int height) {
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0f, width, 0.0f, height, 0.0f, 1.0f);
    glMatrixMode (GL_MODELVIEW);
    glLoadIdentity();
}

void vec2_norm(float& x, float &y) {
    // sets a vectors length to 1 (which means that x + y == 1)
    float length = sqrt((x * x) + (y * y));
    if (length != 0.0f) {
        length = 1.0f / length;
        x *= length;
        y *= length;
    }
}

void updateBarras(){
    // barra direita bate no topo
    if (racket_right_y + racket_height >= height) {
        racket_right_y -= racket_speed;
    }

    // barra direita bate no fundo
    if (racket_right_y <= 0) {
        racket_right_y += racket_speed;
    }

    // barra esquerda bate no topo
    if (racket_left_y + racket_height >= height) {
        racket_left_y -= racket_speed;
    }

    // barra esquerda bate no fundo
    if (racket_left_y <= 0) {
        racket_left_y += racket_speed;
    }
    
    // left racket
    if (keys['w']){
        racket_left_y += racket_speed;
    } 
    if (keys['s']){
        racket_left_y -= racket_speed;
    }

    // right racket
    if (but_up){
        racket_right_y += racket_speed;
    }
        
    if (but_down){
        racket_right_y -= racket_speed;
    } 
}

void updateBall() {
    // fly a bit
    ball_pos_x += ball_dir_x * ball_speed;
    ball_pos_y += ball_dir_y * ball_speed;
    
    // rotate a bit
    if(ball_dir_x > 0){
        theta-=ball_rot_speed;
    }else{
        theta+=ball_rot_speed;
    }
   
    // Colisao com a barra esquerda
    if (ball_pos_x - ball_size/2 < racket_left_x + racket_width &&
        ball_pos_x - ball_size/2 > racket_left_x &&
        ball_pos_y - ball_size/2 < racket_left_y + racket_height &&
        ball_pos_y + ball_size/2 > racket_left_y) {
        // set fly direction depending on where it hit the racket
        // (t is 0.5 if hit at top, 0 at center, -0.5 at bottom)
        t = ((ball_pos_y - racket_left_y) / racket_height) - 0.5f;
        ball_dir_x = fabs(ball_dir_x); // force it to be positive
        ball_dir_y = t;
        ball_speed += 0.2;
        ball_rot_speed += 2;
        //SoundEngine->play2D("audio/solid.wav");
    }

    // Colisao com a barra direita
    if (ball_pos_x + ball_size/2 > racket_right_x &&
        ball_pos_x + ball_size/2 < racket_right_x + racket_width &&
        ball_pos_y - ball_size/2 < racket_right_y + racket_height &&
        ball_pos_y + ball_size/2 > racket_right_y) {
        // set fly direction depending on where it hit the racket
        // (t is 0.5 if hit at top, 0 at center, -0.5 at bottom)
        t = ((ball_pos_y - racket_right_y) / racket_height) - 0.5f;
        ball_dir_x = -fabs(ball_dir_x); // force it to be negative
        ball_dir_y = t;
        ball_speed += 0.2;
        ball_rot_speed += 2;
        //SoundEngine->play2D("audio/solid.wav");
    }

    // colisao na esquerda
    if (ball_pos_x <= 0) {
        ++score_right;
        ball_dir_x = fabs(ball_dir_x); // force it to be positive
        ball_speed = 0;
        ball_rot_speed = 0;
        point = 1;
    }

    // colisao na direita
    if (ball_pos_x >= width) {
        ++score_left;
        ball_dir_x = -fabs(ball_dir_x); // force it to be negative
        ball_speed = 0;
        ball_rot_speed = 0;
        point = 2;
    }

    if(ball_speed == 0 && point == 1){
        ball_pos_x = racket_right_x - ball_size/2;
        ball_pos_y = racket_right_y + racket_height/2;
    }

    if(ball_speed == 0 && point == 2){
        ball_pos_x = racket_left_x + racket_width + ball_size/2;
        ball_pos_y = racket_left_y + racket_height/2;
    }

    if(point == 0){
        ball_speed = 2;
        ball_rot_speed = 10;
        point = -1;
    }

    // hit top wall?
    if (ball_pos_y + ball_size/2 > height) {
        ball_dir_y = -fabs(ball_dir_y); // force it to be negative
        t-=1;
    }

    // hit bottom wall?
    if (ball_pos_y - ball_size/2 < 0) {
        ball_dir_y = fabs(ball_dir_y); // force it to be positive
        t+=1;
    }

    // make sure that length of dir stays at 1
    vec2_norm(ball_dir_x, ball_dir_y);
}

void normalKeyboard(unsigned char key, int x, int y) {

    switch (key) {
        case 13:// enter
            if(point != -1)
                point = 0;

        break;
        case 27: // Tecla Esc
        exit(0);
        
        break;
        case 32: // escpaco
            if(gameOver){
                gameOver = false;
                score_left = 0;
                score_right = 0;
                point = 0;
                ball_pos_x = width/2;
                ball_pos_y = height/2;
            }
                
            if(paused)
                paused = false;
            else
                paused = true;
            
        break;
    }
    
}

void update(int value) {
    if(score_left <= 2 && score_right <= 2){
        if(!paused){
        // update bola
        updateBall();

        // ubdate barra
        updateBarras();
        }
    }else{
        gameOver = true;    
    }

    if(keys[13]){// enter
        if(point != -1)
            point = 0;
    }

    if(keys[27]){// Tecla Esc
        exit(0);
    }

    // Call update() again in 'interval' milliseconds
    glutTimerFunc(interval, update, 0);            

    // Redisplay frame
    glutPostRedisplay();

}

void keySPressed(int key, int x, int y) {
    switch (key) {
        case GLUT_KEY_UP:
            but_up = true;
        
        break;
        case GLUT_KEY_DOWN:
            but_down = true;

        break;
        
    }
}

void keySUp(int key, int x, int y) {
    switch (key) {
        case GLUT_KEY_UP:
            but_up = false;
        
        break;
        case GLUT_KEY_DOWN:
            but_down = false;

        break;
        
    }
}

void keyPressed(unsigned char key, int x, int y) {
    keys[key] = true;
    if(key == 32){
        // tecla espaco
        if(gameOver){
            gameOver = false;
            score_left = 0;
            score_right = 0;
            point = 0;
            ball_pos_x = width/2;
            ball_pos_y = height/2;
        }
        if(paused)
            paused = false;
        else
            paused = true;
    }
}

void keyUp(unsigned char key, int x, int y) {
    keys[key] = false;
}

void idle(){
    glutPostRedisplay();
}
// program entry point
int main(int argc, char** argv) {

    // initialize opengl (via glut)
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(width, height);
    glutCreateWindow("PHONG");

    // Register callback functions  
    glutKeyboardFunc(normalKeyboard);
    glutSpecialFunc(keySPressed);
    glutSpecialUpFunc(keySUp);
    glutKeyboardFunc(keyPressed);
    glutKeyboardUpFunc(keyUp);
    glutDisplayFunc(draw);
    glutTimerFunc(interval, update, 0);

    // setup scene to 2d mode and set draw color to white
    enable2D(width, height);
    glColor3f(1.0f, 1.0f, 1.0f);
    //glutIdleFunc(idle);
    // start the whole thing
    glutMainLoop();
    
    return 0;
}
