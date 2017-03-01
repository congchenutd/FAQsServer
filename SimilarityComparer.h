#ifndef SIMILARITYCOMPARER_H
#define SIMILARITYCOMPARER_H

#include <QObject>

class QNetworkReply;

// Communicating with a semantic similarity web service
// 比较两个句子的语义相似度
// 使用的是UMBC一个web服务
class SimilarityComparer : public QObject
{
    Q_OBJECT

public:
    SimilarityComparer(QObject* parent = 0);
    void compare(const QString& leadQuestion, const QString& question);

private slots:
    void onReply(QNetworkReply* reply);

signals:
    // 用来发送语义相似度的结果，value的范围是0~1，1表示完全相同
    void comparisonResult(const QString& leadQuestion, const QString& question, qreal value);
};


#endif // SIMILARITYCOMPARER_H
