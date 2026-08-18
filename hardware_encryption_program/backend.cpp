#include "backend.h"

#include <string>
#include <windows.h>
#include <QFileDialog>
#include <QStandardPaths>
#include "crypto/hid.h"
#include "crypto/encrypted_file.h"
#include "sodium.h"
#include "shared_config.h"

const WORD VID = 0x2E8A;
const WORD PID = 0x464A;

const unsigned char DEVICE_PUBLIC_KEY[] = { 0x27, 0x56, 0x30, 0x1B, 0xF0, 0x3B, 0xFA, 0x28, 0xEB, 0xD1, 0x35, 0xFF, 0x48, 0xF2, 0xC5, 0x54, 0x6C, 0x98, 0x8C, 0x4C, 0x1D, 0x61, 0xA2, 0xB4, 0x07, 0x99, 0x4A, 0x40, 0xDB, 0x97, 0x31, 0x0B };

Backend::Backend(QObject *parent) : QObject(parent) {
    // m_hidItems << "HID 1" << "HID 2" << "HID 3";
    m_hid = std::make_unique<HID>(VID, PID);
    m_cryptoHidController = std::make_unique<CryptoHidController>(*m_hid);
}

QString Backend::currentFilePath() const {
    return m_currentFilePath;
}

void Backend::setCurrentFilePath(const QString &path) {
    if (m_currentFilePath != path) {
        m_currentFilePath = path;
        emit currentFilePathChanged();
    }
}

void Backend::setCurrentFilePath(const QUrl &url) {
    QString path = url.toLocalFile();

    setCurrentFilePath(path);
}

bool Backend::isNewFile() const {
    return m_currentFilePath.isEmpty();
}

QString Backend::fileContent() const {
    return m_fileContent;
}

void Backend::setFileContent(const QString &content) {
    if (m_fileContent != content) {
        m_fileContent = content;
        emit fileContentChanged();
    }
}

Q_INVOKABLE void Backend::createNewFile() {
    setCurrentFilePath(QUrl(""));
    setFileContent("");
}

Q_INVOKABLE void Backend::openFile(const QUrl &url) {
    setCurrentFilePath(url);
    std::string path = url.toLocalFile().toStdString();

    decryptFile();
}

void Backend::encrytpAndSaveFile() {
    std::string message = m_fileContent.toStdString();
    EncryptedData ed = encrypt_data(message, DEVICE_PUBLIC_KEY);

    if (m_currentFilePath.isEmpty()) {
        QString filePath = QFileDialog::getSaveFileName(
            nullptr,
            "Save as",
            QStandardPaths::writableLocation(QStandardPaths::DesktopLocation),
            "(*.lhef)"
            );

        if (filePath.isEmpty()) {
            return;
        }

        setCurrentFilePath(filePath);
    }

    if (save_file(m_currentFilePath.toStdString(), ed) != 0) {
        emit showAlert("Error", "Error while saving file");
        return;
    }

    emit showAlert("Success", "File encrypted and saved");
}

void Backend::decryptFile() {
    std::string path = m_currentFilePath.toStdString();
    EncryptedData ed;

    if (read_file(path, &ed) != 0) {
        emit showAlert("Error", "Error while reading file");
        return;
    }

    unsigned char pk[crypto_box_PUBLICKEYBYTES];
    unsigned char sk[crypto_box_SECRETKEYBYTES];
    crypto_box_keypair(pk, sk);

    std::string error_message;

    m_cryptoHidController->send_keys(ed.encrypted_key, SEALED_KEY_BYTESIZE, pk, crypto_box_PUBLICKEYBYTES, error_message);

    if (!error_message.empty()) {
        emit showAlert("Error", QString::fromStdString(error_message));
        return;
    }

    unsigned char readed_key[SEALED_KEY_BYTESIZE];
    m_cryptoHidController->read_key(readed_key, error_message);

    if (!error_message.empty()) {
        emit showAlert("Error", QString::fromStdString(error_message));
        return;
    }

    unsigned char decrypted_key[DECRYPTED_KEY_BYTESIZE];

    if (crypto_box_seal_open(decrypted_key, readed_key, SEALED_KEY_BYTESIZE, pk, sk) != 0) {
        emit showAlert("Error", "Message corrupted or not intended for this recipient");
        return;
    }

    std::string decrypted = decrypt_data(ed, decrypted_key, error_message);

    if (!error_message.empty()) {
        emit showAlert("Error", QString::fromStdString(error_message));
        return;
    }

    setFileContent(QString::fromStdString(decrypted));
}