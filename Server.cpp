#include "Server.h"
#include "DAO.h"
#include "SnippetCreator.h"
#include "Settings.h"

#include <QStringList>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QUrl>
#include <QDebug>

#include <qhttpserver.h>
#include <qhttprequest.h>
#include <qhttpresponse.h>
#include <QFile>
#include <QDir>

Server::Server()
{
    QHttpServer* server = new QHttpServer(this);
    connect(server, SIGNAL(newRequest(QHttpRequest*, QHttpResponse*)),
            this,   SLOT  (onRequest (QHttpRequest*, QHttpResponse*)));
            
    Settings* settings = Settings::getInstance();
    server->listen(settings->getServerPort());

    qDebug() << "FAQsServer running on port" << settings->getServerPort();
}

/**
 * Process HTTP request
 * @param req   - the request
 * @param res   - response object
 */
void Server::onRequest(QHttpRequest* req, QHttpResponse* res)
{
    QString url = req->url().toString();
    if(!url.startsWith("/?action"))
    {
        processStaticResourceRequest(url, res);
        return;
    }

    url.remove(0, 2);  // remove "/?"
    Parameters params = parseParameters(url);   // parameters in the request
    QString action = params["action"];
    if(action == "ping")
        processPingRequest(params, res);
    else if(action == "save")
        processSaveRequest(params, res);
    else if(action == "logapi")
        processLogDocumentReadingRequest(params, res);
    else if(action == "loganswer")
        processLogAnswerClickingRequest(params, res);
    else if(action == "query")
        processQueryRequest(params, res);
    else if(action == "personal")
        processQueryUserProfileRequest(params, res);
    else if(action == "submitphoto")
    {
        processSubmitPhotoRequest(params, res);
        connect(req, SIGNAL(end()), this, SLOT(onPhotoDone()));
        req->storeBody();  // the request object will store the data internally. WHY?
    }
}

/**
 * Parse a request URL and get its parameters
 * e.g., the URL is XXX/?action=query&apisig=YYY
 * @param url   - the URL
 * @return      - a Parameters object
 */
Server::Parameters Server::parseParameters(const QString& url) const
{
    Parameters result;
    if(url.isEmpty())
        return result;

    QStringList sections = url.split("&");
    if(sections.length() == 0)
        return result;

    foreach(const QString& section, sections)
        result.insert(section.section('=', 0,  0),   // left part of the 1st =
                      section.section('=', 1, -1));  // right of the =

    return result;
}

/**
 * Process ping request and respond with a pong
 * @param params    - parameters of the request
 * @param res       - response
 */
void Server::processPingRequest(const Parameters& params, QHttpResponse* res)
{
    res->setHeader("Content-Type", "text/html");
    res->writeHead(200);
    QString userName = params.contains("username") ? params["username"] : "anonymous";
    res->write(tr("Hello %1, I'm alive!").arg(userName).toUtf8());
    res->end();
}

/**
 * Process saving FAQ request
 * @param params    - parameters of the request
 * @param res       - response
 */
void Server::processSaveRequest(const Parameters& params, QHttpResponse* res)
{
    // link and title may contain percentage encoded reserved chars, such as & < > #
    // convert them back to human readable chars
    DAO::getInstance()->save(params["username"],
                             params["email"],
                             params["apisig"],
                             params["question"],
                             QUrl::fromPercentEncoding(params["link"] .toUtf8()),
                             QUrl::fromPercentEncoding(params["title"].toUtf8()));

    res->setHeader("Content-Type", "text/html");
    res->writeHead(200);
    res->write(tr("Your FAQ is saved").toUtf8());
    res->end();
}

/**
 * Process log document reading request
 * @param params    - parameters of the request
 * @param res       - response
 */
void Server::processLogDocumentReadingRequest(const Server::Parameters& params, QHttpResponse* res)
{
    DAO::getInstance()->logDocumentReading(params["username"],
                               params["email"],
                               params["apisig"]);

    res->setHeader("Content-Type", "text/html");
    res->writeHead(200);
    res->write(tr("Your API is logged").toUtf8());
    res->end();
}

/**
 * Process log answer clicking request
 * @param params    - parameters of the request
 * @param res       - response
 */
void Server::processLogAnswerClickingRequest(const Server::Parameters& params, QHttpResponse* res)
{
    // link may contain percentage encoded reserved chars, such as & < > #
    // convert them back to human readable chars
    DAO::getInstance()->logAnswerClicking(params["username"],
                                  params["email"],
                                  QUrl::fromPercentEncoding(params["link"] .toUtf8()));

    res->setHeader("Content-Type", "text/html");
    res->writeHead(200);
    res->write(tr("Your Answer is logged").toUtf8());
    res->end();
}

/**
 * Process query FAQs request
 * @param params    - parameters of the request
 * @param res       - response
 */
void Server::processQueryRequest(const Server::Parameters& params, QHttpResponse* res)
{
    QJsonArray jaFAQs = DAO::getInstance()->queryFAQs(params["class"]).array();
    if(jaFAQs.isEmpty())   // returned is a json array
        return;
    res->setHeader("Content-Type", "text/html");
    res->writeHead(200);
    QJsonDocument jdSnippet = SnippetCreator().createFAQs(jaFAQs);  // create html, encapsulated in a json doc
    res->write(jdSnippet.toJson());
    res->end();
}

/**
 * Process query user profile request
 * @param params    - parameters of the request
 * @param res       - response
 */
void Server::processQueryUserProfileRequest(const Server::Parameters& params, QHttpResponse* res)
{
    QJsonDocument json = DAO::getInstance()->queryUserProfile(params["username"]);
    res->setHeader("Content-Type", "text/html");
    res->writeHead(200);
    res->write(SnippetCreator().createProfilePage(json.object()));
    res->end();

    qDebug() << SnippetCreator().createProfilePage(json.object());
}

/**
 * Process user photo submission
 * @param params
 * @param res
 */
void Server::processSubmitPhotoRequest(const Server::Parameters& params, QHttpResponse* res)
{
    _photoUser = params["username"];
    res->writeHead(200);
    res->write("Accepting photo ...");
    // do not call res->end(), because the photo transfer is not finished
}

/**
 * Save the photo when the transfer is done
 */
void Server::onPhotoDone()
{
    QHttpRequest* req = static_cast<QHttpRequest*>(sender());
    QDir::current().mkdir("Photos");
    QFile file("./Photos/" + _photoUser + ".png");
    if(file.open(QFile::WriteOnly))
        file.write(req->body());

    // FIXME: Regardless of the input file type, photos are saved as png. Test this!
}

/**
 * Process static web page request
 * @param url   - requested URL
 * @param res   - response
 */
void Server::processStaticResourceRequest(const QString& url, QHttpResponse* res)
{
    QFile file("." + url);
    if(file.open(QFile::ReadOnly))  // open the local file and send it back
    {
        res->writeHead(200);
        res->write(file.readAll());
    }
    else {
        res->writeHead(404);
    }
    res->end();
}
