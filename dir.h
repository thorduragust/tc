#pragma once
#include <dirent.h>//notum dirent windows portið
#include <sys/stat.h>

#include "array.h"
#include "string.h"

namespace tc {
	struct file_listing {
		string<256> name;
		bool is_dir;
	};

	array<file_listing> listdir(const char *dirname) {
		array<file_listing> result = {};

		DIR *dp;
		dirent *entry;
		dp = opendir(dirname);

		if(dp) {
			while((entry = readdir(dp))) {
				const char *fn = entry->d_name;
				if(strcmp(fn, ".") == 0) continue;
				if(strcmp(fn, "..") == 0) continue;

				string<1024> path;
				struct stat file_info;
				snprintf(path.data, path.capacity, "%s/%s", dirname, fn);
				stat(path.data, &file_info);

				file_listing fl;

				fl.name.copy(fn);
				fl.is_dir = S_ISDIR(file_info.st_mode);
				
				result.push(fl);
			}
		}

		closedir(dp);

		return result;
	}
};
