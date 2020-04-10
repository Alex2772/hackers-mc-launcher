#include "User.h"
#include "Util/Crypto.h"

QString User::id() const
{
	return Crypto::md5(mUsername).mid(0, 32);
}
