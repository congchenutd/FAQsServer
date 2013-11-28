#include "SnippetCreator.h"
#include "Template.h"
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QTextStream>
#include <QDebug>

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
//}
//]
//
//[
//{
//	"api": "java.util.ArrayList.ensureCapacity",
//	"html": "..."
//}
//]
QJsonDocument SnippetCreator::createFAQs(const QJsonArray& jaAPIs) const
{
    QJsonArray jaFAQs;
    for(QJsonArray::ConstIterator it = jaAPIs.begin(); it != jaAPIs.end(); ++it)
    {
        QJsonObject joAPI = (*it).toObject();
        QJsonObject joFAQ;
        joFAQ.insert("apisig", joAPI.value("apisig").toString());
        joFAQ.insert("html",   QString(createFAQ(joAPI)));
        jaFAQs.append(joFAQ);
    }
    return QJsonDocument(jaFAQs);
}

QByteArray SnippetCreator::createFAQ(const QJsonObject& joAPI) const
{
    Template tTitle("./Templates/FAQ.html");
    tTitle.setValue("Questions", createQuestions(joAPI));
    return tTitle.toHTML();
}

// e.g.
//{
//    "api": "javax.swing.AbstractAction.getValue(java.lang.String)",
//    "questions": [
//        {
//            "answers": [
//                {
//                    "link": "http://www.scribd.com/",
//                    "title": "Wrox.professional Java JDK Edition"
//                }
//            ],
//            "question": "question1",
//            "users": [
//                {
//                    "email": "carl@gmail.com",
//                    "name": "Carl"
//                }
//            ]
//        }
//    ]
//}
//
//<ul>
//	<li>question1 ( <a href="profile:Carl">Carl</a> )
//		<ul>
//			<li><a href="http://www.scribd.com/">Wrox.professional Java JDK Edition"</a></li>
//		</ul>
//	</li>
//</ul>
QByteArray SnippetCreator::createQuestions(const QJsonObject& joAPI) const
{
    QJsonArray jaQuestions = joAPI.value("questions").toArray();
    Template tQuestions("./Templates/Questions.html");

    // for each question
    for(QJsonArray::Iterator itq = jaQuestions.begin(); itq != jaQuestions.end(); ++itq)
    {
        // the question itself
        QJsonObject joQuestion = (*itq).toObject();
        Template tQuestion("./Templates/Question.html");
        tQuestion.setValue("Title", joQuestion.value("question").toString());

        // users
        QJsonArray users = joQuestion.value("users").toArray();
        for(QJsonArray::Iterator itu = users.begin(); itu != users.end(); ++itu)
        {
            QJsonObject joUser = (*itu).toObject();
            Template tUser("./Templates/User.html");
            tUser.setValue("Name", joUser.value("name").toString());  // format user
            tQuestion.addValue("User", tUser.toHTML());               // add to the question
        }

        // answers
        QJsonArray joAnswers = joQuestion.value("answers").toArray();
        if(joAnswers.isEmpty())
        {
            Template tAnswer("./Templates/Answer.html");
            tAnswer.setValue("Title", "Not answered!");
            tQuestion.addValue("Answer", tAnswer.toHTML());
        }
        else {
            for(QJsonArray::Iterator ita = joAnswers.begin(); ita != joAnswers.end(); ++ita)
            {
                QJsonObject joAnswer = (*ita).toObject();
                QString link  = joAnswer.value("link") .toString();
                QString title = joAnswer.value("title").toString();
                if(title.isEmpty())
                    title = "Link";

                Template tAnswer("./Templates/Answer.html");
                tAnswer.setValue("Title", title);                // format answer
                tAnswer.setValue("Link",  link);
                tQuestion.addValue("Answer", tAnswer.toHTML());  // add to the question
            }
        }
        tQuestions.addValue("Question", tQuestion.toHTML());     // add the question
    }
    return tQuestions.toHTML();
}

QByteArray SnippetCreator::createProfilePage(const QJsonObject& joProfile) const
{
    Template tProfilePage("./Templates/ProfilePage.html");
    QString name = joProfile.value("name").toString();
    tProfilePage.setValue("Name",           name);
    tProfilePage.setValue("Profile",        createProfile     (joProfile));
    tProfilePage.setValue("InterestedAPIs", createAPIs        (joProfile));
    tProfilePage.setValue("RelatedUsers",   createRelatedUsers(joProfile));

    qDebug() << tProfilePage.toHTML();

    return tProfilePage.toHTML();
}

QByteArray SnippetCreator::createProfile(const QJsonObject& joProfile) const
{
    QString name  = joProfile.value("name").toString();
    QString email = joProfile.value("email").toString();
    Template tProfile("./Templates/Profile.html");
    tProfile.setValue("Name",  name);
    tProfile.setValue("Email", email);
    return tProfile.toHTML();
}

QByteArray SnippetCreator::createAPIs(const QJsonObject& joProfile) const
{
    Template tAPIs("./Templates/APIs.html");
    QJsonArray jaAPIs = joProfile.value("apis").toArray();
    for(QJsonArray::Iterator it = jaAPIs.begin(); it != jaAPIs.end(); ++it)
    {
        QJsonObject joAPI = (*it).toObject();
        QString apiSig = joAPI.value("apisig").toString();

        Template tAPI("./Templates/API.html");
        tAPI.setValue("Signature", apiSig);
        tAPI.setValue("Questions", createQuestions(joAPI));

        tAPIs.addValue("API", tAPI.toHTML());
    }
    return tAPIs.toHTML();
}

QByteArray SnippetCreator::createRelatedUsers(const QJsonObject& joProfile) const
{
    Template tUsers("./Templates/RelatedUsers.html");
    QJsonArray jaUsers = joProfile.value("relatedusers").toArray();
    for(QJsonArray::Iterator it = jaUsers.begin(); it != jaUsers.end(); ++it)
    {
        QJsonObject joUser = (*it).toObject();
        Template userTemp("./Templates/RelatedUser.html");
        userTemp.setValue("Name", joUser.value("name").toString());

        tUsers.addValue("RelatedUser", userTemp.toHTML());
    }
    return tUsers.toHTML();
}
