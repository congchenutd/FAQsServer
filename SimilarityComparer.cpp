#include "SimilarityComparer.h"
#include <QNetworkAccessManager>
#include <QUrl>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QStringList>
#include <QDebug>

SimilarityComparer::SimilarityComparer(QObject* parent) :
    QObject(parent) {}

void SimilarityComparer::compare(const QString& leadQuestion, const QString& question)
{
    // find common prefix of two sentences and remove it
    // otherwise, the common prefix may make the sentences very similar
    QString sentence1 = leadQuestion.simplified();
    QString sentence2 = question    .simplified();

    MyNetworkAccessManager* manager = new MyNetworkAccessManager(this);
    int i;
    for(i = 0; i < qMin(sentence1.length(), sentence2.length()); ++i)
        if(sentence1.at(i) != sentence2.at(i))
            break;
    manager->setPrefix(sentence1.left(i));
    sentence1.remove(0, i);
    sentence2.remove(0, i);

    connect(manager, SIGNAL(finished(QNetworkReply*)), this,    SLOT(onReply(QNetworkReply*)));
    connect(manager, SIGNAL(finished(QNetworkReply*)), manager, SLOT(deleteLater()));

    QString url = tr("http://swoogle.umbc.edu/StsService/GetStsSim?operation=api&phrase1=%1&phrase2=%2")
            .arg(sentence1)
            .arg(sentence2);
    manager->get(QNetworkRequest(QUrl(url)));
}

void SimilarityComparer::onReply(QNetworkReply* reply)
{
    if(MyNetworkAccessManager* manager = qobject_cast<MyNetworkAccessManager*>(sender()))
    {
        QString prefix = manager->getPrefix();
        QString value = reply->readAll();
        QString query = reply->request().url().query();
        QStringList sections = query.split("&");
        if(sections.size() == 3)
            emit comparisonResult(prefix+sections[1].remove("phrase1="),
                                  prefix+sections[2].remove("phrase2="),
                                  value.toDouble());
    }
}


///////////////////////////////////////////////////////////////////////////
MyNetworkAccessManager::MyNetworkAccessManager(QObject* parent)
    : QNetworkAccessManager(parent) {}
