#include "stdafx.h"
#include <functional>   // std::hash (replaces MSVC <xhash>/hash_value)

#include "Hasher.h"

Hasher::Hasher(wstring &salt)
{
	this->salt = salt;
}

wstring Hasher::getHash(wstring &name)
{
	// Removed try/catch
	//try {
		wstring s = wstring( salt ).append( name );
		//MessageDigest m;
		//m = MessageDigest.getInstance("MD5");
		//m.update(s.getBytes(), 0, s.length());
		//return new BigInteger(1, m.digest()).toString(16);

		// TODO Will this hash us with the same distribution as the MD5?
		return _toString( std::hash<std::wstring>()( s ) );
	//}
	//catch (NoSuchAlgorithmException e)
	//{
	//	throw new RuntimeException(e);
	//}
}