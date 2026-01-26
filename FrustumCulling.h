#ifndef FRUSTUMCULLING_H
#define FRUSTUMCULLING_H
#include<QVector3D>
#include<QMatrix4x4>
#include<QOpenGLFunctions_3_3_Core>
#include<QOpenGLWidget>
#include<QRandomGenerator>
#include<QList>
#include"vertes.h"
#include"createworld.h"
class Plane {//平面类
public:
    Plane(){}
    Plane(float a,float b,float c,float d):normal(QVector3D(a,b,c)),distance(d){}

    float getSignedDistance(const QVector3D point){//计算点到平面的距离
        return QVector3D::dotProduct(normal,point)+distance;
    }
    void normalize(){        //单位化
        float magnitude=normal.length();
        if(magnitude>0){
            normal/=magnitude;
            distance/=magnitude;
        }
    }
private:
    QVector3D normal;//平面的法向量
    float distance;
};


class Frustum {//视锥体类
public:
    enum PlanePos{
        Frustum_NEAR,
        Frustum_FAR,
        Frustum_LEFT,
        Frustum_RIGHT,
        Frustum_COUNT
    };

    Frustum(){}
    void update(QMatrix4x4 projectionview){//更新裁剪面
        const float *m=projectionview.constData();
        plane[Frustum_NEAR]=Plane(m[3]+m[2],m[7]+m[6],m[11]+m[10],m[15]+m[14]);
        plane[Frustum_FAR]=Plane(m[3]-m[2],m[7]-m[6],m[11]-m[10],m[15]-m[14]);
        plane[Frustum_LEFT]=Plane(m[3]+m[0],m[7]+m[4],m[11]+m[8],m[15]+m[12]);
        plane[Frustum_RIGHT]=Plane(m[3]-m[0],m[7]-m[4],m[11]-m[8],m[15]-m[12]);
        for(int i=0;i<Frustum_COUNT;i++){
            plane[i].normalize();
        }
    }

    bool containsCube(QVector<QVector3D>pos){//判断地图区块是否在视锥体内
        for(int i=0;i<pos.size();i++){
            bool isok=1;
            for(int j=0;j<Frustum_COUNT;j++){
                if(plane[j].getSignedDistance(pos[i])<0){
                    isok=0;
                }
            }
            if(isok==1){
                return true;
            }
        }
        return false;
    }
private:
    Plane plane[Frustum_COUNT];
};

struct InstanceData {
    float posx,posy,posz;
    float faceType;
    float blockType;

    bool operator==(const InstanceData &other){
        return this->posx==other.posx&&this->posy==other.posy&&this->posz==other.posz&&this->blockType==other.blockType&&this->faceType==other.faceType;
    }
};

struct TreeData{
    float x,y,z;
    float type;
};
class Chunk :public QOpenGLFunctions_3_3_Core{//区块类
public:
    Chunk(int x,int z):chunkx(x),chunkz(z){
        //initializeOpenGLFunctions();
        createworld=new CreateWorld();

        createworld->createterrain(chunkx,chunkz);

        //initchunk();

        //calculateVertices();//更新顶点坐标

        //generateInstancedData();
    }

    ~Chunk(){
        delete createworld;
        clearchunk();
    }

    void clearchunk(){

        if(VAO)glDeleteVertexArrays(1,&VAO);
        if(VBO)glDeleteBuffers(1,&VBO);
        if(instanceVBO)glDeleteBuffers(1,&instanceVBO);
        if(treeVAO)glDeleteVertexArrays(1,&treeVAO);
        if(treeVBO)glDeleteBuffers(1,&treeVBO);
        if(treeinstanceVBO)glDeleteBuffers(1,&treeinstanceVBO);
        if(flowerVAO)glDeleteVertexArrays(1,&flowerVAO);
        if(flowerVBO)glDeleteBuffers(1,&flowerVBO);
        if(flowerinstanceVBO)glDeleteBuffers(1,&flowerinstanceVBO);
    }

    void initchunk(){//初始化
        allterrainmap.clear();
        allterrainmap.resize(WORLD_BLOCK);
        boolterrainmap.resize(WORLD_BLOCK);
        for(int i=0;i<WORLD_BLOCK;i++){
            allterrainmap[i].resize(WORLD_HEIGHT);
            boolterrainmap[i].resize(WORLD_HEIGHT);
            for(int j=0;j<WORLD_HEIGHT;j++){
                allterrainmap[i][j].resize(WORLD_BLOCK,false);
                boolterrainmap[i][j].resize(WORLD_BLOCK,false);
            }
        }

        initializeOpenGLFunctions();
        clearchunk();
        glGenVertexArrays(1,&VAO);
        glGenBuffers(1,&VBO);
        glGenBuffers(1,&instanceVBO);
        glBindVertexArray(VAO);
        static const float faceVertices[] = {
            -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
            0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
            0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
            0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
            -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
            -0.5f, -0.5f,  0.5f,  0.0f, 0.0f
        };
        glBindBuffer(GL_ARRAY_BUFFER,VBO);
        glBufferData(GL_ARRAY_BUFFER,sizeof(faceVertices),faceVertices,GL_STATIC_DRAW);
        glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,5*sizeof(float),(void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,5*sizeof(float),(void*)(3*sizeof(float)));
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER,instanceVBO);
        glBufferData(GL_ARRAY_BUFFER,0,nullptr,GL_DYNAMIC_DRAW);
        glVertexAttribPointer(2,3,GL_FLOAT,GL_FALSE,sizeof(InstanceData),(void*)0);
        glEnableVertexAttribArray(2);
        glVertexAttribDivisor(2,1);
        glVertexAttribPointer(3,1,GL_FLOAT,GL_FALSE,sizeof(InstanceData),(void*)(3*sizeof(float)));
        glEnableVertexAttribArray(3);
        glVertexAttribDivisor(3,1);
        glVertexAttribPointer(4,1,GL_FLOAT,GL_FALSE,sizeof(InstanceData),(void*)(4*sizeof(float)));
        glEnableVertexAttribArray(4);
        glVertexAttribDivisor(4,1);
        glBindVertexArray(0);
        //初始化树的VAO、VBO
        glGenVertexArrays(1,&treeVAO);
        glGenBuffers(1,&treeVBO);
        glGenBuffers(1,&treeinstanceVBO);
        glBindVertexArray(treeVAO);
        glBindBuffer(GL_ARRAY_BUFFER,treeVBO);
        glBufferData(GL_ARRAY_BUFFER,sizeof(cubeVertices),cubeVertices,GL_STATIC_DRAW);
        glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,8*sizeof(float),(void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,8*sizeof(float),(void*)(3*sizeof(float)));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2,3,GL_FLOAT,GL_FALSE,8*sizeof(float),(void*)(5*sizeof(float)));
        glEnableVertexAttribArray(2);
        glBindBuffer(GL_ARRAY_BUFFER,treeinstanceVBO);
        glBufferData(GL_ARRAY_BUFFER,0,nullptr,GL_DYNAMIC_DRAW);
        glVertexAttribPointer(3,3,GL_FLOAT,GL_FALSE,sizeof(TreeData),(void*)0);
        glEnableVertexAttribArray(3);
        glVertexAttribDivisor(3,1);
        glVertexAttribPointer(4,1,GL_FLOAT,GL_FALSE,sizeof(TreeData),(void*)(3*sizeof(float)));
        glEnableVertexAttribArray(4);
        glVertexAttribDivisor(4,1);
        glBindVertexArray(0);
        //初始化花的VAO、VBO
        glGenVertexArrays(1,&flowerVAO);
        glGenBuffers(1,&flowerVBO);
        glGenBuffers(1,&flowerinstanceVBO);
        glBindVertexArray(flowerVAO);
        glBindBuffer(GL_ARRAY_BUFFER,flowerVBO);
        glBufferData(GL_ARRAY_BUFFER,sizeof(flowerVertices),flowerVertices,GL_STATIC_DRAW);
        glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,5*sizeof(float),(void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,5*sizeof(float),(void*)(3*sizeof(float)));
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER,flowerinstanceVBO);
        glBufferData(GL_ARRAY_BUFFER,0,nullptr,GL_DYNAMIC_DRAW);
        glVertexAttribPointer(2,3,GL_FLOAT,GL_FALSE,sizeof(TreeData),(void*)0);
        glEnableVertexAttribArray(2);
        glVertexAttribDivisor(2,1);
        glVertexAttribPointer(3,1,GL_FLOAT,GL_FALSE,sizeof(TreeData),(void*)(3*sizeof(float)));
        glEnableVertexAttribArray(3);
        glVertexAttribDivisor(3,1);
        glBindVertexArray(0);
    }
    //更新实例化的数据
    void generateInstancedData(){
        instanceData.clear();
        for(int x=chunkx;x<chunkx+WORLD_BLOCK;x++){
            for(int z=chunkz;z<chunkz+WORLD_BLOCK;z++){
                int height=createworld->terrainmap[x-chunkx][z-chunkz];
                for(int y=0;y<=height;y++){
                    checkAndAddFaces(x,y,z,height);
                }
            }
        }
        uploadInstanceData();
    }

    //绘制区块
    void drawchunk(){
        if(VAO==0||instanceData.size()==0||!visible||VBO==0||instanceVBO==0){
            return;
        }
        glBindVertexArray(VAO);
        glDrawArraysInstanced(GL_TRIANGLES,0,6,instanceData.size());
        glBindVertexArray(0);
    }

    void drawtree(){//绘制区块中的树
        if(treeVAO==0||treedata.size()==0||!visible||treeVBO==0||treeinstanceVBO==0){
            return;
        }
        glBindVertexArray(treeVAO);
        glDrawArraysInstanced(GL_TRIANGLES,0,36,treedata.size());
        glBindVertexArray(0);
    }

    void drawflower(){//绘制花和草
        if(flowerVAO==0||flowerdata.size()==0||!visible||flowerVBO==0||flowerinstanceVBO==0){
            return;
        }
        glBindVertexArray(flowerVAO);
        glDrawArraysInstanced(GL_TRIANGLES,0,12,flowerdata.size());
        glBindVertexArray(0);
    }


    //检查要绘制哪些面
    void checkAndAddFaces(int x,int y,int z,int height){
        allterrainmap[x-chunkx][y][z-chunkz]=true;
        boolterrainmap[x-chunkx][y][z-chunkz]=true;
        if (y==height){
            float p=getdensity(x,y,z);
            addInstance(x,y,z,4.0,p);
            addInstance(x,y,z,0.0,p);
            addInstance(x,y,z,1.0,p);
            addInstance(x,y,z,2.0,p);
            addInstance(x,y,z,3.0,p);
        }
        if (y==0){
            addInstance(x,y,z,5.0,2.0);
        }
        if (z+1-chunkz<WORLD_BLOCK&&y>createworld->terrainmap[x-chunkx][z+1-chunkz]){
            float p=getdensity(x,y,z);
            addInstance(x,y,z,0.0,p);
        }
        if (z-chunkz>0&&y>createworld->terrainmap[x-chunkx][z-1-chunkz]){
            float p=getdensity(x,y,z);
            addInstance(x,y,z,1.0,p);
        }
        if (x-chunkx>0&&y>createworld->terrainmap[x-1-chunkx][z-chunkz]){
            float p=getdensity(x,y,z);
            addInstance(x,y,z,2.0,p);
        }
        if (x+1-chunkx<WORLD_BLOCK&&y>createworld->terrainmap[x+1-chunkx][z-chunkz]){
            float p=getdensity(x,y,z);
            addInstance(x,y,z,3.0,p);
        }
        if(z-chunkz==WORLD_BLOCK-1||x-chunkx==WORLD_BLOCK-1||x-chunkx==0||z-chunkz==0){
            float p=getdensity(x,y,z);
            addInstance(x,y,z,0.0,p);
            addInstance(x,y,z,1.0,p);
            addInstance(x,y,z,2.0,p);
            addInstance(x,y,z,3.0,p);
        }
        if(height<28){
            addInstance(x,28,z,6.0,0.0);
        }
    }

    void addInstance(float x,float y,float z,float faceType,float blockType){
        InstanceData ins;
        ins.posx=x;
        ins.posy=y;
        ins.posz=z;
        ins.faceType=faceType;
        ins.blockType=blockType;
        instanceData.append(ins);
    }

    //实例数据上传到GPU
    void uploadInstanceData(){
        if(instanceData.isEmpty()){
            qDebug()<<"数据为空"<<'\n';
            return;
        }
        glBindBuffer(GL_ARRAY_BUFFER,instanceVBO);
        glBufferData(GL_ARRAY_BUFFER,instanceData.size()*sizeof(InstanceData),instanceData.data(),GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER,0);
        //上传树的信息
        glBindBuffer(GL_ARRAY_BUFFER,treeinstanceVBO);
        glBufferData(GL_ARRAY_BUFFER,treedata.size()*sizeof(TreeData),treedata.data(),GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER,0);
        //上传花的信息
        glBindBuffer(GL_ARRAY_BUFFER,flowerinstanceVBO);
        glBufferData(GL_ARRAY_BUFFER,flowerdata.size()*sizeof(TreeData),flowerdata.data(),GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER,0);
    }


    void calculateVertices(){//计算区块顶点坐标
        pos.clear();
        pos.push_back(QVector3D(chunkx,0,chunkz));
        pos.push_back(QVector3D(chunkx+WORLD_BLOCK,0,chunkz));
        pos.push_back(QVector3D(chunkx,0,chunkz+WORLD_BLOCK));
        pos.push_back(QVector3D(chunkx+WORLD_BLOCK,0,chunkz+WORLD_BLOCK));
    }

    bool canPlaceTreeAt(float x,float z){//检测当前位置是否合适生成树
        if(treepos.size()==0){
            return true;
        }
        if(x<2||x>WORLD_BLOCK-3||z<2||z>WORLD_BLOCK-3){
            return false;
        }
        for(auto &it:treepos){
            int d=std::max(abs(x-it.x),abs(z-it.z));
            if(d>=4){
                return true;
            }
        }
        return false;
    }

    bool canPlaceFlowerAt(float x,float z){//检测当前位置是否有花
        for(auto &it:flowerdata){
            if(it.x==x&&it.z==z){
                return false;
            }
        }
        return true;
    }

    void generateTreesInChunk(){//在区块生成有树和花的位置
        int num=QRandomGenerator::global()->bounded(0,100);
        if(num>=50){
            int treeCount=QRandomGenerator::global()->bounded(0,3);
            //treeCount=1;
            for(int i=0;i<treeCount;i++){
                int x=chunkx+QRandomGenerator::global()->bounded(2,WORLD_BLOCK-3);
                int z=chunkz+QRandomGenerator::global()->bounded(2,WORLD_BLOCK-3);
                float fx=x,fy=createworld->terrainmap[x-chunkx][z-chunkz],fz=z;
                if(createworld->terrainmap[x-chunkx][z-chunkz]<28)continue;
                //treepos.push_back({fx,fy,fz,0});
                if(canPlaceTreeAt(fx,fz)){
                    treepos.push_back({fx,fy,fz,0});
                }
            }
        }

        int flowernum=QRandomGenerator::global()->bounded(0,100);
        if(flowernum>=40){
            int flowerCount=QRandomGenerator::global()->bounded(4,40);
            for(int i=0;i<flowerCount;i++){
                int x=chunkx+QRandomGenerator::global()->bounded(0,WORLD_BLOCK);
                int z=chunkz+QRandomGenerator::global()->bounded(0,WORLD_BLOCK);
                if(createworld->terrainmap[x-chunkx][z-chunkz]<28)continue;
                int y=createworld->terrainmap[x-chunkx][z-chunkz];
                float fx=x,fy=y+0.4f,fz=z;
                if(canPlaceFlowerAt(x,z)){//检测当前位置是否有花
                    flowerdata.push_back({fx,fy,fz,1});
                    allterrainmap[x-chunkx][y+1][z-chunkz]=true;
                }
            }
        }

        // int grassnum=QRandomGenerator::global()->bounded(0,100);
        // if(grassnum<10)return;
        int grassCount=QRandomGenerator::global()->bounded(10,100);
        for(int i=0;i<grassCount;i++){
            int x=chunkx+QRandomGenerator::global()->bounded(0,WORLD_BLOCK);
            int z=chunkz+QRandomGenerator::global()->bounded(0,WORLD_BLOCK);
            if(createworld->terrainmap[x-chunkx][z-chunkz]<28)continue;
            int y=createworld->terrainmap[x-chunkx][z-chunkz];
            float fx=x,fy=y+0.4f,fz=z;
            if(canPlaceFlowerAt(x,z)){//检测当前位置是否有花草
                flowerdata.push_back({fx,fy,fz,0});
                allterrainmap[x-chunkx][y+1][z-chunkz]=true;
            }
        }
    }

    void settreedata(){//根据树的形状设置需要绘制的点
        if(treepos.size()==0){
            return;
        }
        for(auto &it:treepos){
            for(int i=1;i<=6;i++){
                treedata.push_back({it.x,it.y+i,it.z,0});
                if(it.x-chunkx<0||it.x-chunkx>=WORLD_BLOCK||it.z-chunkz<0||it.z-chunkz>=WORLD_BLOCK){
                    qDebug()<<"数据不符合"<<'\n';
                }
                allterrainmap[it.x-chunkx][it.y+i][it.z-chunkz]=true;
                boolterrainmap[it.x-chunkx][it.y+i][it.z-chunkz]=true;
            }
            treedata.push_back({it.x,it.y+7,it.z,1});
            allterrainmap[it.x-chunkx][it.y+7][it.z-chunkz]=true;
            treedata.push_back({it.x-1,it.y+7,it.z,1});
            allterrainmap[it.x-1-chunkx][it.y+7][it.z-chunkz]=true;
            treedata.push_back({it.x,it.y+7,it.z-1,1});
            allterrainmap[it.x-chunkx][it.y+7][it.z-1-chunkz]=true;
            treedata.push_back({it.x+1,it.y+7,it.z,1});
            allterrainmap[it.x+1-chunkx][it.y+7][it.z-chunkz]=true;
            treedata.push_back({it.x,it.y+7,it.z+1,1});
            allterrainmap[it.x-chunkx][it.y+7][it.z+1-chunkz]=true;
            treedata.push_back({it.x-1,it.y+6,it.z,1});
            allterrainmap[it.x-1-chunkx][it.y+6][it.z-chunkz]=true;
            treedata.push_back({it.x,it.y+6,it.z-1,1});
            allterrainmap[it.x-chunkx][it.y+6][it.z-1-chunkz]=true;
            treedata.push_back({it.x+1,it.y+6,it.z,1});
            allterrainmap[it.x+1-chunkx][it.y+6][it.z-chunkz]=true;
            treedata.push_back({it.x,it.y+6,it.z+1,1});
            allterrainmap[it.x-chunkx][it.y+6][it.z+1-chunkz]=true;
            treedata.push_back({it.x+1,it.y+6,it.z-1,1});
            allterrainmap[it.x+1-chunkx][it.y+6][it.z-1-chunkz]=true;
            treedata.push_back({it.x-1,it.y+6,it.z+1,1});
            allterrainmap[it.x-1-chunkx][it.y+6][it.z+1-chunkz]=true;
            treedata.push_back({it.x+1,it.y+6,it.z+1,1});
            allterrainmap[it.x+1-chunkx][it.y+6][it.z+1-chunkz]=true;
            treedata.push_back({it.x-1,it.y+6,it.z-1,1});
            allterrainmap[it.x-1-chunkx][it.y+6][it.z-1-chunkz]=true;
            for(int i=4;i<=5;i++){
                for(int j=-2;j<=2;j++){
                    for(int k=-2;k<=2;k++){
                        if(j==0&&k==0)continue;
                        treedata.push_back({it.x+j,it.y+i,it.z+k,1});
                        allterrainmap[it.x+j-chunkx][it.y+i][it.z+k-chunkz]=true;
                    }
                }
            }
        }
    }

    void eliminateblock(QVector3D blockpos){
        qDebug()<<"进入"<<'\n';
        bool isok=false;
        for(int i=0;i<flowerdata.size();i++){
            if(flowerdata[i].x==blockpos.x()&&flowerdata[i].y==blockpos.y()-0.6f&&flowerdata[i].z==blockpos.z()){
                flowerdata.removeAt(i);
                allterrainmap[blockpos.x()-chunkx][blockpos.y()][blockpos.z()-chunkz]=false;
                qDebug()<<"删除"<<'\n';
                isok=true;
                break;
            }
        }
        if(isok==false){
            for(int i=0;i<treedata.size();i++){
                if(treedata[i].x==blockpos.x()&&treedata[i].y==blockpos.y()&&treedata[i].z==blockpos.z()){
                    treedata.removeAt(i);
                    allterrainmap[blockpos.x()-chunkx][blockpos.y()][blockpos.z()-chunkz]=false;
                    boolterrainmap[blockpos.x()-chunkx][blockpos.y()][blockpos.z()-chunkz]=false;
                    isok=true;
                    break;
                }
            }
        }
        if(isok==false){
            QVector<QVector3D>deletepos;
            for(int i=instanceData.size()-1;i>=0;i--){
                if(instanceData[i].posx==blockpos.x()&&instanceData[i].posy==blockpos.y()&&instanceData[i].posz==blockpos.z()){
                    deletepos.append({instanceData[i].posx,instanceData[i].posy,instanceData[i].posz});
                    instanceData.removeAt(i);
                    boolterrainmap[blockpos.x()-chunkx][blockpos.y()][blockpos.z()-chunkz]=false;
                    allterrainmap[blockpos.x()-chunkx][blockpos.y()][blockpos.z()-chunkz]=false;
                }
            }
            for(int i=0;i<deletepos.size();i++){
                if(allterrainmap[deletepos[i].x()-chunkx][deletepos[i].y()+1][deletepos[i].z()-chunkz]){
                    for(int i=0;i<flowerdata.size();i++){
                        if(flowerdata[i].x==deletepos[i].x()&&flowerdata[i].y==deletepos[i].y()+1.0f-0.6f&&flowerdata[i].z==deletepos[i].z()){
                            flowerdata.removeAt(i);
                            allterrainmap[deletepos[i].x()-chunkx][deletepos[i].y()+1][deletepos[i].z()-chunkz]=false;
                            break;
                        }
                    }
                }
                if(deletepos[i].y()-1>=0&&boolterrainmap[deletepos[i].x()-chunkx][deletepos[i].y()-1][deletepos[i].z()-chunkz]){
                    float p=getdensity(deletepos[i].x(),deletepos[i].y()-1,deletepos[i].z());
                    if(!checkinstanceData(deletepos[i].x(),deletepos[i].y()-1,deletepos[i].z(),4.0,p)){
                        instanceData.append({deletepos[i].x(),deletepos[i].y()-1,deletepos[i].z(),4.0,p});
                    }
                }
                if(boolterrainmap[deletepos[i].x()-chunkx][deletepos[i].y()+1][deletepos[i].z()-chunkz]){
                    float p=getdensity(deletepos[i].x(),deletepos[i].y()+1,deletepos[i].z());
                    if(!checkinstanceData(deletepos[i].x(),deletepos[i].y()+1,deletepos[i].z(),5.0,p)){
                        instanceData.append({deletepos[i].x(),deletepos[i].y()+1,deletepos[i].z(),5.0,p});
                    }
                }
                if(deletepos[i].x()-chunkx+1<WORLD_BLOCK&&boolterrainmap[deletepos[i].x()-chunkx+1][deletepos[i].y()][deletepos[i].z()-chunkz]){
                    float p=getdensity(deletepos[i].x()+1,deletepos[i].y(),deletepos[i].z());
                    if(!checkinstanceData(deletepos[i].x()+1,deletepos[i].y(),deletepos[i].z(),2.0,p)){
                        instanceData.append({deletepos[i].x()+1,deletepos[i].y(),deletepos[i].z(),2.0,p});
                    }
                }
                if(deletepos[i].x()-chunkx-1>=0&&boolterrainmap[deletepos[i].x()-chunkx-1][deletepos[i].y()][deletepos[i].z()-chunkz]){
                    float p=getdensity(deletepos[i].x()-1,deletepos[i].y(),deletepos[i].z());
                    if(!checkinstanceData(deletepos[i].x()-1,deletepos[i].y(),deletepos[i].z(),3.0,p)){
                        instanceData.append({deletepos[i].x()-1,deletepos[i].y(),deletepos[i].z(),3.0,p});
                    }
                }
                if(deletepos[i].z()-chunkz-1>=0&&boolterrainmap[deletepos[i].x()-chunkx][deletepos[i].y()][deletepos[i].z()-chunkz-1]){
                    float p=getdensity(deletepos[i].x(),deletepos[i].y(),deletepos[i].z()-1);
                    if(!checkinstanceData(deletepos[i].x(),deletepos[i].y(),deletepos[i].z()-1,0.0,p)){
                        instanceData.append({deletepos[i].x(),deletepos[i].y(),deletepos[i].z()-1,0.0,p});
                    }
                }
                if(deletepos[i].z()-chunkz+1<WORLD_BLOCK&&boolterrainmap[deletepos[i].x()-chunkx][deletepos[i].y()][deletepos[i].z()-chunkz+1]){
                    float p=getdensity(deletepos[i].x(),deletepos[i].y(),deletepos[i].z()+1);
                    if(!checkinstanceData(deletepos[i].x(),deletepos[i].y(),deletepos[i].z()+1,1.0,p)){
                        instanceData.append({deletepos[i].x(),deletepos[i].y(),deletepos[i].z()+1,1.0,p});
                    }
                }
            }
        }
        uploadInstanceData();
    }

    float getdensity(int x,int y,int z){
        if(createworld->densityMap[x-chunkx][y][z-chunkz]<=0.3){
            return 0.0;
        }
        if(createworld->densityMap[x-chunkx][y][z-chunkz]>0.3&&createworld->densityMap[x-chunkx][y][z-chunkz]<=0.7){
            if(y==createworld->terrainmap[x-chunkx][z-chunkz]){
                if(y<28){
                    return 3.0;
                }else{
                    return 1.0;
                }
            }
            return 3.0;
        }
        if(createworld->densityMap[x-chunkx][y][z-chunkz]>0.7){
            return 2.0;
        }
        return 0.0;
    }

    bool checkinstanceData(float x,float y,float z,float type1,float type2){
        InstanceData ins{x,y,z,type1,type2};
        for(int i=0;i<instanceData.size();i++){
            if(ins==instanceData[i]){
                return true;
            }
        }
        return false;
    }

    void addblock(QVector3D pos){
        allterrainmap[pos.x()-chunkx][pos.y()][pos.z()-chunkz]=true;
        boolterrainmap[pos.x()-chunkx][pos.y()][pos.z()-chunkz]=true;
    }

    void deleteblock(QVector3D pos){
        allterrainmap[pos.x()-chunkx][pos.y()][pos.z()-chunkz]=false;
        boolterrainmap[pos.x()-chunkx][pos.y()][pos.z()-chunkz]=false;
    }

    QVector<QVector3D> getpos(){
        return pos;
    }

    bool visible=false;
    bool isinit=false;
    int chunkx,chunkz;
    CreateWorld *createworld=NULL;
    QVector<QVector<QVector<bool>>> allterrainmap;//记录哪些方块可以被选中
    QVector<QVector<QVector<bool>>>boolterrainmap;//用于碰撞检测


private:
    unsigned int VAO,VBO,instanceVBO;
    unsigned int treeVAO,treeVBO,treeinstanceVBO;
    unsigned int flowerVAO,flowerVBO,flowerinstanceVBO;
    QList<InstanceData>instanceData;
    QVector<QVector3D>waterinstanceData;
    QVector<QVector3D>pos;
    QVector<TreeData>treepos;
    QList<TreeData>treedata;
    QList<TreeData>flowerdata;
};

#endif // FRUSTUMCULLING_H
