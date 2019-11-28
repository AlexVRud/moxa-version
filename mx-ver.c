
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <errno.h>

#define BUFFSIZE 256
#define CONFIG_FILE "/etc/moxa-configs/moxa-version.conf"

#ifdef __x86_64__
#define DMI_VAR_BOARD_NAME "board_name"
#else
#define UBOOT_VAR_MODELNAME "modelname"
#endif
#define CONFIG_VAR_FW_VERSION "FW_VERSION"
#define CONFIG_VAR_BUILDDATE "BUILDDATE"

static void usage()
{
	fprintf(stderr, "Usage: mx-ver [Option]\n");
	fprintf(stderr, "	-a: show all information\n");
	fprintf(stderr, "	-m: show the model name\n");
	fprintf(stderr, "	-v: show the firmware version\n");
	fprintf(stderr, "	-b: show the build time\n");
}

static int exec_shell(const char *cmd, char *output)
{
	FILE *fp;
	int state;

	fp = popen(cmd, "r");
	if (fp == NULL) {
		perror("popen");
		return -1;
	}

	if (fgets(output, BUFFSIZE, fp) == NULL) {
		perror("fgets");
		pclose(fp);
		return -1;
	}

	state = pclose(fp); 
	if (WIFEXITED(state)) {
		return  WEXITSTATUS(state);
	} else {
		fprintf(stderr, "Error: run command failed\n");
		return -1;
	}
}

#ifdef __x86_64__
static int get_dmi_env_var(const char *varname, char *output)
{
	char cmd[BUFFSIZE] = {0};

	sprintf(cmd, "cat /sys/class/dmi/id/%s", varname);
	if (exec_shell(cmd, output) < 0)
		return -1;

	output[strcspn(output, "\n")] = 0;
	return 0;
}
#else
static int get_uboot_env_var(const char *varname, char *output)
{
	char cmd[BUFFSIZE] = {0};

	sprintf(cmd, "fw_printenv -n %s", varname);
	if (exec_shell(cmd, output) < 0)
		return -1;

	output[strcspn(output, "\n")] = 0;
	return 0;
}
#endif

static int find_var_in_file(FILE *fp, const char *varname, char *output)
{
	char *line = NULL, *tok;
	size_t len = 0;
	int ret = -1;

	while (getline(&line, &len, fp) != -1) {
		if (!strncmp(line, varname, strlen(varname))) {
			tok = strtok(line, "=");
			tok = strtok(NULL, "\n");
			strcpy(output, tok);	
			ret = 0;
			break;
		}
	}

	if (line)
		free(line);
	return ret;
}

static int get_var_from_config(const char *varname, char *output)
{
	FILE *fp;

	fp = fopen(CONFIG_FILE, "r");
	if (fp == NULL) {
		perror("fopen");
		return -1;
	}

	if (find_var_in_file(fp, varname, output) < 0) {
		fclose(fp);
		fprintf(stderr, "Error: variable \"%s\" not found in config file\n", varname);
		return -1;
	}

	fclose(fp);
	return 0;
}

int main(int argc, char *argv[])
{
	char buff[BUFFSIZE], tmp[BUFFSIZE];

	if (argc != 1 && argc != 2) {
		usage();
		return 2;
	}

	if (argc == 1 || !strcmp(argv[1], "-a")) {
#ifdef __x86_64__
		if (get_dmi_env_var(DMI_VAR_BOARD_NAME, buff) < 0)
#else
		if (get_uboot_env_var(UBOOT_VAR_MODELNAME, buff) < 0)
#endif
			return 1;
		if (get_var_from_config(CONFIG_VAR_FW_VERSION, tmp) < 0) 
			return 1;
		strcat(buff, " version ");
		strcat(buff, tmp);
		if (get_var_from_config(CONFIG_VAR_BUILDDATE, tmp) < 0) 
			return 1;
		strcat(buff, " Build ");
		strcat(buff, tmp);
	} else if (!strcmp(argv[1], "-b")) {
		if (get_var_from_config(CONFIG_VAR_BUILDDATE, buff) < 0) 
			return 1;
	} else if (!strcmp(argv[1], "-m")) {
#ifdef __x86_64__
		if (get_dmi_env_var(DMI_VAR_BOARD_NAME, buff) < 0)
#else
		if (get_uboot_env_var(UBOOT_VAR_MODELNAME, buff) < 0)
#endif
			return 1;
	} else if (!strcmp(argv[1], "-v")) {
		if (get_var_from_config(CONFIG_VAR_FW_VERSION, buff) < 0) 
			return 1;
	} else {
		usage();
		return 2;
	}
	
	printf("%s\n", buff);
	return 0;
}
