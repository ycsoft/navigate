#include <math.h>
#include "guidance.h"

guidance::guidance()
{
    m_northAngleDiff = 0.0;

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
void guidance::doProcess(double almx, double almy, double almz, double rotx, double roty, double rotz)
{
    rotx = rotx / 180.0 * 3.1415926;
    roty = roty / 180.0 * 3.1415926;
    rotz = (rotz + m_northAngleDiff) / 180.0f* 3.1415926f;

    if (almz > 1.0f)
    {
        m_newStep = true;
        m_sc = 0;
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
    }

    //一个行走周期结束
    if (almz < -1.0f && m_newStep == true)
    {
        m_newStep = false;
        m_sc = 0;

        int imax = getMaxIndex(m_sgm2_x, m_sgm2_y, m_sgm2_z);

        double dsx = 0.0f;
        double dsy = 0.0f;

        double dsm = 0.6f;
        if (imax == 0)
        {
            dsx = dsm*(cos(m_rot_y_p)*cos(m_rot_z_p));
            dsy =-dsm*(sin(m_rot_z_p)*cos(m_rot_y_p));
        }
        else if (imax == 1)
        {
            dsx = dsm*(sin(m_rot_z_p)*cos(m_rot_x_p));
            dsy = dsm*(cos(m_rot_z_p)*cos(m_rot_x_p));
        }
        else
        {
            dsx = dsm*(sin(m_rot_z_p)*sin(m_rot_x_p));
            dsy = dsm*(cos(m_rot_z_p)*sin(m_rot_x_p));
        }
    }
}

void guidance::initData()
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
}

// 返回最大值的索引
int guidance::getMaxIndex(double a, double b, double c)
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

