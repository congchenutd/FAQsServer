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

private:
    Parameters parseParameters(const QString& url) const;
    void processPing (const Parameters& params, QHttpResponse* res);
    void processSave (const Parameters& params, QHttpResponse* res);
    void processQuery(const Parameters& params, QHttpResponse* res);
};

