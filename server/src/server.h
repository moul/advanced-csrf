#ifndef _SERVER_H
# define _SERVER_H

/**********************/
/* INCLUDES           */
/**********************/

# include <sys/select.h>
# include <sys/types.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <arpa/inet.h>
# include <netdb.h>

/**********************/
/* DEFINES            */
/**********************/

# define COLOR(id)		((id) % 6 + 31)

# define GETIP(A,I)		(((A)>>(I*8))&0xFF)

# define BUF_SIZE		1024

# define SET_ONLY_READ		2
# define SET_ONLY_WRITE		4
# define SET_READ_WRITE		6

# define TIMEOUT_SESSION	(double)5

# define RECALC_THREADS		1

enum
  {
    ACTION_NONE,
    ACTION_READY,
    ACTION_DONE
  };

/**********************/
/* STRUCTURES         */
/**********************/

typedef struct		s_session_attack
{
  void			*ptr;		/* pointer for addons structures*/
  int			in_progress;
  int			step;
  double		time;
}			t_session_attack;

typedef struct		s_session_conf
{
  int			fd_out;
  FILE			*file;
  char			*line;
}			t_session_conf;

typedef struct		s_session
{
  t_session_attack	attack;
  t_session_conf	conf;

  int			last;
  double		time;
  int			active;
  int			nb_clients;
  int			id;		/* just for colorized debug */
  char			*host;
  struct s_session	*next;
}			t_session;

typedef struct		s_client
{
  double		time;
  int			fd;
  int			action;

  char			*buffin;
  int			sizein;
  char			*buffout;
  int			sizeout;

  t_session		*session;
  struct s_client	*next;
}			t_client;

typedef struct		s_cmd
{
  char			*command;
  int			(*action)(t_client *, char *);
}			t_cmd;

typedef struct		s_conf_cmd
{
  char			*command;
  int			(*action)(t_client *);
}			t_conf_cmd;

typedef struct		s_context
{
  int			fd;
  struct s_client	*clients;
  struct s_session	*sessions;
}			t_context;

/**********************/
/* PROTOTYPES         */
/**********************/
int		file_exists(char *);
void		usage(char *);
int		check_session_threads(t_client *);
int		conf_cmd_blind(t_client *);
void		session_write_time(t_session *);
int		http_cache_poisonning(t_client *, char *, char *);
int		http_send_404(t_client *);
int		http_send_302(t_client *, char *, int);
int		http_send_301(t_client *, char *, int);
int		session_do_step(t_client *);
void		session_end(t_session *);
void		session_delete(t_session *);
t_session	*session_get(t_context *, struct sockaddr_in *);
int		set_my_fd(struct s_client *, fd_set *, fd_set *, int);
void		*xmalloc(unsigned int);
int		xioctl(t_client *, t_client *, int *);
int		init_fdset(fd_set *, fd_set *, t_context *);
void		check_fdset(fd_set *, fd_set *, t_context *);
void		init(t_context *);
void		init_client(t_client *);
void		treat_signal(int);
t_context	*get_context_ptr(t_context *);
void		del_the_client(t_client *);
void		del_client(t_client *);
t_client	*add_the_client(t_context *, int, struct sockaddr_in *);
void		net_init(t_context *, int);
void		net_traitement(t_context *);
void		main_loop(t_context *);
int		xbind(int, struct sockaddr *);
int		xlisten(int);
int		xsocket(struct protoent *);
void		accept_new_client(t_context *);
char		read_from_client(t_client *);
void		write_to_client(t_client *);
char		*check_buffer(t_client *);
int		create_tcp_server(int *);
char		*remove_by_offset(char *, int, int);
int		parser(t_client *, char *);
void		tunneling_line(struct s_client *, char *, int);
void		check_routine(t_context *);
double		time_get(void);

/* VERBOSE */
# ifndef NVERBOSE
#  define VERBOSE(Msg)			\
   do					\
   {					\
       (Msg);				\
   }					\
   while (0)
# else
#  define VERBOSE(Msg)
# endif /* !NVERBOSE */

#endif /* !_SERVER_H */
