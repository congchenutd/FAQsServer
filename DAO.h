#ifndef DAO_H
#define DAO_H

#include <QObject>

class QJsonDocument;
class SimilarityComparer;

class DAO : public QObject
{
    Q_OBJECT

public:
    static DAO* getInstance();

    void save(const QString& userName, const QString& email, const QString& api,
              const QString& question, const QString& link,  const QString& title);
    void logAPI  (const QString& userName, const QString& email, const QString& api);
    void logAnswer(const QString& userName, const QString& email, const QString& link);
    QJsonDocument query(const QString& classSignature);

private slots:
    void onComparisonResult(const QString& leadQuestion,
                            const QString& question, qreal similarity);

private:
    DAO();
    int getNextID    (const QString& tableName) const;
    int getUserID    (const QString& userName)  const;
    int getAPIID     (const QString& api)       const;
    int getQuestionID(const QString& question)  const;
    int getAnswerID  (const QString& link)      const;
    int getID(const QString& tableName, const QString& section, const QString& value) const;

    void updateUser    (const QString& userName, const QString& email);
    void updateAPI     (const QString& api);
    void updateQuestion(const QString& question, int apiID);
    void updateAnswer  (const QString& link, const QString& title);
    void updateQuestionUserRelation  (int groupID, int userID);
    void updateQuestionAPIRelation   (int groupID, int apiID);
    void updateQuestionAnswerRelation(int groupID, int answerID);
    void updateLead(int questionID);   // try to make questionID the new lead

    // compare question with other lead question associated with apiID
    void measureSimilarity(const QString& question, int apiID);

    void addUserAPIHistory     (int userID, int apiID);
    void addUserQuestionHistory(int userID, int answerID);

    QJsonObject createAnswerJson    (int answerID) const;
    QJsonObject createUserJson      (int userID)   const;
    QJsonArray  createAnswersJson   (const QStringList& questionIDs) const;
    QJsonArray  createUsersJson     (const QStringList& questionIDs) const;
    QJsonObject createQuestionJason (int leadID) const;
    QJsonArray  createQuestionsJason(int apiID)  const;

    QString getCurrentDateTime() const;

private:
    static DAO* _instance;
    SimilarityComparer* _comparer;
};

#endif // DAO_H
