#ifndef TEMPLATE_H
#define TEMPLATE_H

#include <QString>

// A simple and ugly template engine
class Template
{
public:
    Template(const QString& fileName);
    void addValue(const QString& attribute, const QString& value);  // for repetitive attributes
    void setValue(const QString& attribute, const QString& value);  // replace once
    QByteArray toHTML() const;

private:
    QString html;
};

#endif // TEMPLATE_H
