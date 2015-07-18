#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <magic.h>

static magic_t magic;

static const char* strip_prefix(const char* filename){
	if ( strncmp(filename, "static/", 7) == 0 ){
		return filename+7;
	} else {
		return filename;
	}
}

static const char* mangle_filename(const char* filename){
	static char buf[4096];
	const size_t len = strlen(filename);
	for ( unsigned int i = 0; i < len; i++ ){
		if ( isalnum(filename[i]) ){
			buf[i] = filename[i];
		} else {
			buf[i] = '_';
		}
	}
	buf[len] = 0;
	return buf;
}



static int need_escape(char ch){
	return ch == '"' || ch == '\n' || ch == '\r' || ch == '\t' || ch == '\\';
}

static int escaped(char ch){
	if ( ch == '\n' ) return 'n';
	if ( ch == '\r' ) return 'r';
	if ( ch == '\t' ) return 't';
	return ch;
}

int main(int argc, const char* argv[]){
	printf("#include \"static.h\"\n");
	magic = magic_open(MAGIC_MIME_TYPE);
	magic_load(magic, NULL);

	/* file data */
	for ( int i = 2; i < argc; i++ ){
		const char* filename = strip_prefix(argv[i]);
		const char* varname = mangle_filename(filename);
		printf("static const char tweak_static_%s[] = \"", varname);

		FILE* fp = fopen(argv[i], "r");
		if ( fp ){
			char buf[4096];
			int bytes;
			while ( (bytes=fread(buf, 1, sizeof(buf), fp)) > 0 ){
				for ( int j = 0; j < bytes; j++ ){
					if ( !need_escape(buf[j]) ){
						putchar(buf[j]);
					} else {
						putchar('\\');
						putchar(escaped(buf[j]));
					}
				}
			}
		}

		printf("\";\n");
	}

	/* file table */
	printf("struct file_entry file_table[] = {\n");
	printf("\t{\"/\", \"text/html\", tweak_static_index_html, sizeof(tweak_static_index_html)},\n"); /* hack for directory index alias */
	for ( int i = 2; i < argc; i++ ){
		const char* filename = strip_prefix(argv[i]);
		const char* varname = mangle_filename(filename);
		const char* mime = magic_file(magic, argv[i]);
		printf("\t{\"/%s\", \"%s\", tweak_static_%s, sizeof(tweak_static_%s)},\n", filename, mime, varname, varname);
	}
	printf("\t{NULL, NULL, NULL, 0},\n"); /* sentinel */
	printf("};\n");

	magic_close(magic);
}
