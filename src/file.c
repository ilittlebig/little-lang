/* Have to include <windows.h> here since other files
   are using "file.h", which makes some of the types
   conflict with eachother, and therefore it has to
   happen here */
#ifdef _WIN32
	#include <windows.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "file.h"

void write_file(const char* buff, const char* filename) {
	FILE* file;

	file = fopen(filename, "w");
	if (!file) {
		printf("Could not read file '%s'\n", filename);
		exit(1);
	}

	fprintf(file, "%s", buff);
	fclose(file);
}

char* read_file(const char* filename) {
	FILE* file;
	char* line = NULL;
	size_t len = 0;

	file = fopen(filename, "rb");
	if (!file) {
		printf("Could not read file '%s'\n", filename);
		exit(1);
	}

	char* buff = malloc(sizeof(char));
	buff[0] = '\0';

	while (getline(&line, &len, file) != -1) {
		buff = realloc(buff, strlen(buff) + strlen(line) + 1);
		strcat(buff, line);
	}

	fclose(file);
	if (line) {
		free(line);
	}

	return buff;
}

/* Executes the specified command. Returns program output if a
   program was run. */

char* write_command(const char* command) {
	#ifdef _WIN32
		char* output = calloc(1, sizeof(char));
		output[0] = '\0';

		HANDLE hReadPipe;
		HANDLE hWritePipe;
		SECURITY_ATTRIBUTES sa;
		sa.nLength = sizeof(SECURITY_ATTRIBUTES);
		sa.bInheritHandle = TRUE;
		sa.lpSecurityDescriptor = NULL;
		if (!CreatePipe(&hReadPipe, &hWritePipe, &sa, 0)) {
			printf("Failed to create pipe\n");
			exit(1);
		}

		STARTUPINFO si;
		PROCESS_INFORMATION pi;
		ZeroMemory(&si, sizeof(STARTUPINFO));
		si.cb = sizeof(STARTUPINFO);
		si.hStdError = hWritePipe;
		si.hStdOutput = hWritePipe;
		si.wShowWindow = SW_HIDE;
		si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
		if (!CreateProcess(NULL, (char*)command, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) {
			printf("Failed to run command '%s'\n", command);
			exit(1);
		}

		CloseHandle(hWritePipe);

		char buff[2048];
		DWORD nBytesRead;
		while (ReadFile(hReadPipe, buff, sizeof(buff) - 1, &nBytesRead, NULL) && nBytesRead > 0) {
			buff[nBytesRead] = '\0';
			output = realloc(output, strlen(output) + strlen(buff) + 1);
			strcat(output, buff);
		}

		CloseHandle(hReadPipe);
		WaitForSingleObject(pi.hProcess, INFINITE);
		CloseHandle(pi.hThread);
		CloseHandle(pi.hProcess);

		return output;
	#endif

	#ifdef linux
		FILE* file;
		file = popen(command, "r");

		if (!file) {
			printf("Failed to run command '%s'\n", command);
			exit(1);
		}
		fprintf(file, command);

		char* output = calloc(1, sizeof(char));
		output[0] = '\0';

		char buff[2048];
		while (fgets(buff, sizeof(buff), file)) {
			output = realloc(output, strlen(output) + strlen(buff) + 1);
			strcat(output, buff);
		}

		pclose(file);
		return output;
	#endif
}
