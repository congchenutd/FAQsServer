#include "DAO.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QVariant>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

DAO* DAO::_instance = 0;

DAO* DAO::getInstance()
{
    if(_instance == 0)
        _instance = new DAO;
    return _instance;
}

DAO::DAO()
{
    QSqlDatabase database = QSqlDatabase::addDatabase("QSQLITE");
    database.setDatabaseName("FAQs.db");
    database.open();

    QSqlQuery query;
    query.exec("create table Users ( \
               ID    int primary key, \
               Name  varchar unique, \
               Email varchar unique)");
    query.exec("create table APIs ( \
               ID      int primary key, \
               API     varchar unique)");            // lib;package;class;method
    query.exec("create table Questions ( \
               ID       int primary key, \
               Question varchar unique)");
    query.exec("create table Answers ( \
               ID    int primary key, \
               Link  varchar unique, \
               Title varchar)");
    query.exec("create table QuestionsUsersRelation ( \
               QuestionID int references Questions(ID) on delete cascade on update cascade, \
               UserID     int references Users    (ID) on delete cascade on update cascade, \
               primary key (QuestionID, UserID))");
    query.exec("create table QuestionsAPIsRelation ( \
               QuestionID int references Questions(ID) on delete cascade on update cascade, \
               APIID      int references APIs     (ID) on delete cascade on update cascade, \
               primary key (QuestionID, APIID))");
    query.exec("create table QuestionsAnswersRelation ( \
               QuestionID int references Questions(ID) on delete cascade on update cascade, \
               AnswerID   int references Answers  (ID) on delete cascade on update cascade, \
               primary key (QuestionID, AnswerID))");
}

int DAO::getNextID(const QString& tableName) const
{
    if(tableName.isEmpty())
        return 0;
    QSqlQuery query;
    query.exec(tr("select max(ID) from %1").arg(tableName));
    return query.next() ? query.value(0).toInt() + 1 : 0;
}

int DAO::getUserID(const QString& userName) const {
    return getID("Users", "Name", userName);
}
int DAO::getAPIID(const QString& api) const {
    return getID("APIs", "API", api);
}
int DAO::getQuestionID(const QString& question) const {
    return getID("Questions", "Question", question);
}
int DAO::getAnswerID(const QString& link) const {
    return getID("Answers", "Link", link);
}

int DAO::getID(const QString& tableName, const QString& section, const QString& value) const
{
    QSqlQuery query;
    query.exec(tr("select ID from %1 where %2 = \'%3\'").arg(tableName)
                                                        .arg(section)
                                                        .arg(value));
    return query.next() ? query.value(0).toInt() : -1;
}

void DAO::updateUser(const QString& userName, const QString& email)
{
    QSqlQuery query;
    int id = getUserID(userName);
    if(id > 0) {
        query.exec(tr("update Users set Name = \'%1\', Email = \'%2\' where ID = %3")
                   .arg(userName)
                   .arg(email)
                   .arg(id));
    }
    else {
        query.exec(tr("insert into Users values (%1, \'%2\', \'%3\')")
                   .arg(getNextID("Users"))
                   .arg(userName)
                   .arg(email));
    }
}

void DAO::updateAPI(const QString& api)
{
    QSqlQuery query;
    int id = getAPIID(api);
    if(id > 0) {
        query.exec(tr("update APIs set API = \'%1\' where ID = %2")
                   .arg(api)
                   .arg(id));
    }
    else {
        query.exec(tr("insert into APIs values (%1, \'%2\')")
                   .arg(getNextID("APIs"))
                   .arg(api));
    }
}

void DAO::updateQuestion(const QString& question)
{
    QSqlQuery query;
    int id = getQuestionID(question);
    if(id > 0) {
        query.exec(tr("update Questions set Question = \'%1\' where ID = %2")
                   .arg(question)
                   .arg(id));
    }
    else {
        query.exec(tr("insert into Questions values (%1, \'%2\')")
                   .arg(getNextID("Questions"))
                   .arg(question));
    }
}

void DAO::updateAnswer(const QString& link, const QString& title)
{
    QSqlQuery query;
    int id = getAnswerID(link);
    if(id > 0) {
        query.exec(tr("update Answers set Link = \'%1\', Title = \'%2\' where ID = %3")
                   .arg(link)
                   .arg(title)
                   .arg(id));
    }
    else {
        query.exec(tr("insert into Answers values (%1, \'%2\', \'%3\')")
                   .arg(getNextID("Answers"))
                   .arg(link)
                   .arg(title));
    }
}

void DAO::updateQuestionsUsersRelation(int questionID, int userID)
{
    QSqlQuery query;
    query.exec(tr("delete from QuestionsUsersRelation where QuestionID = %1 and UserID = %2")
               .arg(questionID)
               .arg(userID));
    query.exec(tr("insert into QuestionsUsersRelation values (%1, %2)")
               .arg(questionID)
               .arg(userID));
}

void DAO::updateQuestionsAPIsRelation(int questionID, int apiID)
{
    QSqlQuery query;
    query.exec(tr("delete from QuestionsAPIsRelation where QuestionID = %1 and APIID = %2")
               .arg(questionID)
               .arg(apiID));
    query.exec(tr("insert into QuestionsAPIsRelation values (%1, %2)")
               .arg(questionID)
               .arg(apiID));
}

void DAO::updateQuestionsAnswersRelation(int questionID, int answerID)
{
    QSqlQuery query;
    query.exec(tr("delete from QuestionsAnswersRelation where QuestionID = %1 and AnswerID = %2")
               .arg(questionID)
               .arg(answerID));
    query.exec(tr("insert into QuestionsAnswersRelation values (%1, %2)")
               .arg(questionID)
               .arg(answerID));
}

void DAO::save(const QString& userName, const QString& email, const QString& api,
               const QString& question, const QString& link,  const QString& title)
{
    updateUser(userName, email);
    updateAPI(api);
    updateQuestion(question);
    updateAnswer(link, title);

    int questionID = getQuestionID(question);
    int userID     = getUserID    (userName);
    int apiID      = getAPIID     (api);
    int answerID   = getAnswerID  (link);
    updateQuestionsUsersRelation  (questionID, userID);
    updateQuestionsAPIsRelation   (questionID, apiID);
    updateQuestionsAnswersRelation(questionID, answerID);
}


// An example of returned JSON file:
//{
//	"api": "java.util.ArrayList.ensureCapacity",
//	"questions":
//	[
//		{
//			"question": "question1",
//			"users"   : [{"username": "user1", "email": "user1@gmai.com"},
//						 {"username": "user2", "email": "user2@gmail.com"}],
//			"answers" : [{"link": "link1", "title": "title1"},
//						 {"link": "link2", "title": "title2"}]
//		},
//		{
//			"question": "question2",
//			"users"   : [{"username": "user1", "email": "user1@gmai.com"},
//						 {"username": "user2", "email": "user2@gmail.com"}],
//			"answers" : [{"link": "link3", "title": "title3"},
//						 {"link": "link4", "title": "title4"}]
//		},
//	]
//}
QJsonDocument DAO::query(const QString& fullClassName)
{
    QJsonArray apisJson;
    QSqlQuery query;
    query.exec(tr("select ID, API from APIs where API like \'%1%\'").arg(fullClassName));
    while(query.next())
    {
        int     apiID = query.value(0).toInt();
        QString api   = query.value(1).toString().section(";", -1, -1);
        QJsonObject apiJson;
        apiJson.insert("api",       api);
        apiJson.insert("questions", createQuestionsJason(apiID));
        apisJson.append(apiJson);
    }

    return QJsonDocument(apisJson);
}

QJsonObject DAO::createUserJson(int userID) const
{
    QJsonObject result;
    QSqlQuery query;
    query.exec(tr("select Name, Email from Users where ID = %1").arg(userID));
    if(query.next())
    {
        result.insert("name",  query.value(0).toString());
        result.insert("email", query.value(1).toString());
    }
    return result;
}

QJsonArray DAO::createUsersJson(int questionID) const
{
    QJsonArray result;
    QSqlQuery query;
    query.exec(tr("select UserID from QuestionsUsersRelation where QuestionID = %1").arg(questionID));
    while(query.next())
        result.append(createUserJson(query.value(0).toInt()));
    return result;
}

QJsonObject DAO::createAnswerJson(int answerID) const
{
    QJsonObject result;
    QSqlQuery query;
    query.exec(tr("select Link, Title from Answers where ID = %1").arg(answerID));
    if(query.next())
    {
        result.insert("link",  query.value(0).toString());
        result.insert("title", query.value(1).toString());
    }
    return result;
}

QJsonArray DAO::createAnswersJson(int questionID) const
{
    QJsonArray result;
    QSqlQuery query;
    query.exec(tr("select AnswerID from QuestionsAnswersRelation where QuestionID = %1").arg(questionID));
    while(query.next())
        result.append(createAnswerJson(query.value(0).toInt()));
    return result;
}

QJsonObject DAO::createQuestionJason(int questionID) const
{
    QJsonObject result;
    QSqlQuery query;
    query.exec(tr("select Question from Questions where ID = %1").arg(questionID));
    if(query.next())
    {
        result.insert("question", query.value(0).toString());
        result.insert("users",   createUsersJson  (questionID));
        result.insert("answers", createAnswersJson(questionID));
    }
    return result;
}

QJsonArray DAO::createQuestionsJason(int apiID) const
{
    QJsonArray result;
    QSqlQuery query;
    query.exec(tr("select QuestionID from QuestionsAPIsRelation where APIID = %1").arg(apiID));
    while(query.next())
        result.append(createQuestionJason(query.value(0).toInt()));
    return result;
}
