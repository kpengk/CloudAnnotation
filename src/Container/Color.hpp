#pragma once
#include <QtGlobal>
#include <QString>

class Color {
public:
    Color()
        : r_{}
        , g_{}
        , b_{}
        , a_{255} {}
    Color(quint8 r, quint8 g, quint8 b, quint8 a = 255)
        : r_{r}
        , g_{g}
        , b_{b}
        , a_{a} {}

    quint8 alpha() const { return a_; }
    quint8 red() const { return r_; }
    quint8 gree() const { return g_; }
    quint8 blue() const { return b_; }

    float alphaF() const { return a_ / 255.0F; }
    float redF() const { return r_ / 255.0F; }
    float greeF() const { return g_ / 255.0F; }
    float blueF() const { return b_ / 255.0F; }

    void setRgb(quint8 r, quint8 g, quint8 b) {
        r_ = r;
        g_ = g;
        b_ = b;
    }

    QString toCSV() const { return QString("%1,%2,%3").arg(r_).arg(g_).arg(b_); }
    QString toCSVWithAlpha() const { return QString("%1,%2,%3,%4").arg(r_).arg(g_).arg(b_).arg(a_); }

private:
    quint8 r_;
    quint8 g_;
    quint8 b_;
    quint8 a_;
};
