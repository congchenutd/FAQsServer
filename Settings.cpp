#include "Settings.h"

// Singleton方法
Settings* Settings::getInstance()
{
    if(_instance == 0)
        _instance = new Settings();
    return _instance;
}

double  Settings::getThreshold()  const { return value("Threshold").toDouble(); }
QString Settings::getServerIP()   const { return value("IP")       .toString(); }
uint    Settings::getServerPort() const { return value("Port")     .toInt();    }

Settings::Settings()
    : QSettings("FAQServer.ini", QSettings::IniFormat)
{}

Settings* Settings::_instance = 0;


