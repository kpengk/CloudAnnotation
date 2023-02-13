#pragma once
#include <cstdint>
#include <QString>

struct Point 
{
    float x;
    float y;
    float z;
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

class PointCloud
{
public:
    PointCloud(QString name = QString());
    ~PointCloud();

protected:
private:
};
