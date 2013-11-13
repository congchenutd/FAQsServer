#ifndef SIMILARITYCOMPARER_H
#define SIMILARITYCOMPARER_H

#include <QObject>

class QNetworkReply;

// communicating with a semantic similarity web service
class SimilarityComparer : public QObject
{
    Q_OBJECT

public:
    SimilarityComparer(QObject* parent = 0);
    void compare(const QString& leadQuestion, const QString& question);

private slots:
    void onReply(QNetworkReply* reply);

signals:
    void comparisonResult(const QString& leadQuestion, const QString& question, qreal value);
};


#include <QNetworkAccessManager>

class MyNetworkAccessManager : public QNetworkAccessManager
{
    Q_OBJECT

public:
    MyNetworkAccessManager(QObject* parent);
    QString getPrefix() const { return _prefix; }
    void    setPrefix(const QString& prefix) { _prefix = prefix; }

private:
    QString _prefix;  // common prefix of two sentences under comparison
};


#endif // SIMILARITYCOMPARER_H
