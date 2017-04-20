#include "Settings.h"

#include <QFile>

// Singleton方法
Settings* Settings::getInstance()
{
    if(_instance == 0)
        _instance = new Settings();
    return _instance;
}

QString Settings::getServerIP()   const { return value("IP")    .toString(); }
uint    Settings::getServerPort() const { return value("Port")  .toUInt();   }
double  Settings::getSimilarityThreshold()  const { return value("SimilarityThreshold").toDouble(); }

void Settings::setServerIP  (const QString& ip) { setValue("IP", ip); }
void Settings::setServerPort(uint port)         { setValue("Port", port); }
void Settings::setSimilarityThreshold(double threshold) { setValue("SimilarityThreshold", threshold); }

Settings::Settings()
    : QSettings("FAQsServer.ini", QSettings::IniFormat)
{
    if(QFile("FAQsServer.ini").size() == 0)   // no setting
        loadDefaults();
}

void Settings::loadDefaults()
{
    setServerIP("localhost");
    setServerPort(8080);
    setSimilarityThreshold(0.75);
}

Settings* Settings::_instance = 0;


