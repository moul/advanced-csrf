#ifndef _BLIND_H
# define _BLIND_H

# define TIMEOUT_THREAD		0.5

# define BLIND_TIMEOUT		0.3
# define BLIND_TIMEOUT2		0.7

# define STR(a)			#a
# define XSTR(a)		STR(a)
# define APPEND(a, b)		a ## b
# define APPEND3(a, b, c)	a ## b ## c

# define BLIND_CHARS_MIN	"abcdefghijklmnopqrstuvwxyz"
# define BLIND_CHARS_MAJ	"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
# define BLIND_CHARS_NUM	"0123456789"
# define BLIND_CHARS_SPEC	",./<>?!@#$%^&*()_+[]{}-='\"\\|`~"

# define BLIND_CHARS		BLIND_CHARS_MIN

# define BCHR(pos)		(char)((char*)BLIND_CHARS)[(int)pos]

typedef struct		s_blind
{
  int			nb_threads;
  int			nb_threads_tmp;
  char			*result;
  int			length;
  int			length_tmp;

  int			padding;

  int			st_pos;
  char			st_c;

  char			*buf;
  int			url_size;
  int			query_size;

  char			*url;
  char			*query;
}			t_blind;

#endif /* !_BLIND_H */
