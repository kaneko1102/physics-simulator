//
//  Created by 金子卓矢 on 2019/08/04.
//  Copyright © 2019 金子卓矢. All rights reserved.
//

//参考ページ
//https://seesaawiki.jp/w/mikk_ni3_92/d/%BC%AB%CD%B3%CD%EE%B2%BC
//http://marupeke296.com/COL_MV_No2_SpherePlaneColliTimePos.html

#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#include <iostream>
#include <cstdio>
#include <GLUT/GLUT.h>

using namespace std;

void display();
void reshape(int w, int h);
void timer(int value);
void keyboard(unsigned char key, int x, int y);
double dot(double x[],double y[]);
int collision(double x[],double y[]);
void runge_kutta_y(double t,double y_func[]);
double func1y(double t,double v,double y);
double func2y(double t,double v,double y);
void runge_kutta_x(double t,double x_func[]);
double func1x(double t,double v,double x);
double func2x(double t,double v,double x);
void runge_kutta_z(double t,double z_func[]);
double func1z(double t,double v,double z);
double func2z(double t,double v,double z);
void DRAW_FLOOR();

//初期条件 t=0の時
//位置
#define START_X -20 //x軸
#define START_Y 0 //y軸
#define START_Z 10 //z軸
//速度
#define START_VX 5 //x方向
#define START_VZ 0 //y方向
#define START_VY 30 //z方向


#define RADIUS 2 //球の半径
#define FIELD (RADIUS * 100) //床の大きさ
#define GROUND RADIUS //床の位置

#define g 9.8 //重力加速度
#define dt 0.1 //tの刻み幅
#define k 0.24 //空気抵抗の比例定数
#define m 10 //物体の質量
#define e 0.9 //はね返り係数

inline void GLUT_INIT()
{
    glutInitDisplayMode(GLUT_RGBA| GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(300,300);
}

inline void GLUT_CALL_FUNC()
{
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutTimerFunc(1,timer,0);
}

GLfloat light_pos[] = { FIELD, FIELD, -FIELD, 1.0 };

inline void MY_INIT()
{
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
}

int main(int argc, char **argv)
{
    glutInit(&argc,argv);
    GLUT_INIT();
    glutCreateWindow("CG final");
    GLUT_CALL_FUNC();
    MY_INIT();
    glutMainLoop();
    
    return 0;
}

GLfloat floor_Color[] = { 0.9, 0.9, 0.9, 1.0 }; //床の色
GLfloat Obj_Color[] = { 0.0, 1.0, 0.0, 1.0 }; //物体の色
float t;
int flag_y=0;

//x,y,z方向の速度、位置
double func_x[]={START_VX,START_X};
double func_y[]={START_VY,START_Y};
double func_z[]={START_VZ,START_Z};

void display()
{
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    
    gluLookAt(100.0, 100.0, 100.0, 0.0, 8.0, 0.0, 0.0, 1.0, 0.0);
    glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
    
    glEnable(GL_DEPTH_TEST);
    
    /* 床の描画 */
    DRAW_FLOOR();
    
    double vect_x[]={func_x[1],func_y[1]+GROUND,func_z[1]}; //床の点(0,-GROUND,0)に関する球の変位ベクトル
    double n[]={0,1,0}; //床の法線単位ベクトル
    
    
    if(flag_y==0){
        //床と衝突していない時
        if(collision(vect_x,n)==false)
        {
            printf("t:%f,x:%f,y:%f,z:%f\n",t,func_x[1],func_y[1],func_z[1]);
            
            glTranslated(func_x[1],func_y[1],func_z[1]);
            
            glPushMatrix();
            
            glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, Obj_Color);
            glutSolidSphere(RADIUS,36,36);
            glPopMatrix();
            
            glutSwapBuffers();
            
            //はね返りが十分に小さい時
            if(func_y[0]<1)
                if(func_y[1]<1)
                    flag_y=1;
        }
        //床と衝突した時
        else if(collision(vect_x,n)==true){
            func_y[0]=-e*func_y[0];
            
            printf("t:%f,x:%f,y:%f,z:%f\n",t,func_x[1],func_y[1],func_z[1]);
            
            glTranslated(func_x[1],func_y[1],func_z[1]);
            
            glPushMatrix();
            glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, Obj_Color);
            glutSolidSphere(RADIUS,36,36);
            glPopMatrix();
            
            glutSwapBuffers();
        }
        runge_kutta_y(t,func_y);
    }
    
    //はね返りが十分に小さくなった時、はね返りをやめる
    else if(flag_y==1){
        func_y[0]=0;
        func_y[1]=-GROUND+RADIUS;
        printf("t:%f,x:%f,y:%f,z:%f\n",t,func_x[1],func_y[1],func_z[1]);
        
        glTranslated(func_x[1],func_y[1],func_z[1]);
        
        glPushMatrix();
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, Obj_Color);
        
        glutSolidSphere(RADIUS,36,36);
        glPopMatrix();
        
        glutSwapBuffers();
    }
    
    t+=dt;
    runge_kutta_x(t,func_x);
    runge_kutta_z(t,func_z);
}

void reshape(int w, int h)
{
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(30.0, (double)w / (double)h, 1.0, 200.0);
    glMatrixMode(GL_MODELVIEW);
    gluLookAt(3.0, 4.0, 5.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
}

void timer(int value)
{
    glutPostRedisplay();
    glutTimerFunc(1,timer,0); //タイマー関数
}

void keyboard(unsigned char key, int x, int y)
{
    switch(key)
    {
        case 'q': //終了
            cout << "quit\n"<<endl;
            exit(0);
            break;
            
        case 'r': //reset
            t = 0;
            func_y[0]=START_VY;
            func_y[1]=START_Y;
            func_x[0]=START_VX;
            func_x[1]=START_X;
            func_z[0]=START_VZ;
            func_z[1]=START_Z;
            flag_y=0;
            break;
    }
    
}

//内積
double dot(double x[],double y[]){
    double xy=0.0;
    int i;
    for(i=0;i<3;i++)
        xy+=x[i]*y[i];
    return xy;
}

//衝突判定
int collision(double x[],double y[]){
    if(dot(x,y)<RADIUS)
        return true;
    
    else
        return false;
}

//y方向成分についての運動方程式
double func1y(double t,double vy,double y){
    double diff=-g-k/m*vy;
    return diff;
}
double func2y(double t,double vy,double y){
    double diff=vy;
    return diff;
}

//y方向成分について解く (ルンゲ・クッタ法)
void runge_kutta_y(double t,double y_func[]){
    double k1[2],k2[2],k3[2],k4[2];
    
    k1[0]=dt*func1y(t,y_func[0],y_func[1]);
    k1[1]=dt*func2y(t,y_func[0],y_func[1]);
    
    k2[0]=dt*func1y(t+dt/2,y_func[0]+k1[0]/2,y_func[1]+k1[1]/2);
    k2[1]=dt*func2y(t+dt/2,y_func[0]+k1[0]/2,y_func[1]+k1[1]/2);
    
    k3[0]=dt*func1y(t+dt/2,y_func[0]+k2[0]/2,y_func[1]+k2[1]/2);
    k3[1]=dt*func2y(t+dt/2,y_func[0]+k2[0]/2,y_func[1]+k2[1]/2);
    
    k4[0]=dt*func1y(t+dt,y_func[0]+k3[0],y_func[1]+k3[1]);
    k4[1]=dt*func2y(t+dt,y_func[0]+k3[0],y_func[1]+k3[1]);
    
    y_func[0]+=(k1[0]+2*k2[0]+2*k3[0]+k4[0])/6;
    y_func[1]+=(k1[1]+2*k2[1]+2*k3[1]+k4[1])/6;
}

//x方向成分についての運動方程式
double func1x(double t,double vx,double x){
    double diff=-k*vx/m;
    return diff;
}
double func2x(double t,double vx,double x){
    double diff=vx;
    return diff;
}

//x方向成分について解く (ルンゲ・クッタ法)
void runge_kutta_x(double t,double x_func[]){
    double k1[2],k2[2],k3[2],k4[2];
    
    k1[0]=dt*func1x(t,x_func[0],x_func[1]);
    k1[1]=dt*func2x(t,x_func[0],x_func[1]);
    
    k2[0]=dt*func1x(t+dt/2,x_func[0]+k1[0]/2,x_func[1]+k1[1]/2);
    k2[1]=dt*func2x(t+dt/2,x_func[0]+k1[0]/2,x_func[1]+k1[1]/2);
    
    k3[0]=dt*func1x(t+dt/2,x_func[0]+k2[0]/2,x_func[1]+k2[1]/2);
    k3[1]=dt*func2x(t+dt/2,x_func[0]+k2[0]/2,x_func[1]+k2[1]/2);
    
    k4[0]=dt*func1x(t+dt,x_func[0]+k3[0],x_func[1]+k3[1]);
    k4[1]=dt*func2x(t+dt,x_func[0]+k3[0],x_func[1]+k3[1]);
    
    x_func[0]+=(k1[0]+2*k2[0]+2*k3[0]+k4[0])/6;
    x_func[1]+=(k1[1]+2*k2[1]+2*k3[1]+k4[1])/6;
}

//z方向成分についての運動方程式
double func1z(double t,double vz,double z){
    double diff=-k*vz/m;
    return diff;
}
double func2z(double t,double vz,double z){
    double diff=vz;
    return diff;
}

//z方向成分について解く (ルンゲ・クッタ法)
void runge_kutta_z(double t,double z_func[]){
    double k1[2],k2[2],k3[2],k4[2];
    
    k1[0]=dt*func1z(t,z_func[0],z_func[1]);
    k1[1]=dt*func2z(t,z_func[0],z_func[1]);
    
    k2[0]=dt*func1z(t+dt/2,z_func[0]+k1[0]/2,z_func[1]+k1[1]/2);
    k2[1]=dt*func2z(t+dt/2,z_func[0]+k1[0]/2,z_func[1]+k1[1]/2);
    
    k3[0]=dt*func1z(t+dt/2,z_func[0]+k2[0]/2,z_func[1]+k2[1]/2);
    k3[1]=dt*func2z(t+dt/2,z_func[0]+k2[0]/2,z_func[1]+k2[1]/2);
    
    k4[0]=dt*func1z(t+dt,z_func[0]+k3[0],z_func[1]+k3[1]);
    k4[1]=dt*func2z(t+dt,z_func[0]+k3[0],z_func[1]+k3[1]);
    
    z_func[0]+=(k1[0]+2*k2[0]+2*k3[0]+k4[0])/6;
    z_func[1]+=(k1[1]+2*k2[1]+2*k3[1]+k4[1])/6;
}

// 床描画
void DRAW_FLOOR()
{
    glDisable(GL_LIGHTING);
    glDisable(GL_LIGHT0);
    
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    
    glPushMatrix();
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, floor_Color);
    glBegin(GL_QUADS);
    glVertex3d(-FIELD,-GROUND,-FIELD);
    glVertex3d(-FIELD,-GROUND,FIELD);
    glVertex3d(FIELD,-GROUND,FIELD);
    glVertex3d(FIELD,-GROUND,-FIELD);
    glEnd();
    glPopMatrix();
}
