#include "fileOperations.h"
#include <nds.h>
#include <slim.h>
#include <stdio.h>
#include <dirent.h>
#include <vector>

using namespace std;

off_t getFileSize(const char *fileName)
{
	FILE* fp = fopen(fileName, "rb");
	off_t fsize = 0;
	if (fp) {
		fseek(fp, 0, SEEK_END);
		fsize = ftell(fp);			// Get source file's size
		fseek(fp, 0, SEEK_SET);
		fclose(fp);
	}

	return fsize;
}
