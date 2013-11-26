#include "qhttpserverfwd.h"

#include <QObject>

class Server : public QObject
{
    Q_OBJECT
    typedef QMap<QString, QString> Parameters;   // e.g., username=Carl

public:
    Server();

private slots:
    void onRequest(QHttpRequest* req, QHttpResponse* res);
    void onPhotoDone();

private:
    Parameters parseParameters(const QString& url) const;
    void doPing           (const Parameters& params, QHttpResponse* res);
    void doSave           (const Parameters& params, QHttpResponse* res);
    void doLogAPI         (const Parameters& params, QHttpResponse* res);
    void doLogAnswer      (const Parameters& params, QHttpResponse* res);
    void doQuery          (const Parameters& params, QHttpResponse* res);
    void doPersonalProfile(const Parameters& params, QHttpResponse* res);
    void doSubmitPhoto    (const Parameters& params, QHttpResponse* res);

private:
    QString _photoUser;
};

