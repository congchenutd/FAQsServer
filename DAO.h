#ifndef DAO_H
#define DAO_H

#include <QObject>

class QJsonDocument;

class DAO : QObject
{
public:
    static DAO* getInstance();

    void save(const QString& userName, const QString& email, const QString& api,
              const QString& question, const QString& link,  const QString& title);
    QJsonDocument query(const QString& classSignature);

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
    void updateQuestion(const QString& question);
    void updateAnswer  (const QString& link, const QString& title);
    void updateQuestionsUsersRelation  (int questionID, int userID);
    void updateQuestionsAPIsRelation   (int questionID, int apiID);
    void updateQuestionsAnswersRelation(int questionID, int answerID);

    QJsonObject createUserJson      (int userID)     const;
    QJsonArray  createUsersJson     (int questionID) const;
    QJsonObject createAnswerJson    (int answerID)   const;
    QJsonArray  createAnswersJson   (int questionID) const;
    QJsonObject createQuestionJason (int questionID) const;
    QJsonArray  createQuestionsJason(int apiID)      const;

private:
    static DAO* _instance;
};

#endif // DAO_H
