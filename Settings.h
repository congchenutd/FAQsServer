#ifndef SETTINGS_H
#define SETTINGS_H

#include <QSettings>

class Settings : public QSettings
{
public:
    static Settings* getInstance();

    double  getThreshold()  const;
    QString getServerIP()   const;
    uint    getServerPort() const;

private:
    Settings();

private:
    static Settings* _instance;
};

#endif // SETTINGS_H
