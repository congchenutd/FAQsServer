#ifndef TEMPLATE_H
#define TEMPLATE_H

#include <QString>

/**
 * A simple and ugly template engine
 * 表示一个网页模版
 * 每一个模板是一个HTML文件，由构造函数的fileName给定文件路径
 * 构造函数读入这个文件并把内容保存在_html中
 *
 * 模板文件中的$xxx$表示一个属性，其中xxx是属性名
 * addValue在模板中添加一个属性
 * setValue设置一个已有的属性的值
 */
class Template
{
public:
    Template(const QString& fileName);
    void addValue(const QString& attribute, const QString& value);
    void setValue(const QString& attribute, const QString& value);
    QByteArray toHTML() const;
    bool isLoaded() const { return _loaded; }

private:
    QString _html;
    bool    _loaded;
};

#endif // TEMPLATE_H
