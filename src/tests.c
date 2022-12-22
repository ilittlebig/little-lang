#include <dirent.h>

#include "gen.h"
#include "file.h"
#include "typer.h"
#include "tests.h"
#include "parser.h"

static void run_test_for_file(char* path) {
	char expected_output_file_path[2048];
	strcpy(expected_output_file_path, path);
	expected_output_file_path[strlen(expected_output_file_path) - 4] = '\0';
	strcat(expected_output_file_path, ".txt");

	char* src = read_file(path);
	if (src) {
		obj_t* globals = parse(path, src);
		type_check_program(globals);

		codegen(globals);

		write_command("as --32 ./bin/assembly.asm ./lib/stdlib.asm -o ./bin/a.o");
		#ifdef _WIN32
			write_command("ld -m i386pe ./bin/a.o -o ./bin/a -lmsvcrt");
		#endif

		#ifdef linux
			write_command("ld -m elf_i386 ./bin/a.o -o ./bin/a -lc");
		#endif

		char* output = write_command("./bin/a");
		char* expected_output = read_file(expected_output_file_path);
		if (strcmp(output, expected_output) == 0) {
			printf("[\033[1;37m%s\033[0m]: \033[1;32mSucceeded\033[0m\n", path);
		} else {
			printf("[\033[1;37m%s\033[0m]: \033[1;31mFailed\033[0m\nExpected Output:\n%sActual Output:\n%s", path, expected_output, output);
		}
	}
}

void run_tests() {
	DIR *dir = opendir(TESTS_DIR);
	struct dirent *ent;

	while (ent = readdir(dir)) {
		if (ent->d_type == DT_DIR || ent->d_type == DT_UNKNOWN) {
			continue;
		}

		char* extension = strrchr(ent->d_name, '.');
		if (strcmp(extension, ".txt") == 0) {
			continue;
		}

		char* path = calloc(1,
			strlen(TESTS_DIR) +
			strlen(ent->d_name) + 1);
		path[0] = '\0';

		strcat(path, TESTS_DIR);
		strcat(path, ent->d_name);

		run_test_for_file(path);
	}

	closedir(dir);
}
