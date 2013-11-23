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


#endif // SIMILARITYCOMPARER_H
