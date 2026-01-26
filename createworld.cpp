#include "createworld.h"
#include"noise.h"
#include"vertes.h"
CreateWorld::CreateWorld(QObject *parent)
    : QObject{parent}
{

}

void CreateWorld::createterrain(int mapx,int mapz)//生成地形高度
{
    seed(12345);
    terrainmap.clear();
    terrainmap.resize(WORLD_BLOCK, QVector<int>(WORLD_BLOCK, 0));

    for (int x=mapx;x<mapx+WORLD_BLOCK;x++) {
        for (int z=mapz;z<mapz+WORLD_BLOCK;z++) {
            float base = simplex2(x * 0.0005f, z * 0.0005f, 4, 0.5f, 2.0f);
            float mountains = simplex2(x * 0.002f, z * 0.002f, 6, 0.5f, 2.2f);
            float hills = simplex2(x * 0.008f, z * 0.008f, 4, 0.4f, 2.5f);
            float detail = simplex2(x * 0.03f, z * 0.03f, 2, 0.3f, 3.0f);
            float height = base * 0.5f +
                           mountains * 0.3f +
                           hills * 0.15f +
                           detail * 0.05f;
            height = powf(height, 1.5f);
            terrainmap[x-mapx][z-mapz] = (int)(height * WORLD_HEIGHT * 0.7f) +
                               (WORLD_HEIGHT * 0.15f);
        }
    }
    addMountainRanges(mapx,mapz);
    //applyThermalErosion(3);
    addRiverValleys(mapx,mapz);
    generate3DDensity(mapx,mapz);
}

void CreateWorld::applyThermalErosion(int iterations)
{
    QVector<QVector<float>> tempMap;
    tempMap.resize(WORLD_WIDTH, QVector<float>(WORLD_WIDTH, 0));

    // 转换为浮点数便于计算
    for (int x = 0; x < WORLD_WIDTH; x++) {
        for (int z = 0; z < WORLD_WIDTH; z++) {
            tempMap[x][z] = terrainmap[x][z];
        }
    }

    for (int iter = 0; iter < iterations; iter++) {
        auto newMap = tempMap;

        for (int x = 1; x < WORLD_WIDTH - 1; x++) {
            for (int z = 1; z < WORLD_WIDTH - 1; z++) {
                float current = tempMap[x][z];
                float totalDiff = 0;
                float avgNeighbor = 0;
                int neighborCount = 0;

                // 检查8个邻居
                for (int dx = -1; dx <= 1; dx++) {
                    for (int dz = -1; dz <= 1; dz++) {
                        if (dx == 0 && dz == 0) continue;

                        float neighbor = tempMap[x + dx][z + dz];
                        if (neighbor < current) {
                            totalDiff += current - neighbor;
                            avgNeighbor += neighbor;
                            neighborCount++;
                        }
                    }
                }

                if (neighborCount > 0 && totalDiff > 1.0f) {
                    avgNeighbor /= neighborCount;
                    float transfer = totalDiff * 0.1f; // 转移量因子
                    newMap[x][z] -= transfer;

                    // 重新分配到较低邻居
                    for (int dx = -1; dx <= 1; dx++) {
                        for (int dz = -1; dz <= 1; dz++) {
                            if (dx == 0 && dz == 0) continue;

                            int nx = x + dx;
                            int nz = z + dz;
                            if (tempMap[nx][nz] < current) {
                                float ratio = (current - tempMap[nx][nz]) / totalDiff;
                                newMap[nx][nz] += transfer * ratio;
                            }
                        }
                    }
                }
            }
        }
        tempMap = newMap;
    }

    // 转换回整数
    for (int x = 0; x < WORLD_WIDTH; x++) {
        for (int z = 0; z < WORLD_WIDTH; z++) {
            terrainmap[x][z] = (int)tempMap[x][z];
        }
    }
}

void CreateWorld::addRiverValleys(int mapx,int mapz)
{
    riverNoise.clear();
    riverNoise.resize(WORLD_BLOCK, QVector<float>(WORLD_BLOCK, 0));

    seed(54321); // 使用不同的种子

    // 生成多层河流噪声
    for (int x = mapx; x < mapx+WORLD_BLOCK; x++) {
        for (int z = mapz; z < mapz+WORLD_BLOCK; z++) {
            // 主河道噪声 - 使用中频，避免太宽或太窄
            float mainRiver = simplex2(x * 0.0003f, z * 0.0003f, 3, 0.5f, 2.0f);

            // 蜿蜒细节噪声
            float meander = simplex2(x * 0.0008f, z * 0.0008f, 2, 0.4f, 2.5f);

            // 河床细节噪声
            float detail = simplex2(x * 0.005f, z * 0.005f, 3, 0.3f, 2.2f);

            // 合并噪声
            riverNoise[x-mapx][z-mapz] = mainRiver * 0.7f + meander * 0.2f + detail * 0.1f;
        }
    }

    // 应用河流效果
    for (int x = mapx; x < mapx+WORLD_BLOCK; x++) {
        for (int z = mapz; z < mapz+WORLD_BLOCK; z++) {
            // 找出河流位置（使用梯度变化，不是简单的阈值）
            float riverDist = fabs(riverNoise[x-mapx][z-mapz] - 0.5f);

            // 河流宽度控制：0.03f比0.05f窄，比0.01f宽
            float riverWidth = 0.01f;

            if (riverDist < riverWidth) {
                // 计算到河中心的距离比例 (0-1)，0表示河中心，1表示河边
                float distanceFromCenter = riverDist / riverWidth;

                // 使用曲线函数创建从岸边到河心逐渐加深的效果
                // 平方曲线：岸边浅，河心深
                float depthFactor = 1.0f - distanceFromCenter * distanceFromCenter;

                // 基础深度 + 河床噪声变化
                float baseDepth = 10.0f; // 河心最大深度
                float riverbedNoise = simplex2(x * 0.02f, z * 0.02f, 2, 0.5f, 2.5f);

                // 河床起伏：-3到+3的变化
                float riverbedVariation = (riverbedNoise - 0.5f) * 6.0f;

                // 计算最终深度：基础深度 × 深度因子 + 河床起伏
                float depth = baseDepth * depthFactor + riverbedVariation;

                // 确保最小深度为1
                depth = std::max(1.0f, depth);

                // 应用河谷深度
                terrainmap[x-mapx][z-mapz] = std::max(1, terrainmap[x-mapx][z-mapz] - (int)depth);

                // 河岸平滑（在外部函数中统一处理）
            }
        }
    }
}



void CreateWorld::addMountainRanges(int mapx,int mapz)
{
    seed(12345* 2 + 1234);

    // 1. 生成山脉走向（脊线）
    QVector<QVector<float>> mountainRidge;
    mountainRidge.resize(WORLD_BLOCK,QVector<float>(WORLD_BLOCK,0));

    for (int x=mapx;x<mapx+WORLD_BLOCK;x++) {
        for (int z=mapz;z<mapz+WORLD_BLOCK;z++) {
            // 使用Worley噪声生成山脉脊线
            float nx =x*0.0003f;
            float nz =z*0.0003f;

            // 主山脉方向
            float ridge1 = fabs(simplex2(nx, nz, 1, 0, 0));

            // 次级山脉方向
            float ridge2 = fabs(simplex2(nx * 1.7f, nz * 1.7f, 1, 0, 0));

            // 合并脊线
            mountainRidge[x-mapx][z-mapz] = ridge1 * 0.7f + ridge2 * 0.3f;
        }
    }

    // 2. 山脉高度场
    QVector<QVector<float>> mountainHeight;
    mountainHeight.resize(WORLD_BLOCK, QVector<float>(WORLD_BLOCK, 0));

    for (int x = mapx; x < mapx+WORLD_BLOCK; x++) {
        for (int z = mapz; z < mapz+WORLD_BLOCK; z++) {
            // 高频细节噪声
            float nx = x * 0.005f;
            float nz = z * 0.005f;

            // 多层噪声叠加创建复杂山脉
            float height = 0;
            float amplitude = 1.0f;
            float frequency = 1.0f;
            float maxAmplitude = 0;

            // 分形噪声
            for (int i = 0; i < 6; i++) {
                float noise = simplex2(nx * frequency, nz * frequency, 1, 0, 0);
                noise = 1.0f - fabs(noise); // 创建尖锐的山峰

                height += noise * amplitude;
                maxAmplitude += amplitude;
                amplitude *= 0.5f;
                frequency *= 2.0f;
            }

            height = height / maxAmplitude;

            // 应用脊线掩码
            height *= mountainRidge[x-mapx][z-mapz];
            mountainHeight[x-mapx][z-mapz] = height;
        }
    }

    // 3. 山脚过渡
    QVector<QVector<float>> mountainMask;
    mountainMask.resize(WORLD_BLOCK, QVector<float>(WORLD_BLOCK, 0));

    for (int x = mapx; x < mapx+WORLD_BLOCK; x++) {
        for (int z = mapz; z < mapz+WORLD_BLOCK; z++) {

            // 使用多个圆圈定义山脉区域
            float mask = 0;

            // 生成几个主要山脉区域
            for (int i = 0; i < 5; i++) {
                float centerX = simplex2(123.45f + i * 100, 0, 1, 0, 0) * WORLD_BLOCK;
                float centerZ = simplex2(0, 678.90f + i * 100, 1, 0, 0) * WORLD_BLOCK;
                float radius = 50.0f + simplex2(i * 50, i * 50, 1, 0, 0) * 100.0f;

                float dx = x - centerX;
                float dz = z - centerZ;
                float distance = sqrt(dx * dx + dz * dz);

                if (distance < radius) {
                    float falloff = 1.0f - (distance / radius);
                    mask = std::max(mask, falloff * falloff);
                }
            }

            mountainMask[x-mapx][z-mapz] = mask;
        }
    }

    // 4. 应用山脉到地形
    for (int x = mapx; x < mapx+WORLD_BLOCK; x++) {
        for (int z = mapz; z < mapz+WORLD_BLOCK; z++) {
            float mask = mountainMask[x-mapx][z-mapz];

            if (mask > 0.01f) {
                float height = mountainHeight[x-mapx][z-mapz];

                // 应用指数曲线让山峰更陡峭
                height = powf(height, 1.8f);

                // 添加山脊效果
                float ridgeFactor = mountainRidge[x-mapx][z-mapz];
                if (ridgeFactor > 0.7f) {
                    height *= 1.5f; // 山脊更高
                }

                // 计算最终高度增加值
                float mountainAdd = height * mask * 80.0f; // 山脉高度

                // 平滑过渡边缘
                if (mask < 0.3f) {
                    mountainAdd *= (mask / 0.3f);
                }

                // 添加到基础地形
                terrainmap[x-mapx][z-mapz] += (int)mountainAdd;

                // 确保不会超过世界高度限制
                terrainmap[x-mapx][z-mapz] = std::min(terrainmap[x-mapx][z-mapz], WORLD_HEIGHT - 1);
            }
        }
    }

}

void CreateWorld::generate3DDensity(int mapx, int mapz)
{
    densityMap.clear();
    densityMap.resize(WORLD_BLOCK);

    // 使用成员变量，不再重新计算
    // QVector<QVector<float>> riverNoise = riverNoiseMap; // 可以直接使用

    for (int x = 0; x < WORLD_BLOCK; x++) {
        densityMap[x].resize(WORLD_HEIGHT);
        for (int y = 0; y < WORLD_HEIGHT; y++) {
            densityMap[x][y].resize(WORLD_BLOCK);
            for (int z = 0; z < WORLD_BLOCK; z++) {
                float worldX = mapx + x;
                float worldY = y;
                float worldZ = mapz + z;

                int height = terrainmap[x][z];
                int depth = height - y;  // 注意：这里是关键

                // 基础密度分配
                float density = 0.0f;
                if (depth < 0) {
                    // 空气
                    density = 0.0f;
                }
                else if (depth == 0) {
                    // 地表 - 必须是泥土
                    density = 0.65f; // 泥土范围：0.3-0.7，所以0.65在中间偏岩石
                }
                else if (depth < 4) {
                    // 浅层泥土
                    density = 0.65f - depth * 0.05f;
                }
                else {
                    // 深层岩石
                    density = 0.5f + (depth / 30.0f) * 0.4f;
                    density = std::min(density, 0.95f);
                }

                // 河流检测
                bool isRiver = false;
                bool isRiverCenter = false;

                if (riverNoise.size() > x && riverNoise[x].size() > z) {
                    float riverDist = fabs(riverNoise[x][z] - 0.5f);
                    const float riverWidth = 0.015f; // 略微增加宽度

                    // 只在特定高度范围生成河流
                    if (riverDist < riverWidth && height >= 24 && height <= 30) {
                        isRiver = true;
                        isRiverCenter = riverDist < riverWidth * 0.3f;

                        // 调整河流密度
                        if (depth == 0) {
                            // 河床材质
                            if (isRiverCenter) {
                                density = 0.25f; // 沙子 (小于0.3)
                            } else {
                                density = 0.55f; // 河边泥土
                            }
                        } else if (depth <= 2) {
                            // 河床下层
                            density = 0.35f;
                        }

                        // ========== 新增：在河流中添加岩石块 ==========
                        // 条件1：河流较深的区域（深度大于3）有概率出现岩石
                        if (depth > 3 && depth < 10) {
                            // 使用3D噪声控制岩石分布
                            float rockNoise = simplex3(
                                worldX * 0.05f,
                                worldY * 0.1f,
                                worldZ * 0.05f,
                                2, 0.5f, 2.0f);

                            // 岩石出现的概率：深度越深，概率越高
                            float rockProbability = (depth - 3) / 7.0f * 0.4f; // 0-40%概率

                            // 使用噪声决定是否放置岩石
                            if (rockNoise > 0.6f + rockProbability) {
                                density = 0.75f + (rockNoise - 0.6f) * 0.2f; // 0.75-0.95
                            }
                        }

                        // 条件2：河流陡峭的岸边可能有岩石
                        if (depth == 1 || depth == 2) {
                            float slope = calculateSlope(x, z);
                            if (slope > 0.3f) { // 坡度较陡
                                float rockChance = simplex2(
                                    worldX * 0.02f,
                                    worldZ * 0.02f,
                                    1, 0, 0);

                                if (rockChance > 0.7f) { // 30%概率
                                    density = 0.8f + rockChance * 0.1f; // 0.8-0.9
                                }
                            }
                        }

                        // 条件3：河床底部的岩石层
                        if (depth >= 5 && depth <= 8) {
                            // 使用分形噪声创建岩石层
                            float rockLayerNoise = simplex2(
                                worldX * 0.008f,
                                worldZ * 0.008f,
                                3, 0.5f, 2.0f);

                            // 在河床底部创建不连续的岩石层
                            if (rockLayerNoise > 0.75f) { // 25%概率形成岩石层
                                // 岩石层的厚度由噪声控制
                                float rockThickness = simplex3(
                                    worldX * 0.03f,
                                    worldY * 0.02f,
                                    worldZ * 0.03f,
                                    2, 0.5f, 1.5f);

                                if (rockThickness > 0.6f) {
                                    density = 0.85f + (rockLayerNoise - 0.75f) * 0.3f; // 0.85-0.95
                                }
                            }
                        }
                    }
                }

                // 海滩检测
                bool isBeach = false;
                if (height >= 28 && height <= 30 && depth <= 2) {
                    // 世界边缘检测
                    const int edgeDistance = 50;
                    int worldAbsX = mapx + x;
                    int worldAbsZ = mapz + z;

                    if (worldAbsX < edgeDistance || worldAbsZ < edgeDistance ||
                        worldAbsX > WORLD_WIDTH - edgeDistance ||
                        worldAbsZ > WORLD_WIDTH - edgeDistance) {
                        isBeach = true;
                        if (depth == 0) {
                            density = 0.25f + (simplex2(worldX * 0.1f, worldZ * 0.1f, 1, 0, 0) * 0.1f);
                        }
                    }
                }

                // 添加3D噪声（轻微影响）
                float noise3D = simplex3(worldX * 0.02f, worldY * 0.02f, worldZ * 0.02f,
                                         2, 0.5f, 2.0f) * 0.05f;
                density += noise3D;

                // 确保地表材质
                if (depth == 0 && !isRiver && !isBeach) {
                    // 确保地表是泥土（0.3-0.7）
                    if (density < 0.3f) {
                        density = 0.45f;
                    }
                    if (density > 0.7f) {
                        density = 0.6f;
                    }
                }

                // 最终限制
                density = std::max(0.0f, std::min(1.0f, density));

                densityMap[x][y][z] = density;
            }
        }
    }
}

float CreateWorld::calculateSlope(int x, int z)
{
    if (x <= 0 || x >= WORLD_BLOCK-1 || z <= 0 || z >= WORLD_BLOCK-1)
        return 0.0f;

    int currentHeight = terrainmap[x][z];
    float maxDiff = 0;

    // 检查四个邻居
    if (x > 0) {
        float diff = abs(currentHeight - terrainmap[x-1][z]);
        maxDiff = std::max(maxDiff, diff);
    }
    if (x < WORLD_BLOCK-1) {
        float diff = abs(currentHeight - terrainmap[x+1][z]);
        maxDiff = std::max(maxDiff, diff);
    }
    if (z > 0) {
        float diff = abs(currentHeight - terrainmap[x][z-1]);
        maxDiff = std::max(maxDiff, diff);
    }
    if (z < WORLD_BLOCK-1) {
        float diff = abs(currentHeight - terrainmap[x][z+1]);
        maxDiff = std::max(maxDiff, diff);
    }

    return maxDiff / 10.0f;
}


