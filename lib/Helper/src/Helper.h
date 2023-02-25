#pragma once

typedef struct
{
    float x, y, z = 0.0f;
    float xtrim, ytrim, ztrim = 0.0f;
    // roll - крен pitch - наклон yaw - рысканье
    float Pitch = 0.0f;
    float Roll = 0.0f;
    float Yaw = 0.0f;

    float Q1, Q2, Q3, Q4 = 0.0f;

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
