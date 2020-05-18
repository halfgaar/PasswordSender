#include <iostream>

#include <QtTest>
#include <qfileencrypted.h>
#include <QtGlobal>
#include "mynamedtempfile.h"


// add necessary includes here

class PasswordSenderTests : public QObject
{
    Q_OBJECT
    QByteArray iv;
    QByteArray cipherKey;

    void testQFileEncryptedWriteOnceReadOnceBigChunksBase(const int size);
    void testQFileEncryptedXBytes(int nbytes);

public:
    PasswordSenderTests();
    ~PasswordSenderTests();

private slots:
    void testQFileEncryptedWriteNonAlignedSizeInThreeTimes();
    void testQFileEncryptedWriteNonAlignedSizeInThreeTimesReadPastEnd();
    void testQFileEncryptedWriteOnceReadOnceBigChunks();
    void testQFileEncryptedWriteChunkThenBiggerChunk();
    void testQFileEncrypted137447Bytes();
    void testQFileEncrypted16369594Bytes();
};

PasswordSenderTests::PasswordSenderTests() :
    iv(QByteArray(32,0)),
    cipherKey(QByteArray(32,0))
{
    quint8 i = 2;
    for (char &b : iv)
    {
        b = i++;
    }

    i = 60;
    for (char &b : cipherKey)
    {
        b = i++;
    }
}

PasswordSenderTests::~PasswordSenderTests()
{

}

void PasswordSenderTests::testQFileEncryptedWriteNonAlignedSizeInThreeTimes()
{
    MyNamedTempFile tempFile;

    QFileEncrypted out(tempFile.path, iv, cipherKey);
    out.open(QFile::WriteOnly);
    QByteArray in1(11, 1);
    QByteArray in2(11, 2);
    QByteArray in3(11, 3);
    QByteArray inTotal(33, 0);
    inTotal.replace(0, in1.length(), in1);
    inTotal.replace(11, in1.length(), in2);
    inTotal.replace(22, in1.length(), in3);

    QVERIFY(inTotal.length() == (in1.length() + in2.length() + in3.length()));

    out.write(in1.data(), in1.length());
    out.write(in2.data(), in2.length());
    out.write(in3.data(), in3.length());
    out.close();

    QByteArray bytesRead(33, 0);

    QFileEncrypted in(tempFile.path, iv, cipherKey);
    in.open(QFile::ReadOnly);
    qint64 n = in.read(bytesRead.data(), 11);
    n = in.read(bytesRead.data() + 11, 11);
    n = in.read(bytesRead.data() + 22, 11);
    in.close();

    QVERIFY(bytesRead == inTotal);

    QFileEncrypted inFile(tempFile.path, iv, cipherKey);
    inFile.open(QFile::ReadOnly);
    QByteArray bytesRead2 = inFile.readAll();

    QVERIFY(bytesRead2 == inTotal);

}



void PasswordSenderTests::testQFileEncryptedWriteNonAlignedSizeInThreeTimesReadPastEnd()
{
    MyNamedTempFile tempFile;

    QFileEncrypted out(tempFile.path, iv, cipherKey);
    out.open(QFile::WriteOnly);
    QByteArray in1(11, 1);
    QByteArray in2(11, 2);
    QByteArray in3(11, 3);
    QByteArray inTotal(33, 0);
    inTotal.replace(0, in1.length(), in1);
    inTotal.replace(11, in1.length(), in2);
    inTotal.replace(22, in1.length(), in3);

    QVERIFY(inTotal.length() == (in1.length() + in2.length() + in3.length()));

    out.write(in1.data(), in1.length());
    out.write(in2.data(), in2.length());
    out.write(in3.data(), in3.length());
    out.close();

    QFileEncrypted inFile(tempFile.path, iv, cipherKey);
    inFile.open(QFile::ReadOnly);
    QByteArray bytesRead2 = inFile.read(1024*1024);
    inFile.close();

    QVERIFY(bytesRead2 == inTotal);
}

void PasswordSenderTests::testQFileEncryptedWriteOnceReadOnceBigChunksBase(const int size)
{
    MyNamedTempFile tempFile;

    QByteArray big(size, 0);
    quint8 i= 0;
    for (auto &b : big)
    {
        b = i++;
    }

    QFileEncrypted outFile(tempFile.path, iv, cipherKey);
    outFile.open(QFile::WriteOnly);
    outFile.write(big);
    outFile.close();

    QFileEncrypted inFile(tempFile.path, iv, cipherKey);
    QVERIFY2(inFile.size() == size, "File size is not the same upon reading. Padding error.");
    inFile.open(QFile::ReadOnly);
    QByteArray resultRead = inFile.readAll();
    inFile.close();

    QVERIFY(resultRead == big);
}

void PasswordSenderTests::testQFileEncryptedWriteOnceReadOnceBigChunks()
{
    testQFileEncryptedWriteOnceReadOnceBigChunksBase(1024*1024);
    testQFileEncryptedWriteOnceReadOnceBigChunksBase(1024*1024 + 1);
    testQFileEncryptedWriteOnceReadOnceBigChunksBase(1024*1024 - 1);
    testQFileEncryptedWriteOnceReadOnceBigChunksBase(1024*1024 - 13);
    testQFileEncryptedWriteOnceReadOnceBigChunksBase(1024*1024 + 13);
}

/**
 * @brief PasswordSenderTests::testQFileEncryptedWriteChunkThenBiggerChunk test the circular buffer resizing.
 */
void PasswordSenderTests::testQFileEncryptedWriteChunkThenBiggerChunk()
{
    MyNamedTempFile tempFile;

    QByteArray big(135, 0);
    quint8 i= 10;
    for (auto &b : big)
    {
        b = i++;
    }

    QByteArray big2(64*1023, 0);
    i= 129;
    for (auto &b : big2)
    {
        b = i++;
    }

    QByteArray inTotal = big + big2;

    QFileEncrypted outFile(tempFile.path, iv, cipherKey);
    outFile.open(QFile::WriteOnly);
    outFile.write(big);
    outFile.write(big2);
    outFile.close();

    QFileEncrypted inFile(tempFile.path, iv, cipherKey);
    inFile.open(QFile::ReadOnly);
    QByteArray resultRead = inFile.readAll();
    inFile.close();

    QVERIFY(resultRead == inTotal);
}

void PasswordSenderTests::testQFileEncryptedXBytes(int nbytes)
{
    MyNamedTempFile tempFile;

    QByteArray sourceData(nbytes, 0);
    quint8 i= 10;
    for (auto &b : sourceData)
    {
        b = i++;
    }

    QFileEncrypted outFile(tempFile.path, iv, cipherKey);
    outFile.open(QFile::WriteOnly);

    int size= sourceData.length();
    while (size > 0)
    {
        qint64 len = qMin(65536, size);
        outFile.write(sourceData.data(), len);
        size -= len;
    }

    outFile.close();

    QFileEncrypted inFile(tempFile.path, iv, cipherKey);
    QVERIFY2(inFile.size() == nbytes, "File size is not the same upon reading. Padding error.");
    QByteArray dataRead;
    inFile.open(QFile::ReadOnly);
    while (!inFile.atEnd())
    {
        dataRead.append(inFile.read(65536));
    }

    QVERIFY2(sourceData.length() == dataRead.length(), "Length of source data and data read don't match.");
    QVERIFY2(sourceData == dataRead, "Data doesn't match.");
}

/**
 * @brief PasswordSenderTests::testQFileEncrypted137447Bytes
 *
 * Based on a case found during normal testing.
 */
void PasswordSenderTests::testQFileEncrypted137447Bytes()
{
    testQFileEncryptedXBytes(137447);
}

/**
 * @brief PasswordSenderTests::testQFileEncrypted16369594Bytes
 *
 * Based on a case found during normal testing.
 */
void PasswordSenderTests::testQFileEncrypted16369594Bytes()
{
    testQFileEncryptedXBytes(16369594);
}

QTEST_APPLESS_MAIN(PasswordSenderTests)

#include "tst_passwordsendertestcase.moc"
