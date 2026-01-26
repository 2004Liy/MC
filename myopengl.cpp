#include "myopengl.h"
#include"vertes.h"
#include<QElapsedTimer>
#include<QPainter>
unsigned int skyboxVAO,skyboxVBO;
QMatrix4x4 projection,view,model;
float currentFrame,lastFrame;
QElapsedTimer elapsedtimer;

MyOpenGL::MyOpenGL(QWidget *parent):QOpenGLWidget(parent) {
    setMouseTracking(true);
    enableMouseLock(true);
    timer.setInterval(16);
    connect(&timer,&QTimer::timeout,[&](){
        currentFrame = elapsedtimer.elapsed();
        float deltaTime = (currentFrame - lastFrame) * 0.001f;
        lastFrame = currentFrame;
        handleMovement(0.001);//移动
        // 更新物理
        updatePhysics(deltaTime);
        //makeCurrent();
        update();//绘制
        //doneCurrent();

        num++;
        if(num>=60){
            updatechunkmap();//更新地图
            num%=60;
        }
    });
    timer.start();
    elapsedtimer.start();

}


void MyOpenGL::initializeGL()
{
    initializeOpenGLFunctions();
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glEnable(GL_BLEND);//启用混合
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    //启用背面剔除以提高性能
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glViewport(0, 0, width(), height());

    // 初始投影矩阵
    projection.setToIdentity();
    projection.perspective(camear.Zoom, (float)width()/height(), 0.01f, 200.0f);
    // projection.perspective(camear.Zoom,(float)width()/height(),0.01f,200.0f);
    view=camear.getViewMatrix();

    frustum.update(projection*view);//更新视锥体
    updateChunkVisibility();//更新区块的可见性

    m_textures.append(loadTexture(":/icon/GrassBlock/front.png"));   // 前面
    m_textures.append(loadTexture(":/icon/GrassBlock/back.png"));    // 后面
    m_textures.append(loadTexture(":/icon/GrassBlock/left.png"));    // 左面
    m_textures.append(loadTexture(":/icon/GrassBlock/right.png"));   // 右面
    m_textures.append(loadTexture(":/icon/GrassBlock/top.png"));     // 上面
    m_textures.append(loadTexture(":/icon/GrassBlock/bottom.png"));
    m_textures.append(loadTexture(":/icon/water/water.png"));
    m_textures.append(loadTexture(":/icon/Sand/back.png"));
    m_textures.append(loadTexture(":/icon/Stone/back.png"));
    m_textures.append(loadTexture(":/icon/Dirt/back.png"));
    //设置树的纹理
    treetexture.append(loadTexture(":/icon/OakLog/front.png"));
    treetexture.append(loadTexture(":/icon/OakLog/top.png"));
    treetexture.append(loadTexture(":/icon/LeaveGreen/111.png"));

    //设置放置方块纹理
    placeblocktexture.append(loadTexture(":/icon/blocktexture/board.png"));
    placeblocktexture.append(loadTexture(":/icon/blocktexture/bricks.png"));
    placeblocktexture.append(loadTexture(":/icon/blocktexture/ice.png"));
    placeblocktexture.append(loadTexture(":/icon/blocktexture/readbrick.png"));
    placeblocktexture.append(loadTexture(":/icon/blocktexture/brick.png"));
    placeblocktexture.append(loadTexture(":/icon/blocktexture/glass.png"));
    placeblocktexture.append(loadTexture(":/icon/blocktexture/dirt.png"));
    placeblocktexture.append(loadTexture(":/icon/blocktexture/wool.png"));
    placeblocktexture.append(loadTexture(":/icon/blocktexture/stone.png"));
    //设置花和草的纹理
    QImage img1(":/icon/Grass_3.png");
    img1 = img1.mirrored(false, true);
    img1 = img1.convertToFormat(QImage::Format_RGBA8888);
    QOpenGLTexture *texture1 = new QOpenGLTexture(img1);
    texture1->setMinificationFilter(QOpenGLTexture::Linear); // 不使用mipmap
    texture1->setMagnificationFilter(QOpenGLTexture::Linear);
    texture1->setWrapMode(QOpenGLTexture::ClampToEdge);
    flowertexture.append(texture1);
    QImage img2(":/icon/Flower.png");
    img2 = img2.mirrored(false, true);
    img2 = img2.convertToFormat(QImage::Format_RGBA8888);
    QOpenGLTexture *texture2 = new QOpenGLTexture(img2);
    texture2->setMinificationFilter(QOpenGLTexture::Linear); // 不使用mipmap
    texture2->setMagnificationFilter(QOpenGLTexture::Linear);
    texture2->setWrapMode(QOpenGLTexture::ClampToEdge);
    flowertexture.append(texture2);


    placeblock.append(QPixmap(":/icon/placeblock/BirchPlanks.png"));
    placeblock.append(QPixmap(":/icon/placeblock/BlockOfDiamond.png"));
    placeblock.append(QPixmap(":/icon/placeblock/Ice.png"));
    placeblock.append(QPixmap(":/icon/placeblock/Bricks.png"));
    placeblock.append(QPixmap(":/icon/placeblock/DiamondOre.png"));
    placeblock.append(QPixmap(":/icon/placeblock/Glass.png"));
    placeblock.append(QPixmap(":/icon/placeblock/GrassBlock.png"));
    placeblock.append(QPixmap(":/icon/placeblock/LightBlueWool.png"));
    placeblock.append(QPixmap(":/icon/placeblock/Stone.png"));


    blockshader.addShaderFromSourceFile(QOpenGLShader::Vertex,":/shader/block.vert");
    blockshader.addShaderFromSourceFile(QOpenGLShader::Fragment,":/shader/block.frag");
    blockshader.link();

    treeshader.addShaderFromSourceFile(QOpenGLShader::Vertex,":/shader/tree.vert");
    treeshader.addShaderFromSourceFile(QOpenGLShader::Fragment,":/shader/tree.frag");
    treeshader.link();

    flowershader.addShaderFromSourceFile(QOpenGLShader::Vertex,":/shader/flower.vert");
    flowershader.addShaderFromSourceFile(QOpenGLShader::Fragment,":/shader/flower.frag");
    flowershader.link();

    outlineShader.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shader/outline.vert");
    outlineShader.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shader/outline.frag");
    outlineShader.link();

    createOutlineVAO();

    initplaceblock();

    updatechunkmap();//更新地图

}



void MyOpenGL::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);
    projection.setToIdentity();
    projection.perspective(camear.Zoom, (float)w/h, 0.01f, 200.0f);
}



void MyOpenGL::paintGL()
{
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glDepthMask(GL_TRUE); // 启用深度写入
    glEnable(GL_DEPTH_TEST);//开起深度测试

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    view=camear.getViewMatrix();

    drawTerrain();//绘制地图

    targetBlock=getTargetBlockPosition();//计算当前选中方块的坐标
    drawBlockOutline(targetBlock);//绘制选中方块的外框

    drawCrosshair();//绘制准星
}


void MyOpenGL::mouseMoveEvent(QMouseEvent *event)
{
    if (mouseLocked) {
        QPoint center = rect().center();
        QPoint delta = event->pos() - center;
        if (!delta.isNull()) {
            makeCurrent();
            float sensitivity = 0.2f;
            camear.ProcessMouseMoveement(delta.x() * sensitivity, -delta.y() * sensitivity);
            doneCurrent();
            QCursor::setPos(mapToGlobal(center));
            lastMousePos = center;
        }
        view=camear.getViewMatrix();
        frustum.update(projection*view);
        updateChunkVisibility();
    }
}



MyOpenGL::~MyOpenGL()
{
    makeCurrent();
    skybox.reset();
    if(placeblockVAO)glDeleteVertexArrays(1,&placeblockVAO);
    if(placeblockVBO)glDeleteBuffers(1,&placeblockVBO);
    if(placeblockinstVBO)glDeleteBuffers(1,&placeblockinstVBO);

    if(outlineVAO)glDeleteVertexArrays(1,&outlineVAO);
    if(outlineVBO)glDeleteBuffers(1,&outlineVBO);
    if(outlineEBO)glDeleteBuffers(1,&outlineEBO);
    doneCurrent();
}



void MyOpenGL::keyPressEvent(QKeyEvent *event)
{
    //qDebug()<<camear.Position<<'\n';

    currentFrame=elapsedtimer.elapsed();
    float deltaTime=currentFrame-lastFrame;
    if(deltaTime>50)deltaTime=50;
    lastFrame=currentFrame;
    switch (event->key()) {
    case Qt::Key_Escape:
        emit closemc();
        break;
    case Qt::Key_Space:
        if(isOnGround){
            playerVelocity.setY(jumpForce);
            isOnGround=false;
        }
        break;
    default:
        pressedKeys.insert(event->key());
        break;
    }
    view=camear.getViewMatrix();
    frustum.update(projection*view);
    updateChunkVisibility();
    //paintGL();

}

void MyOpenGL::keyReleaseEvent(QKeyEvent *event)
{
    pressedKeys.remove(event->key());
    movedir.setX(0);
    movedir.setY(0);
    movedir.setZ(0);
}

void MyOpenGL::mousePressEvent(QMouseEvent *event)
{
    if(event->button()==Qt::MouseButton::LeftButton){
        if(ishit){
            qDebug()<<targetBlock<<'\n';
            bool isok=false;
            for(int i=placeblocklist.size()-1;i>=0;i--){
                if(placeblocklist[i].x==targetBlock.x()&&placeblocklist[i].y==targetBlock.y()&&placeblocklist[i].z==targetBlock.z()){
                    placeblocklist.removeAt(i);
                    updateplaceblockdata();
                    isok=true;
                }
            }
            for(auto &it:chunkmap){
                if(targetBlock.x()>=it->chunkx&&targetBlock.x()<it->chunkx+WORLD_BLOCK&&
                    targetBlock.z()>=it->chunkz&&targetBlock.z()<it->chunkz+WORLD_BLOCK){
                    if(isok){
                        it->deleteblock(targetBlock);
                    }
                    it->eliminateblock(targetBlock);
                    break;
                }
            }
        }else{
            qDebug()<<"没有物体"<<'\n';
        }
    }else if(event->button()==Qt::MouseButton::RightButton){
        if(!ishit)return;
        QVector3D normal=getplacementlocation();
        QVector3D p=normal+targetBlock;
        if(abs(p.x()-camear.Position.x())<0.6&&abs(p.z()-camear.Position.z())<0.6&&abs(camear.Position.y()-0.9f-p.y())<=0.9){
            return;
        }
        placttheblock pp{p.x(),p.y(),p.z(),(float)curblock};
        placeblocklist.append(pp);
        updateplaceblockdata();
        chunkaddblock(p);
    }
}

void MyOpenGL::wheelEvent(QWheelEvent *event)
{
    QPoint angleDelta=event->angleDelta();
    float wheelAccumulator=0.0f;
    if(!angleDelta.isNull()){
        wheelAccumulator+=static_cast<float>(angleDelta.y());
        if(wheelAccumulator<0){
            curblock+=abs(wheelAccumulator/120);
            curblock%=9;
        }else{
            int num=wheelAccumulator/120;
            curblock-=num;
            if(curblock<0){
                curblock=9+curblock;
            }
        }
    }
}

void MyOpenGL::enableMouseLock(bool enable)
{
    mouseLocked = enable;

    if (mouseLocked) {
        setCursor(Qt::BlankCursor);
        grabMouse();
        lastMousePos = rect().center();
        QCursor::setPos(mapToGlobal(lastMousePos));
    } else {
        setCursor(Qt::ArrowCursor);
        releaseMouse();
    }
}

QOpenGLTexture *MyOpenGL::loadTexture(const QString &imagePath)
{
    QImage img(imagePath);
    if (img.isNull()) {
        qDebug() << "纹理加载失败：" << imagePath;
        return nullptr;
    }
    img = img.mirrored(false, true);
    img = img.convertToFormat(QImage::Format_RGBA8888);

    QOpenGLTexture *texture = new QOpenGLTexture(img);
    texture->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
    texture->setMagnificationFilter(QOpenGLTexture::Linear);
    texture->setWrapMode(QOpenGLTexture::Repeat);
    return texture;
}


void MyOpenGL::createChunks()//创建区块
{
    for(int i=0;i<21;i++){
        for(int j=0;j<21;j++){
            auto it=chunkmap.find(maparray[i][j]);
            if(it==chunkmap.end()){
                std::shared_ptr<Chunk> ptr=std::make_shared<Chunk>(maparray[i][j].x,maparray[i][j].z);
                chunkmap.insert(maparray[i][j],ptr);
                ptr->calculateVertices();
                ptr->initchunk();
                ptr->generateTreesInChunk();//设置区块内树的位置
                ptr->settreedata();//把树的位置放入容器
                ptr->generateInstancedData();
            }
        }
    }
}

void MyOpenGL::updateChunkVisibility()//更新区块的可见性
{
    int sum=0,ss=0;
    for(auto &it:chunkmap){
        ss++;
        if(fmax(abs(it->chunkx-camear.Position.x()),abs(it->chunkz-camear.Position.z()))<=WORLD_BLOCK*2){
            sum++;
            it->visible=true;
            continue;
        }
        if(frustum.containsCube(it->getpos())){
            it->visible=true;
            sum++;
        }else{
            it->visible=false;
        }
    }
    //qDebug()<<"总块数:"<<ss<<'\n';
    //qDebug()<<"可见的块数:"<<sum<<'\n';
}

void MyOpenGL::drawTerrain()
{
    //绘制地形

    blockshader.bind();
    blockshader.setUniformValue("view",view);
    blockshader.setUniformValue("projection",projection);
    blockshader.setUniformValue("viewPos",camear.Position);
    blockshader.setUniformValue("fogHeightThreshold",28.0f);
    glActiveTexture(GL_TEXTURE0);
    m_textures[0]->bind();
    blockshader.setUniformValue("texFront", 0);

    glActiveTexture(GL_TEXTURE1);
    m_textures[1]->bind();
    blockshader.setUniformValue("texBack", 1);

    glActiveTexture(GL_TEXTURE2);
    m_textures[2]->bind();
    blockshader.setUniformValue("texLeft", 2);

    glActiveTexture(GL_TEXTURE3);
    m_textures[3]->bind();
    blockshader.setUniformValue("texRight", 3);

    glActiveTexture(GL_TEXTURE4);
    m_textures[4]->bind();
    blockshader.setUniformValue("texTop", 4);

    glActiveTexture(GL_TEXTURE5);
    m_textures[5]->bind();
    blockshader.setUniformValue("texBottom", 5);

    glActiveTexture(GL_TEXTURE6);
    m_textures[6]->bind();
    blockshader.setUniformValue("water", 6);

    glActiveTexture(GL_TEXTURE7);
    m_textures[8]->bind();
    blockshader.setUniformValue("rock", 7);

    glActiveTexture(GL_TEXTURE8);
    m_textures[7]->bind();
    blockshader.setUniformValue("sand", 8);

    glActiveTexture(GL_TEXTURE9);
    m_textures[9]->bind();
    blockshader.setUniformValue("dirt", 9);

    for(auto &it:chunkmap){
        if(it->visible){
            it->drawchunk();
        }

    }
    blockshader.release();

    //绘制树
    treeshader.bind();
    treeshader.setUniformValue("view",view);
    treeshader.setUniformValue("projection",projection);
    treeshader.setUniformValue("viewPos",camear.Position);
    glActiveTexture(GL_TEXTURE0);
    treetexture[0]->bind();
    treeshader.setUniformValue("treesurface", 0);

    glActiveTexture(GL_TEXTURE1);
    treetexture[1]->bind();
    treeshader.setUniformValue("treeinterior", 1);

    glActiveTexture(GL_TEXTURE2);
    treetexture[2]->bind();
    treeshader.setUniformValue("leaf", 2);

    for(auto &it:chunkmap){
        if(it->visible){
            it->drawtree();
        }
    }
    treeshader.release();

    //绘制玩家放置的物体
    placeblockshader.bind();
    placeblockshader.setUniformValue("view",view);
    placeblockshader.setUniformValue("projection",projection);
    placeblockshader.setUniformValue("viewPos",camear.Position);

    for(int i=0;i<=8;i++){
        glActiveTexture(GL_TEXTURE0+i);
        placeblocktexture[i]->bind();
        QString uniformName=QString("tex%1").arg(i);
        placeblockshader.setUniformValue(uniformName.toUtf8().constData(),i);
    }
    glBindVertexArray(placeblockVAO);
    glDrawArraysInstanced(GL_TRIANGLES,0,36,placeblocklist.size());
    glBindVertexArray(0);

    //绘制花草
    GLboolean cullEnabled;
    glGetBooleanv(GL_CULL_FACE, &cullEnabled);
    GLint cullFaceMode;
    glGetIntegerv(GL_CULL_FACE_MODE, &cullFaceMode);
    // 关闭背面剔除
    glDisable(GL_CULL_FACE);

    flowershader.bind();
    flowershader.setUniformValue("view",view);
    flowershader.setUniformValue("projection",projection);
    flowershader.setUniformValue("viewPos",camear.Position);
    glActiveTexture(GL_TEXTURE0);
    flowertexture[0]->bind();
    flowershader.setUniformValue("grass", 0);

    glActiveTexture(GL_TEXTURE1);
    flowertexture[1]->bind();
    flowershader.setUniformValue("flower", 1);

    for(auto &it:chunkmap){
        if(it->visible){
            it->drawflower();
        }
    }
    flowershader.release();
    //开起背面剔除并恢复之前的状态
    if (cullEnabled) {
        glEnable(GL_CULL_FACE);
        glCullFace(cullFaceMode);
    }
}

void MyOpenGL::updatechunkmap()
{
    float curviewx=camear.Position.x();
    float curviewz=camear.Position.z();
    int chunkX=static_cast<int>(std::floor(curviewx/WORLD_BLOCK));
    int chunkZ=static_cast<int>(std::floor(curviewz/WORLD_BLOCK));

    for(auto &it:chunkmap.keys()){
        int xx=static_cast<int>(std::floor(it.x/WORLD_BLOCK));
        int zz=static_cast<int>(std::floor(it.z/WORLD_BLOCK));
        int d=std::max(abs(xx-chunkX),abs(zz-chunkZ));
        if(d>21){
            deletechunk.append(it);
        }
    }

    for(auto &it:deletechunk){//清理距离远的区块
        if(std::max(abs(it.x-chunkX),abs(it.z-chunkZ))<=11){
            continue;
        }
        chunkmap.take(it);
    }
    deletechunk.clear();

    for(int i=0;i<21;i++){
        for(int j=0;j<21;j++){
            int x=(chunkX+i-10)*WORLD_BLOCK;
            int z=(chunkZ+j-10)*WORLD_BLOCK;
            maparray[i][j]={x,z};
        }
    }
    makeCurrent();
    createChunks();
    doneCurrent();
}

void MyOpenGL::updatePhysics(float deltaTime){
    if (!isOnGround) {
        playerVelocity.setY(playerVelocity.y()-gravity*0.08);
    } else {
        playerVelocity.setY(0.0f);
    }
    QVector3D newPosition=camear.Position+playerVelocity*deltaTime;
    handleCollisions(newPosition);
    view = camear.getViewMatrix();
    frustum.update(projection*view);
    updateChunkVisibility();
}

void MyOpenGL::handleCollisions(QVector3D &newPosition)//碰撞检测
{
    float footHeight=newPosition.y()-1.8f;
    if (playerVelocity.y() > 0) {
        float headHeight = newPosition.y();
        bool hasBlockAbove = checkSolidBlock(newPosition.x() + 0.5f,
                                             headHeight + 0.2f,
                                             newPosition.z() + 0.5f);
        if (hasBlockAbove) {
            int blockY = static_cast<int>(std::floor(headHeight + 0.2f));
            newPosition.setY(blockY - 0.2f - 0.1f);
            playerVelocity.setY(0.0f);
        }
    }
    bool hasBlockBelow=checkSolidBlock(newPosition.x()+0.5f,footHeight-0.01f,newPosition.z()+0.5f);
    if(hasBlockBelow){
        int blockY=static_cast<int>(std::floor(footHeight - 0.01f));
        newPosition.setY(blockY+1.0f+1.8f);
        isOnGround=true;
        playerVelocity.setY(0.0f);
    } else {
        isOnGround=false;
    }

    handleHorizontalCollisions(newPosition);//水平碰撞检测

    camear.Position=newPosition;
}


void MyOpenGL::handleMovement(float deltaTime)
{
    if (pressedKeys.contains(Qt::Key_W)) {
        camear.ProcessKeyboard(FORWARD, deltaTime);
        movedir=camear.Front1;
    }
    if (pressedKeys.contains(Qt::Key_S)) {
        camear.ProcessKeyboard(BACKWARD, deltaTime);
        movedir=camear.Front1*-1;
    }
    if (pressedKeys.contains(Qt::Key_A)) {
        camear.ProcessKeyboard(LEFT, deltaTime);
        movedir=camear.Right*-1;
    }
    if (pressedKeys.contains(Qt::Key_D)) {
        camear.ProcessKeyboard(RIGHT, deltaTime);
        movedir=camear.Right;
    }
    if (pressedKeys.contains(Qt::Key_Q)) {
        camear.ProcessKeyboard(UP, deltaTime);
    }
    if (pressedKeys.contains(Qt::Key_E)) {
        camear.ProcessKeyboard(DOWN, deltaTime);
    }
    movedir.normalize();
    isOnGround=false;
}

void MyOpenGL::handleHorizontalCollisions(QVector3D &playerPos)//水平碰撞
{
    if(movedir.x()==0&&movedir.y()==0&&movedir.z()==0){
        return;
    }
    bool hasCollision=false;

    QVector3D checkPos=playerPos+movedir*playerWidth;
    for(float y=playerPos.y()-1.8f;y<=playerPos.y();y+=0.2){
        if(checkSolidBlock(checkPos.x()+0.5,y,checkPos.z()+0.5)){
            hasCollision=true;
            break;
        }
    }
    if(hasCollision){
        playerPos.setX(playerPos.x()-0.12f*movedir.x());
        playerPos.setZ(playerPos.z()-0.12f*movedir.z());
    }
}

bool MyOpenGL::checkSolidBlock(float worldX, float worldY, float worldZ)
{
    if (worldY < 0) return true;
    int chunkX = static_cast<int>(std::floor(worldX / WORLD_BLOCK)) * WORLD_BLOCK;
    int chunkZ = static_cast<int>(std::floor(worldZ / WORLD_BLOCK)) * WORLD_BLOCK;
    auto it = chunkmap.find({chunkX, chunkZ});
    if (it == chunkmap.end()) {
        return false;
    }
    std::shared_ptr<Chunk> chunk = it.value();
    int localX = static_cast<int>(std::floor(worldX)) - chunkX;
    int localY = static_cast<int>(std::floor(worldY));
    int localZ = static_cast<int>(std::floor(worldZ)) - chunkZ;
    if(worldY>=localY){
        return chunk->boolterrainmap[localX][localY+1][localZ];
    }else{
        return false;
    }
}


void MyOpenGL::drawCrosshair()
{
    glUseProgram(0);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_BLEND);
    glDepthMask(GL_FALSE);

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
    QPen pen(Qt::darkGray);
    pen.setWidth(2);
    painter.setPen(pen);

    int centerX = width() / 2;
    int centerY = height() / 2;
    int crossSize = 10;
    painter.drawLine(centerX, centerY - crossSize, centerX, centerY + crossSize);
    painter.drawLine(centerX - crossSize, centerY, centerX + crossSize, centerY);

    drawBottomImage(painter);

    painter.end();

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
}

QVector3D MyOpenGL::getTargetBlockPosition()
{
    QVector3D rayOrigin = camear.Position;
    QVector3D rayDirection = (getWorldCenterPosition(1.0f) - rayOrigin).normalized();

    float maxDistance = 10.0f;
    float step = 0.1f;
    QVector3D lasepoint;
    for (float t = 0; t < maxDistance; t += step) {
        QVector3D testPos = rayOrigin + rayDirection * t;
        testPos.setX(testPos.x()+0.5f);
        testPos.setZ(testPos.z()+0.5f);
        testPos.setY(testPos.y()+0.5f);
        if (checkAnyBlockAt(testPos)) {
            impacepoint=(lasepoint+testPos)*0.5;
            float blockX = static_cast<int>(std::floor(testPos.x()));
            float blockY = static_cast<int>(std::floor(testPos.y()));
            float blockZ = static_cast<int>(std::floor(testPos.z()));
            ishit = true;
            return QVector3D(blockX, blockY, blockZ);
        }else{
            lasepoint=testPos;
        }
    }
    ishit = false;
    return QVector3D();
}

QVector3D MyOpenGL::getWorldCenterPosition(float depth)
{
    float x = 0.0f;
    float y = 0.0f;
    float z = depth;

    QVector4D clipCoord(x, y, /*2.0f * */z /*- 1.0f*/, 1.0f);

    QMatrix4x4 invProjection = projection.inverted();
    QVector4D viewCoord = invProjection * clipCoord;
    viewCoord /= viewCoord.w();

    QMatrix4x4 invView = view.inverted();
    QVector4D worldCoord = invView * viewCoord;

    return worldCoord.toVector3D();
}

bool MyOpenGL::checkAnyBlockAt(const QVector3D &worldPos)
{
    int chunkX = static_cast<int>(std::floor(worldPos.x() / WORLD_BLOCK)) * WORLD_BLOCK;
    int chunkZ = static_cast<int>(std::floor(worldPos.z() / WORLD_BLOCK)) * WORLD_BLOCK;
    auto it = chunkmap.find({chunkX, chunkZ});
    if (it == chunkmap.end()) {
        return false;
    }
    std::shared_ptr<Chunk> chunk = it.value();
    int localX = static_cast<int>(std::floor(worldPos.x())) - chunkX;
    int localY = static_cast<int>(std::floor(worldPos.y()));
    int localZ = static_cast<int>(std::floor(worldPos.z())) - chunkZ;
    if (localX < 0 || localX >= WORLD_BLOCK ||
        localY < 0 || localY >= WORLD_HEIGHT ||
        localZ < 0 || localZ >= WORLD_BLOCK) {
        return false;
    }
    return chunk->allterrainmap[localX][localY][localZ];
}

void MyOpenGL::drawBlockOutline(const QVector3D &blockCenter)
{
    if (!ishit) return;  // 没有选中则不绘制

    //保存并修改OpenGL状态
    GLboolean depthTestEnabled, cullEnabled;
    glGetBooleanv(GL_DEPTH_TEST, &depthTestEnabled);
    glGetBooleanv(GL_CULL_FACE, &cullEnabled);

    //glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glLineWidth(1.0f);  // 设置线宽

    // 使用边框着色器
    outlineShader.bind();

    // 计算MVP矩阵
    QMatrix4x4 model;
    model.translate(blockCenter);
    model.scale(1.008f);  // 轻微放大避免深度冲突

    QMatrix4x4 mvp = projection * view * model;
    outlineShader.setUniformValue("mvp", mvp);
    outlineShader.setUniformValue("outlineColor", QColor(255, 255, 0)); // 黄色

    // 绘制边框
    glBindVertexArray(outlineVAO);
    glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    outlineShader.release();

    //恢复OpenGL状态
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    if (depthTestEnabled) glEnable(GL_DEPTH_TEST);
    if (cullEnabled) glEnable(GL_CULL_FACE);
}

void MyOpenGL::createOutlineVAO()
{
    float vertices[8][3] = {
        {-0.5f, -0.5f, -0.5f},  // 0
        { 0.5f, -0.5f, -0.5f},  // 1
        { 0.5f,  0.5f, -0.5f},  // 2
        {-0.5f,  0.5f, -0.5f},  // 3
        {-0.5f, -0.5f,  0.5f},  // 4
        { 0.5f, -0.5f,  0.5f},  // 5
        { 0.5f,  0.5f,  0.5f},  // 6
        {-0.5f,  0.5f,  0.5f}   // 7
    };

    // 12条边的索引（每2个索引定义一条线段）
    GLuint indices[24] = {
        0,1, 1,2, 2,3, 3,0,  // 底面
        4,5, 5,6, 6,7, 7,4,  // 顶面
        0,4, 1,5, 2,6, 3,7   // 竖边
    };

    // 创建VAO
    glGenVertexArrays(1, &outlineVAO);
    glGenBuffers(1, &outlineVBO);
    glGenBuffers(1, &outlineEBO);

    glBindVertexArray(outlineVAO);

    // 绑定VBO并上传顶点数据
    glBindBuffer(GL_ARRAY_BUFFER, outlineVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // 绑定EBO并上传索引数据
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, outlineEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // 设置顶点属性指针
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,3*sizeof(float),(void*)0);
    glEnableVertexAttribArray(0);

    // 解绑
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void MyOpenGL::drawBottomImage(QPainter &painter)
{
    static QPixmap bottomPixmap(":/icon/equipBar.png");
    int imageWidth=360;
    int imageHeight=40;
    int posX = (width() - imageWidth) / 2;
    int posY = height() - imageHeight - 20;
    painter.drawPixmap(posX, posY, imageWidth, imageHeight, bottomPixmap);
    drawItemIcons(painter, posX, posY, imageWidth, imageHeight);
}

void MyOpenGL::drawItemIcons(QPainter &painter, int barX, int barY, int barWidth, int barHeight)
{
    int slotCount = 9;
    int slotSize = 30; // 每个格子的尺寸
    int slotSpacing = (barWidth - slotCount * slotSize) / (slotCount + 1);

    // 计算第一个格子的起始位置
    int startX = barX + slotSpacing;
    int startY = barY + (barHeight - slotSize) / 2;

    // 保存painter状态
    painter.save();

    // 设置抗锯齿
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    // 绘制每个物品
    for (int i = 0; i < qMin(slotCount, placeblock.size()); i++) {
        int slotX = startX + i * (slotSize + slotSpacing);

        // 绘制物品图标
        if (!placeblock[i].isNull()) {
            // 绘制图标背景
            painter.setBrush(QBrush(QColor(100, 100, 100, 50)));
            painter.setPen(Qt::NoPen);
            painter.drawRect(slotX, startY, slotSize, slotSize);

            // 缩放并绘制图标
            QPixmap scaledPixmap = placeblock[i].scaled(
                slotSize-4,slotSize-4,
                Qt::KeepAspectRatio, Qt::SmoothTransformation);

            int iconX = slotX + (slotSize - scaledPixmap.width()) / 2;
            int iconY = startY + (slotSize - scaledPixmap.height()) / 2;

            painter.drawPixmap(iconX, iconY, scaledPixmap);

            if(abs(curblock)==i){
                QPixmap pix(":/icon/equipSlot.png");
                painter.drawPixmap(barX+40*i,barY,40,40,pix);
            }
        }
    }
    painter.restore();
}

QVector3D MyOpenGL::getplacementlocation()
{
    QVector3D blockCenter=targetBlock+QVector3D(0.5,0.5,0.5);
    QVector3D diff=impacepoint-blockCenter;
    float absx=abs(diff.x());
    float absy=abs(diff.y());
    float absz=abs(diff.z());
    QVector3D normal;
    if(absx>absy&&absx>absz){
        normal=QVector3D(diff.x()>0?1:-1,0,0);
    }else if(absy>absz){
        normal=QVector3D(0,diff.y()>0?1:-1,0);
    }else{
        normal=QVector3D(0,0,diff.z()>0?1:-1);
    }
    return normal;
}

void MyOpenGL::initplaceblock()
{
    glGenVertexArrays(1,&placeblockVAO);
    glGenBuffers(1,&placeblockVBO);
    glGenBuffers(1,&placeblockinstVBO);
    glBindVertexArray(placeblockVAO);
    glBindBuffer(GL_ARRAY_BUFFER,placeblockVBO);
    glBufferData(GL_ARRAY_BUFFER,sizeof(cubeVertices),cubeVertices,GL_STATIC_DRAW);
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,8*sizeof(float),(void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,8*sizeof(float),(void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER,placeblockinstVBO);
    glBufferData(GL_ARRAY_BUFFER,0,nullptr,GL_DYNAMIC_DRAW);
    glVertexAttribPointer(2,3,GL_FLOAT,GL_FALSE,sizeof(TreeData),(void*)0);
    glEnableVertexAttribArray(2);
    glVertexAttribDivisor(2,1);
    glVertexAttribPointer(3,1,GL_FLOAT,GL_FALSE,sizeof(TreeData),(void*)(3*sizeof(float)));
    glEnableVertexAttribArray(3);
    glVertexAttribDivisor(3,1);
    glBindVertexArray(0);

    placeblockshader.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shader/placeblock.vert");
    placeblockshader.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shader/placeblock.frag");
    placeblockshader.link();
}

void MyOpenGL::updateplaceblockdata()
{
    glBindBuffer(GL_ARRAY_BUFFER,placeblockinstVBO);
    glBufferData(GL_ARRAY_BUFFER,placeblocklist.size()*sizeof(placttheblock),placeblocklist.data(),GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER,0);
}

void MyOpenGL::chunkaddblock(QVector3D pos)
{
    for(auto &it:chunkmap){
        if(pos.x()>=it->chunkx&&pos.x()<it->chunkx+WORLD_BLOCK&&pos.z()>=it->chunkz&&pos.z()<it->chunkz+WORLD_BLOCK){
            it->addblock(pos);
        }
    }
}
//111111111111111111
//11111111111111
