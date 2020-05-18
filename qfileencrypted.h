#ifndef QFILEENCRYPTED_H
#define QFILEENCRYPTED_H

#include <QObject>
#include <QFile>

#include <openssl/evp.h>
#include <openssl/err.h>

/**
 * @brief The QFileEncrypted class can act as a drop in replacement for QFile, or any QIODevice, but then with encryption, using AES256.
 *
 * There are some caveats. Encryption requires padding to align blocks with the encryption key. Data is retained in buffers so we write
 * to disk with the proper padding size. This padding is writting to disk on close, and it's read on object instantiation. All this
 * makes it hard to implement a class that supports reading and writing at the same time.
 *
 * Seeking is not tested, and will probably not work. I have some ideas how to implement it, but I didn't need it so far, so I
 * haven't... At then, it's probably only doable for reading.
 */
class QFileEncrypted : public QFile
{
    Q_OBJECT

    EVP_CIPHER_CTX *writeCipherContext;
    EVP_CIPHER_CTX *readCipherContext;
    QByteArray cipherKey;
    QByteArray iv;
    int cryptBlockSize;

    char *writeCirBuf = nullptr;
    qint64 writeBufWPos = 0;
    qint64 writeBufRPos = 0;

    char *writeCryptBufferIn = nullptr;
    char *writeCryptBufferOut = nullptr;
    qint64 writeCryptBufferSize = 0;

    char *readCirBuf = nullptr;
    qint64 readBufWPos = 0;
    qint64 readBufRPos = 0;

    char *readCryptBufferIn = nullptr;
    char *readCryptBufferOut = nullptr;
    qint64 readCryptBufferSize = 0;

    qint64 decryptedSize = 0;
    int paddingSize = -1;

    void resizeWriteCryptBuffer(qint64 size);
    void resizeReadCryptBuffer(qint64 size);
    void setPaddingSize();
    qint64 getWriteBufUsedBytes() const;
    qint64 getReadBufUsedBytes() const;

protected:
    qint64 readData(char *data, qint64 maxSize) override;
    qint64 writeData(const char *data, qint64 maxSize) override;

    void throwError[[noreturn]]() const
    {
        unsigned long error = ERR_get_error();
        QByteArray r(128, 0);
        ERR_error_string(error, r.data());
        throw std::runtime_error(r.toStdString());
    }

public:
    QFileEncrypted(const QString &name, const QByteArray &iv, const QByteArray &cipherKey, QObject *parent = nullptr);
    ~QFileEncrypted() override;

    void close() override;
    qint64 size() const override;
    bool atEnd() const override;
    bool open(QIODevice::OpenMode mode) override;
};

#endif // QFILEENCRYPTED_H
