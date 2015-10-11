#ifndef GUIDANCE_H
#define GUIDANCE_H


class Guidance
{
public:

    Guidance();
    void doProcess(double almx, double almy, double almz,
                   double rotx, double roty, double rotz,
                   double *outx, double *outy);

private:

    void initData();
    int getMaxIndex(double a, double b, double c);

private:
    // 是否为一个新的周期
    bool m_newStep;

    // 计算中需要用到的中间量
    double m_alm_x_p;
    double m_alm_y_p;
    double m_alm_z_p;

    double m_alm_x_pp;
    double m_alm_y_pp;
    double m_alm_z_pp;

    double m_sgm2_x;
    double m_sgm2_y;
    double m_sgm2_z;

    double m_rot_x_p;
    double m_rot_y_p;
    double m_rot_z_p;

    // 步数
    double m_sc;

public:
    // 地图Y轴与地球正北方夹角
    // Y轴在N顺时针为负值, 画地图时可确定
    double m_nyAngle;

};

#endif // GUIDANCE_H
