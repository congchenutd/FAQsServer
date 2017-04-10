#include "SnippetCreator.h"
#include "Template.h"
#include "Settings.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QTextStream>
#include <QDebug>

//Examples:
//Input:
//{
//    "style": [link to the stylesheet]
//    "apis": [{
//                "api": "java.util.ArrayList.ensureCapacity",
//                "questions":
//                [{
//                    "question": "question1",
//                    "users"   : [{"username": "user1", "email": "user1@gmai.com"},
//                                 {"username": "user2", "email": "user2@gmail.com"}],
//                    "answers" : [{"link": "link1", "title": "title1"},
//                                 {"link": "link2", "title": "title2"}]
//                }]
//            }]
//}

//Output:
//{
//    "style": [link to the stylesheet]
//    "apis": [{
//                "api": "java.util.ArrayList.ensureCapacity",
//                "html": "..."
//            }]
//}

/**
 * Convert FAQs content into HTML
 * @param jaAPIs    - json array representing the content of the FAQs of an API
 * @return          - an json document containing corresponding HTML
 */
QJsonDocument SnippetCreator::createFAQs(const QJsonArray& jaAPIs) const
{
    QJsonObject joDocPage;
    Settings* settings = Settings::getInstance();

    // stylesheet
    joDocPage.insert("style", QObject::tr("http://%1:%2/Templates/faqs.css")
                                        .arg(settings->getServerIP())
                                        .arg(settings->getServerPort()));

    // FAQs -> HTML
    QJsonArray jaFAQs;
    for(QJsonArray::ConstIterator it = jaAPIs.begin(); it != jaAPIs.end(); ++it)
    {
        QJsonObject joAPI = (*it).toObject();
        QJsonObject joFAQ;
        joFAQ.insert("apisig", joAPI.value("apisig").toString());

        QString html = QString(createFAQ(joAPI));
        joFAQ.insert("html",   html);
        jaFAQs.append(joFAQ);
    }

    joDocPage.insert("apis", jaFAQs);

    qDebug() << joDocPage;
    return QJsonDocument(joDocPage);
}

/**
 * Convert a json object representing an API and its FAQs into HTML
 * @param joAPI - a json object representing an API and its FAQs
 * @return      - a QByteArray representing the content of the corresponding FAQs section HTML code
 */
QByteArray SnippetCreator::createFAQ(const QJsonObject& joAPI) const
{
    Template tTitle("./Templates/FAQ.html");
    tTitle.setValue("Questions", createQuestions(joAPI));
    return tTitle.toHTML();
}

// e.g.
//Input:
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
//Output:
//<ul>
//	<li>question1 ( <a href="profile:Carl">Carl</a> )
//		<ul>
//			<li><a href="http://www.scribd.com/">Wrox.professional Java JDK Edition"</a></li>
//		</ul>
//	</li>
//</ul>

/**
 * Do the actual work of createFAQ()
 */
QByteArray SnippetCreator::createQuestions(const QJsonObject& joAPI) const
{
    QJsonArray jaQuestions = joAPI.value("questions").toArray();
    Template tQuestions("./Templates/Questions.html");

    tQuestions.setValue("StyleSheet", "http://localhost:8080/Templates/FAQs.css");

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
            Template tInterestedUser("./Templates/InterestedUser.html");
            tInterestedUser.setValue("User", createUser(joUser));
            tQuestion.addValue("InterestedUser", tInterestedUser.toHTML());
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

/**
 * Create profile page HTML
 */
QByteArray SnippetCreator::createProfilePage(const QJsonObject& joProfile) const
{
    Template tProfilePage("./Templates/ProfilePage.html");
    if(!tProfilePage.isLoaded())
        return "Template not loaded!";

    Settings* settings = Settings::getInstance();
    tProfilePage.setValue("StyleSheet", QObject::tr("http://%1:%2/Templates/ProfilePage.css")
                                            .arg(settings->getServerIP())
                                            .arg(settings->getServerPort()));

    QString name = joProfile.value("name").toString();
    tProfilePage.setValue("Name",           name);
    tProfilePage.setValue("ProfileSection", createProfileSection(joProfile));
    tProfilePage.setValue("InterestedAPIs", createInterestedAPIs(joProfile));
    tProfilePage.setValue("RelatedUsers",   createRelatedUsers  (joProfile));
    return tProfilePage.toHTML();
}

/**
 * Create the profile section of a profile page
 */
QByteArray SnippetCreator::createProfileSection(const QJsonObject& joProfile) const
{
    QString name  = joProfile.value("name") .toString();
    QString email = joProfile.value("email").toString();
    Template tProfile("./Templates/ProfileSection.html");
    tProfile.setValue("Name",  name);
    tProfile.setValue("Email", email);
    return tProfile.toHTML();
}

/**
 * Create the interested APIs section of a profile page
 */
QByteArray SnippetCreator::createInterestedAPIs(const QJsonObject& joProfile) const
{
    Template tAPIs("./Templates/InterestedAPIs.html");
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

/**
 * Create the related users section of a profile page
 */
QByteArray SnippetCreator::createRelatedUsers(const QJsonObject& joProfile) const
{
    Template tUsers("./Templates/RelatedUsers.html");
    QJsonArray jaUsers = joProfile.value("relatedusers").toArray();
    for(QJsonArray::Iterator it = jaUsers.begin(); it != jaUsers.end(); ++it)
    {
        QJsonObject joUser = (*it).toObject();
        Template tRelatedUser("./Templates/RelatedUser.html");
        tRelatedUser.setValue("User", createUser(joUser));
        tUsers.addValue("RelatedUser", tRelatedUser.toHTML());
    }
    return tUsers.toHTML();
}

/**
 * Create the user section of the related users section of a profile page
 */
QByteArray SnippetCreator::createUser(const QJsonObject& joUser) const
{
    QString userName = joUser.value("name").toString();
    Template tUser("./Templates/User.html");
    tUser.setValue("Name", userName);

    Settings* settings = Settings::getInstance();
    tUser.setValue("Photo", QObject::tr("http://%1:%2/Photos/%3.png")
                                    .arg(settings->getServerIP())
                                    .arg(settings->getServerPort())
                                    .arg(userName));
    tUser.setValue("StyleSheet", QObject::tr("http://%1:%2/Templates/Thumbnail.css")
                                            .arg(settings->getServerIP())
                                            .arg(settings->getServerPort()));
    return tUser.toHTML();
}
