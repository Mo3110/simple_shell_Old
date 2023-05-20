#include "shell.h"

/**
 * inputBuf - buffers chained commands
 * @info: parameter struct
 * @buf: address of buffer
 * @len: address of len var
 *
 * Return: bytes read
 */
ssize_t inputBuf(info_t *info, char **buf, size_t *len)
{
	ssize_t n = 0;
	size_t len_p = 0;

	if (!*len)
	{
		free(*buf);
		*buf = NULL;
		signal(SIGINT, sigintHandler);
#if USE_GETLINE
		n = getline(buf, &len_p, stdin);
#else
		n = _getline(info, buf, &len_p);
#endif
		if (n > 0)
		{
			if ((*buf)[n - 1] == '\n')
			{
				(*buf)[n - 1] = '\0';
				n--;
			}
			info->linecount_flag = 1;
			removeComments(*buf);
			buildHistoryList(info, *buf, info->histcount++);
			{
				*len = n;
				info->cmd_buf = buf;
			}
		}
	}
	return (n);
}

/**
 * getInput - gets a line minus the newline
 * @info: parameter struct
 *
 * Return: bytes read
 */
ssize_t getInput(info_t *info)
{
	static char *buf;
	static size_t i, j, len;
	ssize_t n = 0;
	char **buf_p = &(info->arg), *p;

	_putchar(BUF_FLUSH);
	n = inputBuf(info, &buf, &len);
	if (n == -1)
		return (-1);

	if (len)
	{
		j = i;
		p = buf + i;

		checkChain(info, buf, &j, i, len);

		while (j < len)
		{
			if (isChain(info, buf, &j))
				break;
			j++;
		}

		i = j + 1;
		if (i >= len)
		{
			i = len = 0;
			info->cmd_buf_type = CMD_NORM;
		}
		*buf_p = p;
		return (_strlen(p));
	}
	*buf_p = buf;
	return (n);
}

/**
 * readBuf - reads a buffer
 * @info: parameter struct
 * @buf: buffer
 * @i: size
 *
 * Return: n
 */
ssize_t readBuf(info_t *info, char *buf, size_t *i)
{
	ssize_t n = 0;

	if (*i)
		return (0);
	n = read(info->readfd, buf, READ_BUF_SIZE);
	if (n >= 0)
		*i = n;
	return (n);
}

/**
 * _getline - gets the next line of input from STDIN
 * @info: parameter struct
 * @ptr: address of pointer to buffer, preallocated or NULL
 * @length: size of preallocated ptr buffer if not NULL
 *
 * Return: s
 */
int _getline(info_t *info, char **ptr, size_t *length)
{
	static char buf[READ_BUF_SIZE];
	static size_t i, len;
	size_t k;
	ssize_t n = 0, s = 0;
	char *p = NULL, *new_p = NULL, *c;

	p = *ptr;
	if (p && length)
		s = *length;
	if (i == len)
		i = len = 0;

	n = readBuf(info, buf, &len);
	if (n == -1 || (n == 0 && len == 0))
		return (-1);

	c = strChr(buf + i, '\n');
	k = c ? 1 + (unsigned int)(c - buf) : len;
	new_p = strnCat(_realloc(p, s, s ? s + k : k + 1), buf + i, k - i);
	if (!new_p)
		return (p ? (free(p), -1) : -1);
	s += k - i;
	i = k;
	p = new_p;

	if (length)
		*length = s;
	*ptr = p;
	return (s);
}

/**
 * sigintHandler - blocks ctrl-C
 * @sig_num: the signal number
 *
 * Return: void
 */
void sigintHandler(__attribute__((unused))int sig_num)
{
	_puts("\n");
	_puts("$ ");
	_putchar(BUF_FLUSH);
}
