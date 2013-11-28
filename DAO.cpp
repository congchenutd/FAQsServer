#include "DAO.h"
#include "SimilarityComparer.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QVariant>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>
#include <QStringList>
#include <QDateTime>
#include <QSettings>

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
    query.exec("create table APIs ( \
               ID        int primary key, \
               Signature varchar unique not null)");    // lib;package.class.method
    query.exec("create table Answers ( \
               ID    int primary key, \
               Link  varchar unique not null, \
               Title varchar        not null)");
    query.exec("create table Users ( \
               ID    int primary key, \
               Name  varchar unique not null, \
               Email varchar unique)");
    query.exec("create table Questions ( \
               ID         int primary key, \
               Question   varchar unique not null, \
               AskCount   int, \
               Parent     int)");
    query.exec("create table UserAskQuestion ( \
               QuestionID int references Questions(ID) on delete cascade on update cascade, \
               UserID     int references Users    (ID) on delete cascade on update cascade, \
               primary key (QuestionID, UserID))");
    query.exec("create table QuestionAboutAPI ( \
               QuestionID int references Questions(ID) on delete cascade on update cascade, \
               APIID      int references APIs     (ID) on delete cascade on update cascade, \
               primary key (QuestionID, APIID))");
    query.exec("create table AnswerToQuestion ( \
               QuestionID int references Questions(ID) on delete cascade on update cascade, \
               AnswerID   int references Answers  (ID) on delete cascade on update cascade, \
               primary key (QuestionID, AnswerID))");
    query.exec("create table UserReadAPI ( \
               UserID int references Users(ID) on delete cascade on update cascade, \
               APIID  int references APIs (ID) on delete cascade on update cascade, \
               Time   varchar, \
               primary key (UserID, APIID, Time))");
    query.exec("create table UserReadQuestion ( \
                UserID     int references Users    (ID) on delete cascade on update cascade, \
                QuestionID int references Questions(ID) on delete cascade on update cascade, \
                Time       varchar, \
                primary key (UserID, QuestionID, Time))");

    _comparer = new SimilarityComparer(this);
    connect(_comparer, SIGNAL(comparisonResult  (QString,QString,qreal)),
            this,      SLOT  (onComparisonResult(QString,QString,qreal)));
}

int DAO::getNextID(const QString& tableName) const
{
    if(tableName.isEmpty())
        return 0;
    QSqlQuery query;
    query.exec(tr("select max(ID) from %1").arg(tableName));
    return query.next() ? query.value(0).toInt() + 1 : 0;
}

int DAO::getID(const QString& tableName, const QString& section, const QString& value) const
{
    QSqlQuery query;
    query.prepare(tr("select ID from %1 where %2 = :value").arg(tableName)
                                                           .arg(section));
    query.bindValue(":value", value);
    query.exec();
    return query.next() ? query.value(0).toInt() : -1;
}
int DAO::getUserID    (const QString& userName)  const { return getID("Users",     "Name",      userName); }
int DAO::getAPIID     (const QString& signature) const { return getID("APIs",      "Signature", signature); }
int DAO::getQuestionID(const QString& question)  const { return getID("Questions", "Question",  question); }
int DAO::getAnswerID  (const QString& link)      const { return getID("Answers",   "Link",      link); }

void DAO::updateAPI(const QString& signature)
{
    if(signature.isEmpty() || getAPIID(signature) >= 0)
        return;

    QSqlQuery query;
    query.prepare("insert into APIs values (:id, :sig)");
    query.bindValue(":id",  getNextID("APIs"));
    query.bindValue(":sig", signature);
    query.exec();
}

void DAO::updateUser(const QString& userName, const QString& email)
{
    if(userName.isEmpty())
        return;

    // update existing user or insert a new one
    QSqlQuery query;
    int id = getUserID(userName);
    if(id > 0) {
        query.prepare("update Users set Name = :name, Email = :email where ID = :id");
        query.bindValue(":id", id);
    }
    else {
        query.prepare("insert into Users values (:id, :name, :email)");
        query.bindValue(":id", getNextID("Users"));
    }
    query.bindValue(":name",  userName);
    query.bindValue(":email", email);
    query.exec();
}

void DAO::updateQuestion(const QString& question, int apiID)
{
    if(question.isEmpty())
        return;

    // update the ask count of the existing question
    QSqlQuery query;
    int questionID = getQuestionID(question);
    if(questionID >= 0)
    {
        query.exec(tr("update Questions set AskCount = AskCount + 1 where ID = %1")
                   .arg(questionID));
        updateLead(questionID);
        return;
    }

    // or insert a new question
    query.prepare("insert into Questions values (:id, :question, 1, -1)");
    query.bindValue(":id",       getNextID("Questions"));
    query.bindValue(":question", question);
    query.exec();

    measureSimilarity(question, apiID);  // initiate measure
}

void DAO::measureSimilarity(const QString& question, int apiID)
{
    // find the lead questions the API has
    QSqlQuery query;
    query.exec(tr("select Question from QuestionAboutAPI, Questions\
                   where APIID = %1 and QuestionID = ID and Parent = -1").arg(apiID));

    // compare this question with each lead question
    while(query.next())
        _comparer->compare(query.value(0).toString(), question);
}

void DAO::onComparisonResult(const QString& leadQuestion,
                             const QString& question, qreal similarity)
{
    QSettings settings("FAQServer.ini", QSettings::IniFormat);
    double threshold = qMax(settings.value("SimilarityThreshold").toDouble(), 0.5);
    if(similarity <= threshold)
        return;

    // set lead question to be the parent of question if similar
    QSqlQuery query;
    query.exec(tr("update Questions set Parent = %1 where ID = %2")
               .arg(getQuestionID(leadQuestion))
               .arg(getQuestionID(question)));
}

void DAO::updateLead(int questionID)
{
    // get lead id and my ask count
    QSqlQuery query;
    query.exec(tr("select Parent, AskCount from Questions where ID = %1 and Parent <> -1")
               .arg(questionID));
    if(!query.next())   // questionID is the lead
        return;

    int leadID    = query.value(0).toInt();
    int thisCount = query.value(1).toInt();   // my ask count

    // ask count of the lead question
    query.exec(tr("select AskCount from Questions where ID = %1").arg(leadID));
    int leadCount = query.next() ? query.value(0).toInt() : 0;

    // cannot beat lead
    if(thisCount <= leadCount)
        return;

    // all children of lead now become my chidren
    query.exec(tr("update Questions set Parent = %1 where Parent = %2")
               .arg(questionID).arg(leadID));

    // lead is now my child
    query.exec(tr("update Questions set Parent = %1 where ID = %2")
               .arg(questionID).arg(leadID));

    // I'm nobody's child
    query.exec(tr("update Questions set Parent = -1 where ID = %1")
               .arg(questionID));
}

void DAO::updateAnswer(const QString& link, const QString& title)
{
    if(link.isEmpty())
        return;

    // update existing answer or insert a new one
    QSqlQuery query;
    int id = getAnswerID(link);
    if(id > 0) {
        query.prepare("update Answers set Link = :link, Title = :title where ID = :id");
        query.bindValue(":id", id);
    }
    else {
        query.prepare("insert into Answers values (:id, :link, :title)");
        query.bindValue(":id", getNextID("Answers"));
    }
    query.bindValue(":link",  link);
    query.bindValue(":title", title);
    query.exec();
}

void DAO::updateQuestionUserRelation(int groupID, int userID)
{
    if(groupID < 0 || userID < 0)
        return;

    QSqlQuery query;
    query.exec(tr("delete from UserAskQuestion where GroupID = %1 and UserID = %2")
               .arg(groupID)
               .arg(userID));
    query.exec(tr("insert into UserAskQuestion values (%1, %2)")
               .arg(groupID)
               .arg(userID));
}

void DAO::updateQuestionAPIRelation(int groupID, int apiID)
{
    if(groupID < 0 || apiID < 0)
        return;

    QSqlQuery query;
    query.exec(tr("delete from QuestionAboutAPI where GroupID = %1 and APIID = %2")
               .arg(groupID)
               .arg(apiID));
    query.exec(tr("insert into QuestionAboutAPI values (%1, %2)")
               .arg(groupID)
               .arg(apiID));
}

void DAO::updateQuestionAnswerRelation(int groupID, int answerID)
{
    if(groupID < 0 || answerID < 0)
        return;

    QSqlQuery query;
    query.exec(tr("delete from AnswerToQuestion where GroupID = %1 and AnswerID = %2")
               .arg(groupID)
               .arg(answerID));
    query.exec(tr("insert into AnswerToQuestion values (%1, %2)")
               .arg(groupID)
               .arg(answerID));
}

void DAO::save(const QString& userName, const QString& email, const QString& apiSig,
               const QString& question, const QString& link,  const QString& title)
{
    updateUser  (userName, email);
    updateAPI   (apiSig);
    updateAnswer(link, title);

    int apiID = getAPIID(apiSig);
    updateQuestion(question, apiID);

    int answerID   = getAnswerID  (link);
    int userID     = getUserID    (userName);
    int questionID = getQuestionID(question);
    updateQuestionUserRelation  (questionID, userID);
    updateQuestionAPIRelation   (questionID, apiID);
    updateQuestionAnswerRelation(questionID, answerID);
}

void DAO::logAPI(const QString& userName, const QString& email, const QString& apiSig)
{
    qDebug() << "Log link: " << userName << email << apiSig;

    updateUser(userName, email);
    updateAPI (apiSig);
    addUserReadAPI(getUserID(userName), getAPIID(apiSig));
}

void DAO::logAnswer(const QString& userName, const QString& email, const QString& link)
{
    qDebug() << "Log answer: " << userName << email << link;

    updateUser(userName, email);
    addUserReadQuestion(getUserID(userName), getAnswerID(link));
}

void DAO::addUserReadAPI(int userID, int apiID)
{
    QSqlQuery query;
    query.prepare("insert into UserReadAPI values (:userID, :apiID, :time)");
    query.bindValue(":userID", userID);
    query.bindValue(":apiID",  apiID);
    query.bindValue(":time",   getCurrentDateTime());
    query.exec();
}

void DAO::addUserReadQuestion(int userID, int answerID)
{
    // find the question id associated with the answer
    QSqlQuery query;
    query.exec(tr("select QuestionID from AnswerToQuestion where AnswerID = %1")
               .arg(answerID));
    if(query.next())
    {
        int questionID = query.value(0).toInt();

        // add a UserReadQuestion record
        query.prepare("insert into UserReadQuestion values (:userID, :questionID, :time)");
        query.bindValue(":userID",     userID);
        query.bindValue(":questionID", questionID);
        query.bindValue(":time",       getCurrentDateTime());
        query.exec();
    }
}

QString DAO::getCurrentDateTime() const {
    return QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
}

// An example of returned JSON array:
//[
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
//			"answers" : []
//		},
//	]
//},
//{...}
//]
QJsonDocument DAO::query(const QString& classSig) const
{
    QJsonArray apisJson;
    QSqlQuery query;
    query.exec(tr("select ID, Signature from APIs where Signature like \'%1%\'").arg(classSig));

    // for all the classes
    while(query.next())
    {
        int apiID = query.value(0).toInt();
        QJsonArray questions = createQuestionsJason(apiID);  // questions about this API
        if(!questions.isEmpty())
        {
            QString apiSig = query.value(1).toString().section(";", -1, -1);  // remove library
            QJsonObject apiJson;
            apiJson.insert("apisig",    apiSig);
            apiJson.insert("questions", questions);
            apisJson.append(apiJson);
        }
    }

    return QJsonDocument(apisJson);
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

QJsonArray DAO::createAnswersJson(const QStringList& questionIDs) const
{
    QJsonArray result;
    QSqlQuery query;
    query.exec(tr("select distinct AnswerID from AnswerToQuestion\
                   where QuestionID in (%1)").arg(questionIDs.join(",")));
    while(query.next())
    {
        QJsonObject answerJson = createAnswerJson(query.value(0).toInt());
        if(!answerJson.isEmpty())
            result.append(answerJson);
    }
    return result;
}

QJsonArray DAO::createUsersJson(const QStringList& questionIDs) const
{
    QJsonArray result;
    QSqlQuery query;
    query.exec(tr("select distinct UserID from UserAskQuestion \
                   where QuestionID in (%1) \
                   union \
                   select distinct UserID from UserReadQuestion \
                   where QuestionID in (%1)").arg(questionIDs.join(",")));
    while(query.next())
        result.append(createUserJson(query.value(0).toInt()));
    return result;
}

QJsonObject DAO::createQuestionJason(int leadID) const
{
    QJsonObject result;
    QSqlQuery query;
    query.exec(tr("select Question from Questions where ID = %1").arg(leadID));
    if(query.next())
    {
        result.insert("question", query.value(0).toString());  // lead question

        // all questions in this group
        QStringList questionIDs;
        query.exec(tr("select ID from Questions \
                       where Parent = %1 or ID = %1").arg(leadID));
        while(query.next())
            questionIDs << query.value(0).toString();

        result.insert("users",   createUsersJson  (questionIDs));
        result.insert("answers", createAnswersJson(questionIDs));
    }

    return result;
}

QJsonArray DAO::createQuestionsJason(int apiID) const
{
    QJsonArray result;

    // find all lead questions
    QSqlQuery query;
    query.exec(tr("select QuestionID from QuestionAboutAPI, Questions\
                   where QuestionID = ID and Parent = -1 and APIID = %1").arg(apiID));
    while(query.next())
        result.append(createQuestionJason(query.value(0).toInt()));  // question json
    return result;
}

QJsonDocument DAO::personalProfile(const QString& userName) const
{
    int userID = getUserID(userName);
    if(userID == -1)
        return QJsonDocument();

    // this person's profile
    QJsonObject profileJson;
    profileJson.insert("name", userName);
    QSqlQuery query;
    query.exec(tr("select Email from Users where ID = %1").arg(userID));
    if(query.next())
        profileJson.insert("email", query.value(0).toString());

    // get all the questions userID relates to
    // 1. get all the lead questions asked  by userID
    // 2. get all the lead questions viewed by userID
    // 3. merge (union) 1 and 2
    query.exec(tr("select QuestionID from UserAskQuestion, Questions \
                     where QuestionID = ID and Parent = -1 and UserID = %1 \
                   union \
                   select QuestionID from UserReadQuestion, Questions \
                     where QuestionID = ID and Parent = -1 and UserID = %1 \
                   order by QuestionID").arg(userID));
    QStringList questions;
    while(query.next())
        questions << query.value(0).toString();

    // get all the APIs associated with the questions
    query.exec(tr("select distinct ID, Signature from APIs, QuestionAboutAPI \
                  where ID = APIID and QuestionID in (%1)").arg(questions.join(",")));

    QJsonArray apisJson;
    while(query.next())
    {
        int     apiID  = query.value(0).toInt();
        QString apiSig = query.value(1).toString().section(";", -1, -1);  // remove library
        QJsonObject apiJson;                                          // json for this api
        apiJson.insert("apisig",    apiSig);                          // save api to json
        apiJson.insert("questions", createQuestionsJason(apiID));     // save questions to json
        apisJson.append(apiJson);                                     // add this json to api json array
    }
    profileJson.insert("apis", apisJson);   // add apis

    // get all other users associated with the questions
    query.exec(tr("select Name, Email from UserAskQuestion, Users \
                     where QuestionID in (%1) and UserID = ID and UserID != %2 \
                   union \
                   select Name, Email from UserReadQuestion, Users \
                     where QuestionID in (%1) and UserID = ID and UserID != %2")
               .arg(questions.join(",")).arg(userID));

    QJsonArray usersJson;
    while(query.next())
    {
        QString name  = query.value(0).toString();
        QString email = query.value(1).toString();
        QJsonObject userJson;             // json for this user
        userJson.insert("name",  name);
        userJson.insert("email", email);
        usersJson.append(userJson);       // add this json to users json array
    }
    profileJson.insert("relatedusers", usersJson);   // add related users

    return QJsonDocument(profileJson);
}
