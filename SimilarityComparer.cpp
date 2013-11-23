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

    QNetworkAccessManager* manager = new QNetworkAccessManager(this);
    connect(manager, SIGNAL(finished(QNetworkReply*)), this,    SLOT(onReply(QNetworkReply*)));
    connect(manager, SIGNAL(finished(QNetworkReply*)), manager, SLOT(deleteLater()));

    QString url = tr("http://swoogle.umbc.edu/StsService/GetStsSim?operation=api&phrase1=%1&phrase2=%2")
            .arg(sentence1)
            .arg(sentence2);
    manager->get(QNetworkRequest(QUrl(url)));
}

void SimilarityComparer::onReply(QNetworkReply* reply)
{
    QString value = reply->readAll();
    QString query = reply->request().url().query();  // the query associated with the reply
    QStringList sections = query.split("&");
    if(sections.size() == 3)
        emit comparisonResult(sections[1].remove("phrase1="),
                              sections[2].remove("phrase2="),
                              value.toDouble());
}
