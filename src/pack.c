#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <string.h>
#include <ctype.h>

static const char* strip_prefix(const char* filename){
	if ( strncmp(filename, srcdir, strlen(srcdir)) == 0 ){
		filename += strlen(srcdir);
	}

	static const char* prefix = "static/";
	if ( strncmp(filename, prefix, strlen(prefix)) == 0 ){
		filename += strlen(prefix);
	}

	return filename;
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

static const char* mimetype(const char* filename){
	const size_t len = strlen(filename);

	if ( strcmp(filename + (len-5), ".html") == 0 ) return "text/html; charset=utf-8";
	if ( strcmp(filename + (len-4), ".css") == 0 ) return "text/css";
	if ( strcmp(filename + (len-3), ".js") == 0 ) return "application/javascript";

	return "text/plain";
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

static void write_entry(const char* src, const char* dst){
	const char* filename = strip_prefix(src);
	const char* varname = mangle_filename(strip_prefix(dst));
	const char* mime = mimetype(dst);
	printf("\t{\"/%s\", \"%s\", \"%s\", tweak_static_%s, sizeof(tweak_static_%s)-1},\n", filename, dst, mime, varname, varname);
}

int main(int argc, const char* argv[]){
	printf("#include \"static.h\"\n");

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
	write_entry("", srcdir "static/index.html");
	for ( int i = 2; i < argc; i++ ){
		write_entry(argv[i], argv[i]);
	}
	printf("\t{NULL, NULL, NULL, 0},\n"); /* sentinel */
	printf("};\n");
}
