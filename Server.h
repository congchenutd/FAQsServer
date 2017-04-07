#include "qhttpserverfwd.h"

#include <QObject>

// 一个Web服务器
class Server : public QObject
{
    Q_OBJECT
    typedef QMap<QString, QString> Parameters;   // parameter name -> parameter value, e.g., username=Carl

public:
    Server();

private slots:
    void onRequest(QHttpRequest* req, QHttpResponse* res);
    void onPhotoDone();

private:
    Parameters parseParameters(const QString& url) const;
    void processPingRequest                 (const Parameters& params, QHttpResponse* res);
    void processSaveRequest                 (const Parameters& params, QHttpResponse* res);
    void processLogDocumentReadingRequest   (const Parameters& params, QHttpResponse* res);
    void processLogAnswerClickingRequest    (const Parameters& params, QHttpResponse* res);
    void processQueryRequest                (const Parameters& params, QHttpResponse* res);
    void processQueryUserProfileRequest     (const Parameters& params, QHttpResponse* res);
    void processSubmitPhotoRequest          (const Parameters& params, QHttpResponse* res);
    void processStaticResourceRequest(const QString& url, QHttpResponse* res);

private:
    QString _photoUser;
};

