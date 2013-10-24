#include "Server.h"
#include "DAO.h"
#include <QCoreApplication>
#include <QRegExp>
#include <QStringList>
#include <QJsonDocument>
#include <QDebug>

#include <qhttpserver.h>
#include <qhttprequest.h>
#include <qhttpresponse.h>

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
        res->end("Hello, this is FAQsServer. Please use FAQsBrowser to communicate with me.");
        return;
    }

    qDebug() << url;

    url.remove(0, 2);  // remove "/?"
    Parameters params = parseParameters(url);
    QString action = params["action"];
    if(action == "ping")
        processPing(params, res);
    else if(action == "save")
        processSave(params, res);
    else if(action == "query")
        processQuery(params, res);

    res->end();
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
        result.insert(section.section('=', 0, 0),    // left part of the 1st =
                      section.section('=', 1, -1));  // right of the =

    return result;
}

void Server::processPing(const Parameters& params, QHttpResponse* res)
{
    res->setHeader("Content-Type", "text/html");
    res->writeHead(200);
    QString userName = params.contains("username") ? params["username"] : "anonymous";
    res->write(tr("Hello %1, I'm alive!").arg(userName).toUtf8());
}

void Server::processSave(const Parameters& params, QHttpResponse* res)
{
    DAO::getInstance()->save(params["username"],
                             params["email"],
                             params["api"],
                             params["question"],
                             params["link"],
                             params["title"]);

    res->setHeader("Content-Type", "text/html");
    res->writeHead(200);
    res->write(tr("Your FAQ is saved").toUtf8());
}

void Server::processQuery(const Server::Parameters& params, QHttpResponse* res)
{
    QJsonDocument json = DAO::getInstance()->query(params["api"]);
    res->setHeader("Content-Type", "text/html");
    res->writeHead(200);
    res->write(json.toJson());
    qDebug() << json.toJson();
}


int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    Server server;
    app.exec();
}
