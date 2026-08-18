#ifndef BACKEND_H
#define BACKEND_H

#include <QObject>
#include <QStringList>
#include <QDebug>
#include <QUrl>
#include <memory>

#include "crypto/crypto_hid_controller.h"

class Backend : public QObject
{
    Q_OBJECT
    // Q_PROPERTY(QStringList hidItems READ hidItems NOTIFY hidItemsChanged)
    Q_PROPERTY(QString currentFilePath READ currentFilePath WRITE setCurrentFilePath NOTIFY currentFilePathChanged)
    Q_PROPERTY(bool isNewFile READ isNewFile NOTIFY currentFilePathChanged)
    Q_PROPERTY(QString fileContent READ fileContent WRITE setFileContent NOTIFY fileContentChanged);

private:
    QString m_currentFilePath = "";
    std::unique_ptr<HID> m_hid;
    std::unique_ptr<CryptoHidController> m_cryptoHidController;
    QString m_fileContent = "";

    void decryptFile();

signals:
    void currentFilePathChanged();
    void fileContentChanged();
    void showAlert(const QString &title, const QString &message);

public:
    explicit Backend(QObject *parent = nullptr);

    QString currentFilePath() const;
    void setCurrentFilePath(const QString &path);
    void setCurrentFilePath(const QUrl &url);
    bool isNewFile() const;
    QString fileContent() const;
    void setFileContent(const QString &content);

    Q_INVOKABLE void createNewFile();
    Q_INVOKABLE void openFile(const QUrl &url);
    Q_INVOKABLE void encrytpAndSaveFile();
};

#endif // BACKEND_H
