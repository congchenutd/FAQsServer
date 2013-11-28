#ifndef HTMLCREATOR_H
#define HTMLCREATOR_H

class QByteArray;
class QJsonArray;
class QJsonObject;
class QJsonDocument;

// Convert Json to HTML snippet
class SnippetCreator
{
public:
    QJsonDocument createFAQs       (const QJsonArray&  jaAPIs)    const;
    QByteArray    createFAQ        (const QJsonObject& joAPI)     const;
    QByteArray    createProfilePage(const QJsonObject& joProfile) const;

private:
    QByteArray createQuestions   (const QJsonObject& joAPI)     const;
    QByteArray createProfile     (const QJsonObject& joProfile) const;
    QByteArray createAPIs        (const QJsonObject& joProfile) const;
    QByteArray createRelatedUsers(const QJsonObject& joProfile) const;
};

#endif // HTMLCREATOR_H
