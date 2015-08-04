#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "ipc.h"
#include "log.h"
#include "worker.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

static struct worker worker;

static void output(const char* msg){
	fputs(msg, stdout);
}

static void read_ipc(const char* filename){
	worker.pipe[READ_FD] = open(filename, 0);

	char* payload = NULL;
	size_t size = 0;
	ipc_fetch(&worker, (void*)&payload, &size);
	free(payload);
}

static void write_ipc(){
	worker.pipe[WRITE_FD] = STDOUT_FILENO;

	const char* payload = "0123456789";
	ipc_push(&worker, IPC_SHUTDOWN, payload, strlen(payload));
}

int main(int argc, const char* argv[]){
	if ( argc != 2 ){
		fprintf(stderr,
		        "usage: %s FILENAME\n"
		        "       %s - > FILENAME\n"
		        "\n"
		        "First form read FILENAME as input and tries to read it as an IPC command.\n"
		        "Second form generates a sample IPC command (machine specific) and writes on stdout.\n"
		        , argv[0], argv[0]);
		return 1;
	}

	log_output(output);

	if ( argv[1][0] != '-' ){
		read_ipc(argv[1]);
	} else {
		write_ipc();
	}

	return 0;
}
