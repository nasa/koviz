#ifndef CURVEMODELPARAMETER_H
#define CURVEMODELPARAMETER_H

#include <stdio.h>
#include <stdlib.h>
#include "parameter.h"

class CurveModelParameter : public Parameter
{
public:
    CurveModelParameter();

    QString name() const override;
    QString unit() const override;
    void setName(const QString& name ) override;
    void setUnit(const QString& unit ) override;

    double bias() const;
    double scale() const;
    void setBias(double bias);
    void setScale(double scale);

private:
    QString _name;
    QString _unit;
    double _bias;
    double _scale;
};

#endif // CURVEMODELPARAMETER_H
