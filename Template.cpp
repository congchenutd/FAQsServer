#include "Template.h"
#include <QFile>
#include <QRegExp>

Template::Template(const QString& fileName)
    : _loaded(false)
{
    QFile file(fileName);
    if(file.open(QFile::ReadOnly))
    {
        _html = file.readAll();
        _loaded = true;
    }
}

void Template::addValue(const QString& attribute, const QString& value)
{
    int placeHolder = _html.indexOf("$" + attribute + "$");
    if(placeHolder > -1)
        _html.insert(placeHolder, value);
}

void Template::setValue(const QString& attribute, const QString& value) {
    _html.replace("$" + attribute + "$", value);
}

QByteArray Template::toHTML() const {
    // remove placeholders $XXX$
    return QString(_html).remove(QRegExp("\\$\\w+\\$")).toUtf8();
}
