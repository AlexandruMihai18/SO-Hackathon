#include <dlfcn.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "ipc.h"
#include "server.h"

#ifndef OUTPUTFILE_TEMPLATE
#define OUTPUTFILE_TEMPLATE "../checker/output/out-XXXXXX"
#endif

#ifndef MAX_SIZE
#define MAX_SIZE 256
#endif

#ifndef RUN_FUNC
#define RUN_FUNC "run"
#endif

int output_fd;
char name[MAX_SIZE];
char func[MAX_SIZE];
char params[MAX_SIZE];
char message[MAX_SIZE];

void dealloc(struct lib *lib)
{
	free(lib->libname);
	free(lib->funcname);
	free(lib->filename);
	free(lib->outputfile);
}

static int lib_prehooks(struct lib *lib)
{
	lib->outputfile = strdup(OUTPUTFILE_TEMPLATE);
	if (!lib->outputfile) {
		perror("strdup failed");
		return -1;
	}

	output_fd = mkstemp(lib->outputfile);
	dup2(output_fd, 1);

	lib->libname = malloc(MAX_SIZE);
	if (!lib->libname) {
		perror("malloc failed");
		return -1;
	}

	strcpy(lib->libname, "../checker/");
	strcat(lib->libname, name);

	if (*func) {
		lib->funcname = strdup(func);
	}
	else {
		lib->funcname = strdup(RUN_FUNC);
	}
	
	if (!lib->funcname) {
		perror("strdup failed");
		return -1;
	}

	lib->filename = NULL;

	if (*params) {
		lib->filename = strdup(params);
		if (!lib->filename) {
			perror("strdup failed");
			return -1;
		}
	}

	return 0;
}

static int lib_load(struct lib *lib)
{
	lib->handle = dlopen(lib->libname, RTLD_LAZY);

	if (!lib->handle) {
		perror("dlopen failed");
		dealloc(lib);
		return -1;
	}

	return 0;
}

static int lib_execute(struct lib *lib)
{
	if (!lib->filename) {
		lib->run = (lambda_func_t) dlsym(lib->handle, lib->funcname);
		if (!lib->run) {
			perror("dlsym failed");
			dealloc(lib);
			return -1;
		}
		lib->run();
	}
	else {
		lib->p_run = (lambda_param_func_t) dlsym(lib->handle, lib->funcname);
		if (!lib->p_run) {
			perror("dlsym failed");
			dealloc(lib);
			return -1;
		}
		lib->p_run(lib->filename);
	}

	strcpy(message, lib->outputfile);

	return 0;
}

static int lib_close(struct lib *lib)
{
	int ret = dlclose(lib->handle);

	if (ret) {
		return -1;
	}

	return 0;
}

static int lib_posthooks(struct lib *lib)
{
	dealloc(lib);
	return 0;
}

static int lib_run(struct lib *lib)
{
	(void)lib;
	int err;

	err = lib_prehooks(lib);
	if (err)
		return err;

	err = lib_load(lib);
	if (err)
		return err;

	err = lib_execute(lib);
	if (err)
		return err;

	err = lib_close(lib);
	if (err)
		return err;

	return lib_posthooks(lib);
}

static int parse_command(const char *buf, char *name, char *func, char *params)
{
	return sscanf(buf, "%s %s %s", name, func, params);
}

int main(void)
{
	int ret;
	struct lib lib;

	int socket_client;

	/* TODO - Implement server connection */
	int socket_fd = create_server();
	if (socket_fd == -1) {
		perror("socket");
		exit(1);
	}

	char buffer[MAX_SIZE];

	int argv;
	(void)argv;
	// listen for messages from client and create fork upon receiving request
	while (1) {
		socket_client = accept_socket(socket_fd);
		printf("Connection request received...\n");
		int pid = fork();
		if (pid > 0) {
			printf("Created a fork\n");
		} else {
			printf("Message from fork\n");
		}
		if (pid == 0) {
			break;
		}
	}

	ret = recv_socket(socket_client, buffer, MAX_SIZE);
	if (ret == -1) {
		perror("recv");
		exit(1);
	}

	printf("Received message: %s\n", buffer);

	/* TODO - parse message with parse_command and populate lib */
	argv = parse_command(buffer, name, func, params);

	/* TODO - handle request from client */
	ret = lib_run(&lib);

	if (ret == -1) {
		strcpy(message, "Error: ");
		strcat(message, buffer);
		strcat(message, " could not be executed");
	}

	ret = send_socket(socket_client, message, MAX_SIZE);
	if (ret == -1) {
		perror("send");
		exit(1);
	}

	return 0;
}