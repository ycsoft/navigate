#include <math.h>
#include "guidance.h"

Guidance::Guidance()
{
    m_nyAngle = 0.0f;

    // 是否为一个新的周期
    m_newStep = false;

    // 计算中需要用到的中间量
    m_alm_x_p = 0.0f;
    m_alm_y_p = 0.0f;
    m_alm_z_p = 0.0f;

    m_alm_x_pp = 0.0f;
    m_alm_y_pp = 0.0f;
    m_alm_z_pp = 0.0f;

    m_sgm2_x = 0.0f;
    m_sgm2_y = 0.0f;
    m_sgm2_z = 0.0f;

    m_rot_x_p = 0.0f;
    m_rot_y_p = 0.0f;
    m_rot_z_p = 0.0f;

    // 步数
    m_sc = 0;
}

// 进行一次计算
// 计算中使用线性加速度传感器的值，入参是加速度
// 计算使用弧度值  （弧度= 角度 / 180 * 3.1415926），入参也为弧度值
void Guidance::doProcess(double almx, double almy, double almz,
                         double rotx, double roty, double rotz,
                         double *outx, double *outy)
{

    // rotx = rotx / 180.0f * 3.1415926f;
    // roty = roty / 180.0f * 3.1415926f;
    //  rotz = (rotz + m_nyAngle) / 180.0f * 3.1415926f;

    // 如果大于180， 360-x, 如果不是，直接用

    rotz = 3.1415926 - (rotz - m_nyAngle / 180.f * 3.1415926f);
    almz = almz - 9.81 * cos(rotx); // 加速度传感器 Z 去重力

    if (almz > 1.0f)
    {
        m_newStep = true;
    }

    if (m_newStep == true)
    {
        ++m_sc;

        double n = (double)m_sc;
        double pn = (double)(m_sc-1);

        m_alm_x_pp = sqrt(m_sgm2_x) + m_alm_x_p;
        m_alm_y_pp = sqrt(m_sgm2_y) + m_alm_y_p;
        m_alm_z_pp = sqrt(m_sgm2_z) + m_alm_z_p;

        m_alm_x_p = (m_alm_x_p * pn + almx) / n;
        m_alm_y_p = (m_alm_y_p * pn + almy) / n;
        m_alm_z_p = (m_alm_z_p * pn + almz) / n;

        m_sgm2_x = (pn * (m_alm_x_pp - m_alm_x_p) * (m_alm_x_pp - m_alm_x_p) + (almx-m_alm_x_p) * (almx-m_alm_x_p)) / n;
        m_sgm2_y = (pn * (m_alm_y_pp - m_alm_y_p) * (m_alm_y_pp - m_alm_y_p) + (almy-m_alm_y_p) * (almy-m_alm_y_p)) / n;
        m_sgm2_z = (pn * (m_alm_z_pp - m_alm_z_p) * (m_alm_z_pp - m_alm_z_p) + (almz-m_alm_z_p) * (almz-m_alm_z_p)) / n;

        m_rot_x_p = (m_rot_x_p * pn + rotx) / n;
        m_rot_y_p = (m_rot_y_p * pn + roty) / n;
        m_rot_z_p = (m_rot_z_p * pn + rotz) / n;

        *outx = 0.0f;
        *outy = 0.0f;
    }

    //一个行走周期结束
    if (almz < -1.0f && m_newStep == true)
    {
        double dsx = 0.0f;
        double dsy = 0.0f;
        double dsm = 0.6f;
        dsx = dsm*(sin(m_rot_z_p));
        dsy = dsm*(cos(m_rot_z_p));
        *outx = dsx;
        *outy = dsy;
        initData();
    }
    return;
}

void Guidance::initData()
{
    // 计算中需要用到的中间量
    m_alm_x_p = 0.0f;
    m_alm_y_p = 0.0f;
    m_alm_z_p = 0.0f;

    m_alm_x_pp = 0.0f;
    m_alm_y_pp = 0.0f;
    m_alm_z_pp = 0.0f;

    m_sgm2_x = 0.0f;
    m_sgm2_y = 0.0f;
    m_sgm2_z = 0.0f;

    m_rot_x_p = 0.0f;
    m_rot_y_p = 0.0f;
    m_rot_z_p = 0.0f;

    m_newStep = false;
    m_sc = 0;
}

// 返回最大值的索引
int Guidance::getMaxIndex(double a, double b, double c)
{
    if (a >= b && a >= c)
    {
        return 0;
    }
    else if (b >= c && b >= a)
    {
        return 1;
    }
    else
    {
        return 2;
    }
}

