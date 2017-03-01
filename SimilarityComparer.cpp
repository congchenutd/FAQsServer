#include "SimilarityComparer.h"
#include <QNetworkAccessManager>
#include <QUrl>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QStringList>
#include <QDebug>

SimilarityComparer::SimilarityComparer(QObject* parent) :
    QObject(parent) {}

// 请求计算两个句子的语义相似度
// leadQuestion的意思是，如果很多句子的语义都相近，则把他们分在一个组中，选择其中一个作为代表，这个代表就是leadQuestion
// 更好的方案是采用聚类，但是聚类需要比对大量的句子，所以最好是在本地实现语义相似度的量度，否则太慢
void SimilarityComparer::compare(const QString& leadQuestion, const QString& question)
{
    // 这部分好像还没实现：应该把两个句子字面上完全相同的部分删除，否则比对出来的相似度会比实际偏高
    // TODO: find common prefix of two sentences and remove it
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

// 分析UMBC服务返回的相似度结果，并将其以comparisonResult信号发送
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
