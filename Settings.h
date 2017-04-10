#ifndef SETTINGS_H
#define SETTINGS_H

#include <QSettings>

// 配置信息，保存在FAQsServer.ini文件
class Settings : public QSettings
{
public:
    static Settings* getInstance();

    QString getServerIP()               const;
    uint    getServerPort()             const;
    double  getSimilarityThreshold()    const;  // 判断两个句子是否是语义一致的阈值

    void setServerIP            (const QString& ip);
    void setServerPort          (uint port);
    void setSimilarityThreshold (double threshold);

private:
    Settings();
    void loadDefaults();

private:
    static Settings* _instance;
};

#endif // SETTINGS_H
