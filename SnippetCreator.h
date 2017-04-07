#ifndef HTMLCREATOR_H
#define HTMLCREATOR_H

class QByteArray;
class QJsonArray;
class QJsonObject;
class QJsonDocument;

// Convert Json to HTML snippet
// The process is to follow the structure of the json input (and target HTML), and
// iteratively fill in the templates
class SnippetCreator
{
public:
    QJsonDocument createFAQs       (const QJsonArray&  jaAPIs)    const;  // for query
    QByteArray    createProfilePage(const QJsonObject& joProfile) const;

private:
    QByteArray createFAQ           (const QJsonObject& joAPI)     const;
    QByteArray createQuestions     (const QJsonObject& joAPI)     const;
    QByteArray createProfileSection(const QJsonObject& joProfile) const;
    QByteArray createInterestedAPIs(const QJsonObject& joProfile) const;
    QByteArray createRelatedUsers  (const QJsonObject& joProfile) const;
    QByteArray createUser          (const QJsonObject& joUser)    const;
};

#endif // HTMLCREATOR_H
