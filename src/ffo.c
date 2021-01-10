#include "ffo.h"

#define CMD_BUF_LEN (256)
#define MAX_PATH_LEN (64)
static char *greetings = "Welcome to use flexible file operator.\n";

enum {
	INP_END = 0,
	INP_NONE,
	INP_MANUAL,
	INP_SET,
	INP_EXECUTE,
	INP_RUNSCRIPT,
	INP_PRINT,
	INP_ALLINONE,
};

enum operation {
	OP_NONE = 0,
	OP_OPEN,
	OP_READ,
	OP_PREAD,
	OP_WRITE,
	OP_PWRITE,
	OP_CLOSE,
	OP_NUM,
};

static char *opname[] = {	"none",
							"open",
							"read",
							"write",
							"close",
							};

//TODO: Use union to save space.
struct operation_info {
	enum operation op;
	int fd;
/* For open */
	char filename[MAX_PATH_LEN];
	int open_flags;

/* For read/write */
	size_t rwcount;
	off_t offset;
};

int is_letter(char ch)
{
	if (ch >= 'a' && ch <= 'z')
		return 1;
	if (ch >= 'A' && ch <= 'Z')
		return 1;
	return 0;
}

void parse_open_flags(struct operation_info *opi, char *value_base)
{
	char *p = value_base;
	char *q;
	int not_finished = 1;

	if ('+' == *value_base) {
		p = value_base + 1;
	} else {
		opi->open_flags = O_RDWR;
	}

	while (not_finished) {
		if (!is_letter(*p))
			break;
		q = p + 1;
		while ((*q != '|') && (*q != '\0'))
			q++;
		if (*q == '\0')
			not_finished = 0;
		else
			*q = '\0';

		if (!strcmp(p, "creat"))
			opi->open_flags |= O_CREAT;
		else if (!strcmp(p, "trunc"))
			opi->open_flags |= O_TRUNC;

		p = q + 1;
	}
}


/*
 * ubuf must end with '\0'.
 * ubuf="aaa=bbb\0"
 */
static void set_property_one(struct operation_info *opi, char *ubuf)
{
	int buflen;
	char *arg_base, *value_base;
	int arg_start;
	int arg_end;
	int value_start;

	buflen = strlen(ubuf);
	if (buflen<1)
		return;
	for (arg_start=0;arg_start<buflen;arg_start++) {
		if (is_letter(ubuf[arg_start]))
			goto ARG_START_FOUND;
	}
	return;
ARG_START_FOUND:
	arg_base = ubuf + arg_start;
	for (arg_end = arg_start + 1;arg_end<buflen;arg_end++) {
		if (ubuf[arg_end]=='=' || ubuf[arg_end]==' ')
			goto ARG_END_FOUND;
	}
	return;
ARG_END_FOUND:
	ubuf[arg_end] = '\0';
	for (value_start=arg_end+1;value_start<buflen;value_start++) {
		if (ubuf[value_start]=='=' || ubuf[value_start]==' ')
			continue;
		if (ubuf[value_start]!='\0')
			goto VALUE_START_FOUND;
	}
	return;
VALUE_START_FOUND:
	value_base = ubuf + value_start;

	//Start to parse
	if (!strcmp(arg_base, "op")) {
		if (!strcmp(value_base, "open"))
			opi->op = OP_OPEN;
		else if (!strcmp(value_base, "read"))
			opi->op = OP_READ;
		else if (!strcmp(value_base, "pread"))
			opi->op = OP_PREAD;
		else if (!strcmp(value_base, "write"))
			opi->op = OP_WRITE;
		else if (!strcmp(value_base, "pwrite"))
			opi->op = OP_PWRITE;
		else if (!strcmp(value_base, "close"))
			opi->op = OP_CLOSE;
		return;
	}
	if (!strcmp(arg_base, "fd")) {
		opi->fd = atoi(value_base);
		return;
	}
	if (!strcmp(arg_base, "openflags") || !strcmp(arg_base, "of")) {
		parse_open_flags(opi, value_base);
		return;
	}
	if (!strcmp(arg_base, "filename") || !strcmp(arg_base, "fn")) {
		snprintf(opi->filename, MAX_PATH_LEN, value_base);
		return;
	}
	if (!strcmp(arg_base, "rwcount")) {
		opi->rwcount = atoi(value_base);
		return;
	}
	if (!strcmp(arg_base, "offset")) {
		opi->offset = atol(value_base);
		return;
	}

}

/*
 * ubuf="aaa=bbb;ccc=ddd;eee=fff\0"
 */
static void set_properties(struct operation_info *opi, char *ubuf)
{
	int buflen;
	char *start;
	char *p;

	buflen = strlen(ubuf);
	if (buflen<2)
		return;

	start = ubuf;

GO_ON:
	if ((!*start) || (*start == ';'))
		return;
	p = start;
	while ((*p != ';') && (*p != '\0'))
		p++;
	if (';' == *p) {
		*p = '\0';
		set_property_one(opi, start);
		start = p + 1;
		goto GO_ON;
	} else {
		//Last one
		set_property_one(opi, start);
	}
	
}

static ssize_t do_read(struct operation_info *opi)
{
	ssize_t ret;
	void *buf;

	if (opi->fd == 0 || opi->fd == 0) {
		printf("Oops..fd=%d\n", opi->fd);
		return 0;
	}

	buf = malloc(opi->rwcount);
	if (!buf) {
		printf("malloc failed\n");
		return -1;
	}
	ret = read(opi->fd, buf, opi->rwcount);
	free(buf);
	return ret;
}

static ssize_t do_pread(struct operation_info *opi)
{
	ssize_t ret;
	void *buf;

	if (opi->fd == 0 || opi->fd == 0) {
		printf("Oops..fd=%d\n", opi->fd);
		return 0;
	}

	buf = malloc(opi->rwcount);
	if (!buf) {
		printf("malloc failed\n");
		return -1;
	}
	ret = pread(opi->fd, buf, opi->rwcount, opi->offset);
	free(buf);
	return ret;
}


static ssize_t do_write(struct operation_info *opi)
{
	ssize_t ret;
	void *buf;

	if (opi->fd == 0 || opi->fd == 0) {
		printf("Oops..fd=%d\n", opi->fd);
		return 0;
	}

	buf = malloc(opi->rwcount);
	if (!buf) {
		printf("malloc failed\n");
		return -1;
	}
	ret = write(opi->fd, buf, opi->rwcount);
	free(buf);
	return ret;
}

static ssize_t do_pwrite(struct operation_info *opi)
{
	ssize_t ret;
	void *buf;

	if (opi->fd == 0 || opi->fd == 0) {
		printf("Oops..fd=%d\n", opi->fd);
		return 0;
	}

	buf = malloc(opi->rwcount);
	if (!buf) {
		printf("malloc failed\n");
		return -1;
	}
	ret = pwrite(opi->fd, buf, opi->rwcount, opi->offset);
	free(buf);
	return ret;
}

static void execute_operation(struct operation_info *opi)
{
	int ret;
	ssize_t ret2;

	switch (opi->op)
	{
	case OP_OPEN:
		ret = open(opi->filename, opi->open_flags, 0644);
		printf("Open() return with %d\n", ret);
		break;
	case OP_CLOSE:
		ret = close(opi->fd);
		printf("Close(fd=%d) return with %d\n", opi->fd, ret);
		break;
	case OP_READ:
		ret2 = do_read(opi);
		printf("read(fd=%d) return with %d\n", opi->fd, ret2);
		break;
	case OP_WRITE:
		ret2 = do_write(opi);
		printf("write(fd=%d) return with %d\n", opi->fd, ret2);
		break;
	case OP_PWRITE:
		ret2 = do_pwrite(opi);
		printf("pwrite(fd=%d) return with %d\n", opi->fd, ret2);
		break;
	case OP_PREAD:
		ret2 = do_pread(opi);
		printf("pread(fd=%d) return with %d\n", opi->fd, ret2);
		break;
	default:
		printf("Execute nothing\n");
		return;
	}
}


static void print_manual(void)
{
	printf("Command action:\n");
	printf("   s @arg1=@value1;@arg2=@value2;...\n");
	printf("   \t\t\tSet @arg as @value\n");
	printf("\n");
	printf("   e\t\t\tExecute\n");
	printf("   r @scriptfile\tRun a script\n");
	printf("   p\t\t\tPrint the current operation properties\n");
	printf("   a @cmd\n");
	printf("   \t\t\tOne-line execution\n");
	printf("   \t\t\tEquals to \'s @cmd\' then \'e\' \n");
	printf("\n");
	printf("   m\t\t\tPrint this manual\n");
	printf("   q\t\t\tQuit\n");
	printf("\n");

	printf("Arguments and values for s:\n");
	printf("\top = open/read/pread/write/pwrite/close\n");
	printf("\tfd = int\n");
	printf("\tfn/filename = string\n");
	printf("\tof/openflags = [+]creat|trunc\n");
	printf("\t\tNote: no space nearby \'|\'\n");
	printf("\trwcount = int\n");
	printf("\toffset = long\n");
}

static void reset_command_info(struct operation_info *opinfo)
{
	memset(opinfo, 0, sizeof(struct operation_info));
	opinfo->op = OP_NONE;
	opinfo->open_flags = O_RDWR;
}

static int get_usr_command(char *usr_cmd_buf)
{
	int len = 0;
	char ch;
	int overflow=1;
	int ret=INP_NONE;

	for (;len<CMD_BUF_LEN;len++) {
		ch = usr_cmd_buf[len] = getchar();
		if (ch == '\n') {
			overflow = 0;
			usr_cmd_buf[len] = '\0';
			break;
		}
		if (!len) {
			switch (ch)
			{
			case 'a':
				ret = INP_ALLINONE;
				break;
			case 's':
				ret = INP_SET;
				break;
			case 'e':
				ret = INP_EXECUTE;
				break;
			case 'r':
				ret = INP_RUNSCRIPT;
				break;
			case 'p':
				ret = INP_PRINT;
				break;
			case 'm':
				ret = INP_MANUAL;
				break;
			case 'q':
				ret = INP_END;
				break;
			default:
				ret = INP_NONE;
			}
		}
	}
	if (overflow)
		return INP_NONE;
	else
		return ret;
}

static void clean_up(void)
{

}

static void print_properties(struct operation_info *opinfo)
{
	printf("Operation properties:\n");
	printf("\top=%s\n", opname[opinfo->op]);
	printf("\tfd=%d\n", opinfo->fd);
	printf("\tfilename=%s\n", opinfo->filename);
	printf("\topenflags=0x%x\n", opinfo->open_flags);
	printf("\trwcount=%d\n", opinfo->rwcount);
}


void exec_allinone(char *ubuf)
{
	struct operation_info opinfo;

	reset_command_info(&opinfo);

	set_properties(&opinfo, ubuf);
	execute_operation(&opinfo);
}


int main(int argc, char *argv[])
{
	int inp;
	char usr_cmd_buf[CMD_BUF_LEN];
	struct operation_info opinfo;

	reset_command_info(&opinfo);
	printf(greetings);
	for (;;) {
		printf("-------------------------------------------\n");
		printf("Command (m for help): ");
		inp = get_usr_command(usr_cmd_buf);
		switch (inp) {
		case INP_PRINT:
			print_properties(&opinfo);
			break;
		case INP_SET:
			set_properties(&opinfo, &usr_cmd_buf[1]);
			print_properties(&opinfo);
			break;
		case INP_EXECUTE:
			execute_operation(&opinfo);
			reset_command_info(&opinfo);
			break;
		case INP_RUNSCRIPT:
			printf("Currently not supported\n");
			break;
		case INP_NONE:
			printf("Operation not supported\n");
			break;
		case INP_MANUAL:
			print_manual();
			break;
		case INP_ALLINONE:
			exec_allinone(&usr_cmd_buf[1]);
			break;
		case INP_END:
			goto OUT_CLEANUP;
		}
	}
OUT_CLEANUP:
	printf("Bye\n");
	clean_up();
	return 0;
}
