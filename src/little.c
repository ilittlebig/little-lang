#include "file.h"
#include "error.h"
#include "parser.h"

static void compile_little(char* path) {
	char* src = read_file(path);
	if (src) {
		parse_src(path, src);
		free(src);
	}

	write_command("as --32 ./bin/assembly.asm ./lib/stdlib.asm -o ./bin/a.o");
	write_command("ld -m elf_i386 ./bin/a.o -o ./bin/a");
}

int main(int argc, char* argv[]) {
	char* path = argv[1];
	if (!path) {
		fatal_error("no input files");
		printf("compilation terminated\n");
		return 1;
	}

	compile_little(path);
	return 0;
}
