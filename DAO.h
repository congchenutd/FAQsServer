#ifndef DAO_H
#define DAO_H

#include <QObject>

class QJsonDocument;
class SimilarityComparer;

// 读写数据库的DAO
class DAO : public QObject
{
    Q_OBJECT

public:
    static DAO* getInstance();

    // save a QA pair or just a question
    void save(const QString& userName, const QString& email, const QString& apiSig,
              const QString& question, const QString& link,  const QString& title);

    // log API reading history
    void logDocumentReading(const QString& userName, const QString& email, const QString& apiSig);

    // log answer clicking history
    void logAnswerClicking(const QString& userName, const QString& email, const QString& link);

    // query FAQs for an API (class)
    QJsonDocument queryFAQs(const QString& classSig) const;

    // query personal profile
    QJsonDocument queryUserProfile(const QString& userName) const;

private slots:
    void onComparisonResult(const QString& leadQuestion,
                            const QString& question, qreal similarity);

private:
    DAO();
    int getNextID    (const QString& tableName) const;
    int getID(const QString& tableName, const QString& section, const QString& value) const;

    int getUserID    (const QString& userName)  const;   // for convenience
    int getAPIID     (const QString& signature) const;
    int getQuestionID(const QString& question)  const;
    int getAnswerID  (const QString& link)      const;

    void updateUser    (const QString& userName, const QString& email);  // update single table
    void updateAPI     (const QString& signature);
    void updateQuestion(const QString& question, int apiID);
    void updateAnswer  (const QString& link, const QString& title);

    void updateQuestionUserRelation  (int groupID, int userID);           // update relation table
    void updateQuestionAPIRelation   (int groupID, int apiID);
    void updateQuestionAnswerRelation(int groupID, int answerID);

    void updateLead(int questionID);   // try to make questionID the new lead

    // initiate comparison between the question and other lead questions associated with apiID
    void measureSimilarity(const QString& question, int apiID);

    void addUserReadDocument(int userID, int apiID);     // user viewed API doc
    void addUserClickAnswer (int userID, int answerID);  // user clicked the answer

    // table -> json
    QJsonObject createAnswerJson    (int answerID) const;  // an answer -> json
    QJsonObject createUserJson      (int userID)   const;  // a user    -> json
    QJsonArray  createAnswersJson   (const QStringList& questionIDs) const;  // question group -> its answers
    QJsonArray  createUsersJson     (const QStringList& questionIDs) const;  // question group -> its users
    QJsonObject createQuestionJson (int leadID) const;  // question group -> json
    QJsonArray  createQuestionsJson(int apiID)  const;  // api            -> its questions

    QString getCurrentDateTime() const;

private:
    static DAO* _instance;
    SimilarityComparer* _comparer;
};

#endif // DAO_H
