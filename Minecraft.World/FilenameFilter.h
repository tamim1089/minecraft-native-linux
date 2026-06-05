#pragma once
using namespace std;

#include "stdafx.h"

class File;

// java lirary interface.
class FilenameFilter
{
public:
	virtual bool accept(File *dir, const wstring& name) = 0;
};