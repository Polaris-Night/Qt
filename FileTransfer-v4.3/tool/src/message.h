#ifndef MESSAGE_H
#define MESSAGE_H

#include <QString>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonParseError>

#include <type_traits>

struct SerializeApi {
    virtual ~SerializeApi() = default;

    // 根据对象序列化
    virtual void toJson(QJsonObject &) const = 0;
    virtual void fromJson(const QJsonObject &) = 0;
    // 根据数据序列化
    virtual QByteArray toJson() const { return QByteArray(); }
    virtual void fromJson(const QByteArray &){};
};

template <typename T> struct Message : public SerializeApi {
    static_assert(std::is_base_of<SerializeApi, T>::value, "Template argument T must inherit from SerializeApi");

    Message() = default;
    Message(int code, const QString &msg, const T &data)
    : code(code)
    , msg(msg)
    , data(data) { }

    void toJson(QJsonObject &j) const override {
        j.insert("code", code);
        j.insert("msg", msg);
        QJsonObject dataJson;
        data.toJson(dataJson);
        j.insert("data", dataJson);
    }
    void fromJson(const QJsonObject &j) override {
        code = j.value("code").toInt(-1);
        msg = j.value("msg").toString();
        QJsonObject dataJson = j.value("data").toObject();
        data.fromJson(dataJson);
    }
    QByteArray toJson() const override {
        QJsonObject j;
        j.insert("code", code);
        j.insert("msg", msg);
        QJsonObject dataJson;
        data.toJson(dataJson);
        j.insert("data", dataJson);
        return QJsonDocument(j).toJson(QJsonDocument::Compact);
    }
    void fromJson(const QByteArray &j) override {
        QJsonParseError e;
        auto doc = QJsonDocument::fromJson(j, &e);
        if (e.error != QJsonParseError::NoError || doc.isNull() || !doc.isObject()) {
            return;
        }
        auto obj = doc.object();
        code = obj.value("code").toInt(-1);
        msg = obj.value("msg").toString();
        data.fromJson(obj.value("data").toObject());
    }

    int code{-1};
    QString msg;
    T data;
};

// 特化void类型，无参数
template <> struct Message<void> : public SerializeApi {
    Message() = default;
    Message(int code, const QString &msg)
    : code(code)
    , msg(msg) { }

    void toJson(QJsonObject &j) const override {
        j.insert("code", code);
        j.insert("msg", msg);
    }
    void fromJson(const QJsonObject &j) override {
        code = j.value("code").toInt(-1);
        msg = j.value("msg").toString();
    }

    QByteArray toJson() const override {
        QJsonObject j;
        j.insert("code", code);
        j.insert("msg", msg);
        return QJsonDocument(j).toJson(QJsonDocument::Compact);
    }
    void fromJson(const QByteArray &j) override {
        QJsonParseError e;
        auto doc = QJsonDocument::fromJson(j, &e);
        if (e.error != QJsonParseError::NoError || doc.isNull() || !doc.isObject()) {
            return;
        }
        auto obj = doc.object();
        code = obj.value("code").toInt(-1);
        msg = obj.value("msg").toString();
    }

    int code{-1};
    QString msg;
};

#endif // MESSAGE_H
