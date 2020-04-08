#include "Crypto.h"
#include <qcryptographichash.h>
#include <QFileDialog>

QString Crypto::sha1(QFile& file)
{
	QCryptographicHash hash(QCryptographicHash::Sha1);
	file.open(QFile::ReadOnly);
	hash.addData(&file);
	file.close();

	return hash.result().toHex();
}
