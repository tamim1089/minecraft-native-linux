#pragma once

class File;

// java library interface.
class FileFilter
{
public:
	virtual bool accept(File *dir) = 0;
};