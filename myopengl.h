#ifndef MYOPENGL_H
#define MYOPENGL_H

#include <QOpenGLWidget>
#include<QOpenGLFunctions_3_3_Core>
#include<QOpenGLShaderProgram>
#include<QOpenGLTexture>
#include<QVector>
#include<QMouseEvent>
#include<QKeyEvent>
#include<QTimer>
#include<QObject>
#include<QPoint>
#include<QMap>
#include<QSet>
#include"camera.h"
#include"FrustumCulling.h"

struct coordinate{
    int x,z;
public:
    bool operator<(const coordinate& other)const{
        if(x!=other.x) return x<other.x;
        return z<other.z;
    }
};

struct placttheblock{
    float x,y,z;
    float blocktype;
    bool operator==(const placttheblock&other)const{
        return this->x==other.x&&this->y==other.y&&this->z==other.z;
    }
};


class MyOpenGL : public QOpenGLWidget,QOpenGLFunctions_3_3_Core
{
    Q_OBJECT
public:
    MyOpenGL(QWidget *parent = nullptr);
    ~MyOpenGL();
protected:
    virtual void initializeGL()override;
    virtual void resizeGL(int w, int h)override;
    virtual void paintGL()override;
    virtual void mouseMoveEvent(QMouseEvent *event)override;
    virtual void keyPressEvent(QKeyEvent *event)override;
    virtual void keyReleaseEvent(QKeyEvent *event)override;
    virtual void mousePressEvent(QMouseEvent *event)override;
    virtual void wheelEvent(QWheelEvent *event) override;

private:
    void enableMouseLock(bool enable);
    QOpenGLTexture *loadTexture(const QString &imagePath);
    void createChunks();
    void updateChunkVisibility();
    void drawchunk();
    void drawTerrain();
    void updatechunkmap();
    void updatePhysics(float deltaTime);
    void handleCollisions(QVector3D& newPosition);
    void handleMovement(float deltaTime);
    void handleHorizontalCollisions(QVector3D &playerPos);
    bool checkSolidBlock(float worldX, float worldY, float worldZ);
    void drawCrosshair();
    QVector3D getTargetBlockPosition();
    QVector3D getWorldCenterPosition(float depth);
    bool checkAnyBlockAt(const QVector3D& worldPos);
    void drawBlockOutline(const QVector3D& blockCenter);
    void createOutlineVAO();
    void drawBottomImage(QPainter &painter);
    void drawItemIcons(QPainter &painter, int barX, int barY, int barWidth, int barHeight);
    QVector3D getplacementlocation();
    void initplaceblock();
    void updateplaceblockdata();
    void chunkaddblock(QVector3D pos);

private:
    int num=0;
    QSharedPointer<QOpenGLTexture> skybox;
    QOpenGLShaderProgram skyboxshader;
    QOpenGLShaderProgram blockshader;
    QOpenGLShaderProgram treeshader;
    QOpenGLShaderProgram flowershader;
    QOpenGLShaderProgram outlineShader;
    QOpenGLShaderProgram placeblockshader;
    unsigned int outlineVAO, outlineVBO, outlineEBO;
    unsigned int placeblockVAO,placeblockVBO,placeblockinstVBO;
    Camera camear;
    QTimer timer;
    bool mouseLocked = false;
    QPoint lastMousePos;
    QVector<QOpenGLTexture*>m_textures;
    QVector<QOpenGLTexture*>treetexture;
    QVector<QOpenGLTexture*>flowertexture;
    QVector<QOpenGLTexture*>placeblocktexture;

    Frustum frustum;
    QMap<coordinate,std::shared_ptr<Chunk>>chunkmap;
    QVector<coordinate>deletechunk;
    coordinate maparray[21][21];
    QSet<int> pressedKeys;


    QVector3D playerVelocity;  // 玩家速度
    QVector3D playerAcceleration; // 玩家加速度
    bool isOnGround=false;           // 是否在地面上
    float playerHeight = 1.8f; // 玩家高度（从脚到头）
    float playerWidth = 0.2f;  // 玩家宽度（半径）
    float gravity = 9.8f;    // 重力加速度
    float jumpForce = 11.0f;   // 跳跃力量
    float playerSpeed = 0.15f; // 玩家移动速度
    bool blockedDirections[4] = {false, false, false, false};

    bool ishit=false;
    QVector3D targetBlock;
    QVector3D movedir;

    QVector<QPixmap>placeblock;

    int curblock=0;
    QVector3D impacepoint;
    QList<placttheblock>placeblocklist;
signals:
    void closemc();
};

#endif // MYOPENGL_H
