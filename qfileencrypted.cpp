/**
  * This file is part of PasswordSender.
  *
  * PasswordSender is free software: you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation, either version 3 of the License, or
  * (at your option) any later version.

  * PasswordSender is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  * GNU General Public License for more details.
  *
  * You should have received a copy of the GNU General Public License
  * along with PasswordSender. If not, see <https://www.gnu.org/licenses/>.
  *
  * Copyright 2020 Wiebe Cazemier <wiebe@halfgaar.net>
  */

#include "qfileencrypted.h"
#include "iostream"

void QFileEncrypted::resizeWriteCryptBuffer(qint64 size)
{
    if (size <= this->writeCryptBufferSize)
        return;

    if (this->writeCryptBufferSize > 0)
    {
        delete[] this->writeCryptBufferIn;
        this->writeCryptBufferIn = nullptr;

        delete[] this->writeCryptBufferOut;
        this->writeCryptBufferOut = nullptr;
    }



    this->writeCryptBufferIn = new char[size];
    memset(this->writeCryptBufferIn, 0, static_cast<size_t>(size));

    this->writeCryptBufferOut = new char[size];
    memset(this->writeCryptBufferOut, 0, static_cast<size_t>(size));

    // Now copy the contents of the cir buffer to the new one.

    const qint64 bytesInCirBuffer = getWriteBufUsedBytes();
    char *newBuf = new char[size];
    for (qint64 i = 0; i < bytesInCirBuffer; i++)
    {
        newBuf[i] = writeCirBuf[writeBufRPos];
        writeBufRPos = (writeBufRPos + 1) % writeCryptBufferSize;
        writeBufWPos = (i + 1) % readCryptBufferSize;
    }
    writeBufRPos = 0;

    this->writeCryptBufferSize = size;

    delete[] this->writeCirBuf;
    this->writeCirBuf = newBuf;
}

void QFileEncrypted::resizeReadCryptBuffer(qint64 size)
{
    if (size <= this->readCryptBufferSize)
        return;

    if (this->readCryptBufferSize > 0)
    {
        delete[] this->readCryptBufferIn;
        this->readCryptBufferIn = nullptr;

        delete[] this->readCryptBufferOut;
        this->readCryptBufferOut = nullptr;
    }

    this->readCryptBufferIn = new char[size];
    memset(this->readCryptBufferIn, 0, static_cast<size_t>(size));

    this->readCryptBufferOut = new char[size];
    memset(this->readCryptBufferOut, 0, static_cast<size_t>(size));

    // Now copy the contents of the cir buffer to the new one.

    const qint64 bytesInCirBuffer = getReadBufUsedBytes();
    char *newBuf = new char[size];
    for (qint64 i = 0; i < bytesInCirBuffer; i++)
    {
        newBuf[i] = readCirBuf[readBufRPos];
        readBufRPos = (readBufRPos + 1) % readCryptBufferSize;
        readBufWPos = (i + 1) % readCryptBufferSize;
    }
    readBufRPos = 0;

    this->readCryptBufferSize = size;

    delete[] this->readCirBuf;
    this->readCirBuf = newBuf;
}


/**
 * @brief QFileEncrypted::getPaddingSize gets the size of the PKCS adding by the encryption.
 * @return
 *
 * This is a bootstrapping method, to be used in the constructor, to get around a bit of a catch-22. In other words, the
 * QFileEncrypted instantiated in the function will report the wrong size.
 */
void QFileEncrypted::setPaddingSize()
{
    QFile f(this->fileName());

    if (!f.exists())
        throw std::runtime_error(QString("%1 doesn't exist. This function only works in read-only mode.").arg(this->fileName()).toStdString());

    if (f.size() < (this->cryptBlockSize * 2))
        return;

    f.open(QFile::ReadOnly);
    qint64 secondTwoLastBlockPos = f.size() - (this->cryptBlockSize * 2);

    f.seek(secondTwoLastBlockPos);

    QByteArray iv(this->cryptBlockSize, 0);
    f.read(iv.data(), this->cryptBlockSize);

    QByteArray lastBlockEncrypted(this->cryptBlockSize, 0);
    f.read(lastBlockEncrypted.data(), this->cryptBlockSize);

    QByteArray lastBlockDecrypted(this->cryptBlockSize, 0);

    // Just using this for the bootstrap. It will report an invalid size for the decrypted data.
    QFileEncrypted f2(this->fileName(), iv, this->cipherKey, this);

    int outl = 0;
    if (!EVP_DecryptUpdate(f2.readCipherContext, reinterpret_cast<unsigned char *>(lastBlockDecrypted.data()), &outl,
                      reinterpret_cast<const unsigned char *>(lastBlockEncrypted.constData()), lastBlockEncrypted.length()))
    {
        throwError();
    }

    if (outl != this->cryptBlockSize)
        throw std::runtime_error("Getting the last block has outl != blocksize");

    this->paddingSize = static_cast<int>(lastBlockDecrypted.at(this->cryptBlockSize - 1));

    for (int i = this->cryptBlockSize - this->paddingSize; i < this->cryptBlockSize; i++)
    {
        int b = lastBlockDecrypted[i];
        if (b != this->paddingSize)
            throw std::runtime_error("Padding error in decoding");
    }

    this->decryptedSize = QFile::size() - paddingSize;
}

qint64 QFileEncrypted::getWriteBufUsedBytes() const
{
    qint64 result = 0;
    if (writeBufWPos >= writeBufRPos)
        result = writeBufWPos - writeBufRPos;
    else
        result = (writeCryptBufferSize + writeBufWPos) - writeBufRPos;

    return result;
}

qint64 QFileEncrypted::getReadBufUsedBytes() const
{
    qint64 result = 0;
    if (readBufWPos >= readBufRPos)
        result = readBufWPos - readBufRPos;
    else
        result = (readCryptBufferSize + readBufWPos) - readBufRPos;

    return result;
}

qint64 QFileEncrypted::readData(char *data, qint64 maxSize)
{
    // Safety net, also against losing precision in casts below.
    if (maxSize > (10*1024*1024))
        throw std::runtime_error("Reading a buffer more than 10 MB into QFileEncrypted is not supported, and probably not what you meant.");

    const int desiredBufferSize = maxSize + this->cryptBlockSize*2;
    if (desiredBufferSize > this->readCryptBufferSize)
        resizeReadCryptBuffer(desiredBufferSize);

    if (pos() + maxSize > size())
    {
        maxSize = size() - pos();
    }

    const int rem = maxSize  % cryptBlockSize;
    qint64 newLen = maxSize;
    if (rem > 0)
        newLen = maxSize + (cryptBlockSize - rem);

    const qint64 bytesRead = QFile::readData(this->readCryptBufferIn, newLen);

    // Not sure if this is correct.
    if (bytesRead < 0)
        return bytesRead;

    int outl = 0;
    unsigned char *pointerDest = reinterpret_cast<unsigned char*>(this->readCryptBufferOut);
    const unsigned char *pointerSrc = reinterpret_cast<const unsigned char*>(this->readCryptBufferIn);
    if (!EVP_EncryptUpdate(this->readCipherContext, pointerDest, &outl, pointerSrc, static_cast<int>(bytesRead)))
        throwError();

    // Read more data then we need to align the blocks and put it in the circular buffer.
    for (qint64 i = 0; i < maxSize; i++)
    {
        this->readCirBuf[readBufWPos] = this->readCryptBufferOut[i];
        readBufWPos = (readBufWPos + 1) % this->readCryptBufferSize;
    }

    // Then give the amount of requested bytes from the circular buffer to the caller.
    for (qint64 i = 0; i < bytesRead; i++)
    {
        data[i] = this->readCirBuf[readBufRPos];
        readBufRPos = (readBufRPos + 1) % this->readCryptBufferSize;
    }

    return maxSize;
}

qint64 QFileEncrypted::writeData(const char *data, qint64 maxSize)
{
    // Safety net, also against losing precision in casts below.
    if (maxSize > (10*1024*1024))
        throw std::runtime_error("Writing a buffer more than 10 MB into QFileEncrypted is not supported, and probably not what you meant.");

    const int desiredBufferSize = maxSize + this->cryptBlockSize*2;
    if (desiredBufferSize > this->writeCryptBufferSize)
        resizeWriteCryptBuffer(desiredBufferSize);

    // We write our data to the circular buffer and read from it later in block sizes that the crypto routines demand.
    for (qint64 i = 0; i < maxSize; i++)
    {
        this->writeCirBuf[writeBufWPos] = data[i];
        writeBufWPos = (writeBufWPos + 1) % this->writeCryptBufferSize;
    }

    const qint64 bufUsed = getWriteBufUsedBytes();
    const int rem = bufUsed  % cryptBlockSize;
    const qint64 newLen = bufUsed - rem;

    if (newLen > 0)
    {
        for (qint64 i = 0; i < newLen; i++)
        {
            this->writeCryptBufferIn[i] = writeCirBuf[writeBufRPos];
            writeBufRPos = (writeBufRPos + 1) % writeCryptBufferSize;
        }

        int outl = 0;
        unsigned char *pointerDest = reinterpret_cast<unsigned char*>(this->writeCryptBufferOut);
        const unsigned char *pointerSrc = reinterpret_cast<const unsigned char*>(this->writeCryptBufferIn);
        if (!EVP_EncryptUpdate(this->writeCipherContext, pointerDest, &outl, pointerSrc, static_cast<int>(newLen)))
            throwError();

        if (outl != newLen)
            throw std::runtime_error("EVP_EncryptUpdate outl doesn't match inl. We don't use padding, so how is this possible?");

        QFile::writeData(this->writeCryptBufferOut, newLen);

#ifdef QT_DEBUG
        // To aid in debugging and looking at buffer content. Nothing else really.
        memset(this->writeCryptBufferIn, 0, static_cast<size_t>(this->writeCryptBufferSize));
        memset(this->writeCryptBufferOut, 0, static_cast<size_t>(this->writeCryptBufferSize));
#endif
    }

    return maxSize;
}

bool QFileEncrypted::atEnd() const
{
    if (decryptedSize > 0)
        return pos() >= decryptedSize;

    return QFile::atEnd();
}

void QFileEncrypted::close()
{
    if (this->openMode() & QFile::WriteOnly)
    {
        const char padSize = static_cast<char>(this->cryptBlockSize - getWriteBufUsedBytes());
        Q_ASSERT(padSize <= this->cryptBlockSize && padSize >= 0);
        const char padValue = padSize;
        QByteArray PKCSPadding(padSize, padValue);
        this->writeData(PKCSPadding, PKCSPadding.length());
    }
    Q_ASSERT(getWriteBufUsedBytes() == 0);
    flush();
    QFile::close();
    this->writeBufRPos = 0;
    this->writeBufWPos = 0;
    this->readBufRPos = 0;
    this->readBufWPos = 0;
    memset(this->writeCirBuf, 0, static_cast<size_t>(this->writeCryptBufferSize));
    memset(this->readCirBuf, 0, static_cast<size_t>(this->readCryptBufferSize));
}

qint64 QFileEncrypted::size() const
{
    if (this->decryptedSize > 0)
        return this->decryptedSize;

    return QFile::size();
}

QFileEncrypted::QFileEncrypted(const QString &name, const QByteArray &iv, const QByteArray &cipherKey, QObject *parent) :
    QFile(name, parent),
    cipherKey(cipherKey),
    iv(iv)
{
    writeCipherContext = EVP_CIPHER_CTX_new();
    readCipherContext = EVP_CIPHER_CTX_new();

    resizeWriteCryptBuffer(256);
    resizeReadCryptBuffer(256);

    const unsigned char *cipherKey_d = reinterpret_cast<const unsigned char *>(cipherKey.constData());
    const unsigned char *iv_d = reinterpret_cast<const unsigned char *>(iv.constData());

    const EVP_CIPHER *cipher = EVP_aes_256_cbc();

    if (!EVP_EncryptInit_ex(this->writeCipherContext, cipher, nullptr, cipherKey_d, iv_d))
        throwError();

    if (!EVP_CIPHER_CTX_set_padding(this->writeCipherContext, 0))
        throwError();

    this->cryptBlockSize = EVP_CIPHER_block_size(cipher);
    if (cryptBlockSize <= 0)
        throw std::runtime_error("Cipher block size <= 0?");

    if (!EVP_DecryptInit_ex(this->readCipherContext, EVP_aes_256_cbc(), nullptr, cipherKey_d, iv_d))
        throwError();

    if (!EVP_CIPHER_CTX_set_padding(this->readCipherContext, 0))
        throwError();

    // I want to have this info before we even open the file, but that does mean it will do this when
    // opening the file in write mode too. It's incorrect, but doesn't cause problems.
    QFileEncrypted *parentFile = dynamic_cast<QFileEncrypted*>(parent); // This is used to detect our boot-strapping situation.
    if (!parentFile && exists() && QFile::size() > 0)
        setPaddingSize();
}

QFileEncrypted::~QFileEncrypted()
{
    EVP_CIPHER_CTX_cleanup(writeCipherContext);
    EVP_CIPHER_CTX_free(writeCipherContext);
    EVP_CIPHER_CTX_cleanup(readCipherContext);
    EVP_CIPHER_CTX_free(readCipherContext);
    writeCipherContext = nullptr;
    readCipherContext = nullptr;

    delete[] this->writeCryptBufferIn;
    this->writeCryptBufferIn = nullptr;

    delete[] this->writeCryptBufferOut;
    this->writeCryptBufferOut = nullptr;

    delete[] this->writeCirBuf;
    this->writeCirBuf = nullptr;

    delete[] this->readCryptBufferIn;
    this->readCryptBufferIn = nullptr;

    delete[] this->readCryptBufferOut;
    this->readCryptBufferOut = nullptr;

    delete[] this->readCirBuf;
    this->readCirBuf = nullptr;
}

bool QFileEncrypted::open(QIODevice::OpenMode mode)
{
    if ( (mode & QIODevice::ReadOnly) && (mode & QIODevice::WriteOnly) )
    {
        throw std::runtime_error("Because of complexities with crypto padding, this class only supports WriteOnly or ReadOnly");
    }

    return QFile::open(mode);
}
