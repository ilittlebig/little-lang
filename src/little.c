#include "gen.h"
#include "file.h"
#include "error.h"
#include "typer.h"
#include "tests.h"
#include "parser.h"

static void compile_little(char* path) {
	char* src = read_file(path);
	if (src) {
		obj_t* globals = parse(path, src);
		type_check_program(globals);

		if (has_error) {
			printf("compilation failed\n");
			return;
		}
		codegen(globals);

		write_command("as --32 ./bin/assembly.asm ./lib/stdlib.asm -o ./bin/a.o");
		#ifdef _WIN32
			write_command("ld -m i386pe ./bin/a.o -o ./bin/a -lmsvcrt");
		#endif

		#ifdef linux
			write_command("ld -m elf_i386 ./bin/a.o -o ./bin/a -lc");
		#endif
	}
}

int main(int argc, char* argv[]) {
	char* path = argv[1];

	if (!path) {
		fatal_error("no input files");
		printf("compilation terminated\n");
		return 1;
	} else if (strcmp(path, "--test") == 0) {
		run_tests();
	} else {
		compile_little(path);
	}

	return 0;
}
