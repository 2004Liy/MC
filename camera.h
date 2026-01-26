#ifndef CAMERA_H
#define CAMERA_H
#include<QVector3D>
#include<QMatrix4x4>

const float YAW=-90.0f;//俯仰角
const float PITCH=0.0f;//水平角
enum Camera_Movement{
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    UP,
    DOWN
};

class Camera{
public:
    QVector3D Position;
    QVector3D Front;
    QVector3D Up;
    QVector3D WorldUp;
    QVector3D Right;
    QVector3D Front1;
    float Yaw;
    float Pitch;
    float Zoom=60;
//1870,-1250
    Camera(QVector3D position=QVector3D(1870.0f,60.0f,-1250.0f),QVector3D up=QVector3D(0.0f,1.0f,0.0f),float yaw=YAW,float pitch=0.0){
        Position=position;
        WorldUp=up;
        Yaw=yaw;
        Pitch=pitch;
        updateCameraVectors();
    }

    QMatrix4x4 getViewMatrix(){
        QMatrix4x4 theMattrix;
        theMattrix.lookAt(Position,Position+Front,Up);
        return theMattrix;
    }

    void ProcessMouseMoveement(float xoff,float yoff,bool con=1){
        Yaw+=xoff;
        Pitch+=yoff;
        if(con){
            if(Pitch>89.0f){
                Pitch=89.0f;
            }
            if(Pitch<-89.0f){
                Pitch=-89.0f;
            }
        }
        updateCameraVectors();
    }

    void ProcessKeyboard(Camera_Movement dir,float delTime){
        float velocity=0.006f*20;
        if(dir==FORWARD){
            Position+=Front1*velocity;
        }else if(dir==BACKWARD){
            Position-=Front1*velocity;
        }else if(dir==LEFT){
            Position-=Right*velocity*0.5;
        }else if(dir==RIGHT){
            Position+=Right*velocity*0.5;
        }else if(dir==UP){
            Position+=WorldUp*velocity;
        }else if(dir==DOWN){
            Position-=WorldUp*velocity;
        }
    }

protected:
    void updateCameraVectors()//更新看向的点
    {
        float PI=3.1415926535;
        QVector3D front;
        front.setX(cos(Yaw*PI/180.0)*cos(Pitch*PI/180.0));
        front.setY(sin(Pitch*PI/180.0));
        front.setZ(sin(Yaw*PI/180.0)*cos(Pitch*PI/180.0));
        front.normalize();
        Front=front;
        Front1=Front;
        Front1.setY(0);
        Front1.normalize();
        Right=QVector3D::crossProduct(Front1,WorldUp);
        Right.normalize();
        Up=QVector3D::crossProduct(Right,Front1);
        Up.normalize();
    }
};

#endif // CAMERA_H
