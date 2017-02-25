#ifndef SETTINGS_H
#define SETTINGS_H

#include <QSettings>

// 配置信息，保存在FAQServer.ini文件
class Settings : public QSettings
{
public:
    static Settings* getInstance();

    double  getThreshold()  const;  // 判断两个句子是否是语义一致的阈值
    QString getServerIP()   const;
    uint    getServerPort() const;

private:
    Settings();

private:
    static Settings* _instance;
};

#endif // SETTINGS_H
