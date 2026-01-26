#ifndef CREATEWORLD_H
#define CREATEWORLD_H

#include <QObject>
#include<QVector>



class CreateWorld : public QObject
{
    Q_OBJECT
public:
    explicit CreateWorld(QObject *parent = nullptr);

public:
    //static CreateWorld* Instance();
    void createterrain(int mapx,int mapz);

private:
    void applyThermalErosion(int iterations);
    void addRiverValleys(int mapx,int mapz);
    void addMountainRanges(int mapx,int mapz);
    void generate3DDensity(int mapx, int mapz);
    float calculateSlope(int x, int z);

public:
    QVector<QVector<int>>terrainmap;
    QVector<QVector<QVector<float>>> densityMap;
    QVector<QVector<float>> riverNoise;
signals:
};

#endif // CREATEWORLD_H
