#include "Server.h"
#include "DAO.h"
#include "SnippetCreator.h"
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

Server::Server()
{
    QHttpServer* server = new QHttpServer(this);
    connect(server, SIGNAL(newRequest(QHttpRequest*, QHttpResponse*)),
            this,   SLOT  (onRequest (QHttpRequest*, QHttpResponse*)));
            
    server->listen(QHostAddress::Any, 8080);
}

void Server::onRequest(QHttpRequest* req, QHttpResponse* res)
{
    QString url = req->url().toString();
    if(!url.startsWith("/?action"))
    {
        res->writeHead(403);
        res->end("Hello, this is FAQ Server. Please use FAQ Browser to communicate with me.");
        return;
    }

    qDebug() << url;

    url.remove(0, 2);  // remove "/?"
    Parameters params = parseParameters(url);
    QString action = params["action"];
    if(action == "ping")
        doPing(params, res);
    else if(action == "save")
        doSave(params, res);
    else if(action == "logapi")
        doLogAPI(params, res);
    else if(action == "loganswer")
        doLogAnswer(params, res);
    else if(action == "query")
        doQuery(params, res);
    else if(action == "personal")
        doPersonalProfile(params, res);
    else if(action == "submitphoto")
    {
        doSubmitPhoto(params, res);
        connect(req, SIGNAL(end()), this, SLOT(onPhotoDone()));
        req->storeBody();  // the request object will store the data internally
    }
}

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

void Server::doPing(const Parameters& params, QHttpResponse* res)
{
    res->setHeader("Content-Type", "text/html");
    res->writeHead(200);
    QString userName = params.contains("username") ? params["username"] : "anonymous";
    res->write(tr("Hello %1, I'm alive!").arg(userName).toUtf8());
    res->end();
}

void Server::doSave(const Parameters& params, QHttpResponse* res)
{
    // link and title may contain reserved chars, such as & < > #
    // they are pertentage encoded by the client
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

void Server::doLogAPI(const Server::Parameters& params, QHttpResponse* res)
{
    DAO::getInstance()->logAPI(params["username"],
                               params["email"],
                               params["apisig"]);

    res->setHeader("Content-Type", "text/html");
    res->writeHead(200);
    res->write(tr("Your API is logged").toUtf8());
    res->end();
}

void Server::doLogAnswer(const Server::Parameters& params, QHttpResponse* res)
{
    // link may contain reserved chars, such as & < > #
    // they are pertentage encoded by the client
    // convert them back to human readable chars
    DAO::getInstance()->logAnswer(params["username"],
                                  params["email"],
                                  QUrl::fromPercentEncoding(params["link"] .toUtf8()));

    res->setHeader("Content-Type", "text/html");
    res->writeHead(200);
    res->write(tr("Your Answer is logged").toUtf8());
    res->end();
}

void Server::doQuery(const Server::Parameters& params, QHttpResponse* res)
{
    QJsonDocument json = DAO::getInstance()->query(params["class"]);
    if(json.array().isEmpty())   // returned is a json array
        return;
    res->setHeader("Content-Type", "text/html");
    res->writeHead(200);
    res->write(SnippetCreator().createFAQs(json.array()).toJson());
    res->end();

    qDebug() << json.toJson();
}

void Server::doPersonalProfile(const Server::Parameters& params, QHttpResponse* res)
{
    QJsonDocument json = DAO::getInstance()->personalProfile(params["username"]);
    res->setHeader("Content-Type", "text/html");
    res->writeHead(200);
    res->write(SnippetCreator().createProfilePage(json.object()));
    res->end();

    qDebug() << json.toJson();
}

void Server::doSubmitPhoto(const Server::Parameters& params, QHttpResponse* res)
{
    _photoUser = params["username"];
    res->writeHead(200);
    res->write("Accepting photo ...");
    // do not call res->end(), because the photo transfer is not finished
}

void Server::onPhotoDone()
{
    QHttpRequest* req = static_cast<QHttpRequest*>(sender());
    QFile file(_photoUser + ".png");
    if(file.open(QFile::WriteOnly))
        file.write(req->body());
}
