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
    bool isLoaded() const { return _loaded; }

private:
    QString _html;
    bool    _loaded;
};

#endif // TEMPLATE_H
