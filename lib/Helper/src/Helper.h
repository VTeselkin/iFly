#pragma once

typedef struct
{
    float x, y, z = 0.0f;
    float xtrim, ytrim, ztrim = 0.0f;

} DataFly;

class Helper
{
public:
private:
};
#ifdef Data
#undef Data // workaround for Arduino Due, which defines "RTC"...
#endif

extern Helper Data;

