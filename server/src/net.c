#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/filio.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sysexits.h>
#include <unistd.h>

#include "server.h"

void	tunneling_line(struct s_client *client, char *line, int debug)
{
  char			*tmp;
  int			size;

  size = strlen(line);
  tmp = (char *)xmalloc(client->sizeout + size);
  if (debug)
    {
      VERBOSE(printf("\033[%dm[%03d] > %s\033[m",
		     COLOR(client->fd), client->fd, line));
    }
  if (client->sizeout > 0)
    {
      memcpy(tmp, client->buffout, client->sizeout);
      free(client->buffout);
    }
  memcpy(tmp + client->sizeout, line, size);
  client->buffout = tmp;
  client->sizeout += size;
  return ;
}

void		check_routine(t_context *context)
{
  t_session	*session;
  t_client	*client;

  for (client = context->clients; client; client = client->next)
    if (client->action != ACTION_DONE)
      session_do_step(client);
  for (session = context->sessions; session; session = session->next)
    if (session->active && time_get() - session->time > TIMEOUT_SESSION)
      {
	VERBOSE(printf("\033[%d;1m<%03d> - session timeout\033[m\n",
		       COLOR(session->id), session->id));
	session_end(session);
      }
}

void	net_init(t_context *context, int port)
{
  context->fd = create_tcp_server(&port);
  if (context->fd < 0)
    {
      VERBOSE(printf( "[-] Probleme lors de la creation du serveur.\n"));
      exit(1);
    }
}

void			main_loop(t_context *context)
{
  int			max;
  fd_set		rfds;
  fd_set		wfds;
  struct timeval	timeout;
  int			nb;

  timeout.tv_sec = 1;
  timeout.tv_usec = 0;
  while (42)
    {
      max = init_fdset(&rfds, &wfds, context);
      if ((nb = select(max + 1, &rfds, &wfds, NULL, &timeout)) < 0)
	{
	  if (errno == EINTR)
	    continue ;
	  perror("select");
	  exit(1);
	}
      else if (nb)
	{
	  check_fdset(&rfds, &wfds, context);
	  net_traitement(context);
	}
      check_routine(context);
    }
}

void	net_traitement(t_context *context)
{
  char		*line;
  t_client	*tmp;

  tmp = context->clients;
  while (tmp)
    {
      if (tmp->sizein > 0)
	while ((line = check_buffer(tmp)))
	  {
	    parser(tmp, line);
	    free(line);
	  }
      tmp = tmp->next;
    }
  return ;
}

void	accept_new_client(t_context *context)
{
  struct sockaddr_in	in;
  int			fd;
  int			size;

  size = sizeof(in);
  if ((fd = accept(context->fd, (struct sockaddr *)&in,
		   (socklen_t *)&size)) == -1)
    {
      perror("accept");
      exit(1);
    }
  VERBOSE(printf("\033[%dm[%03d] - Connection from %s\033[m\n",
		 COLOR(fd), fd, inet_ntoa(in.sin_addr)));
  add_the_client(context, fd, &in);
  return ;
}

void		write_to_client(t_client *client)
{
  int		ret;
  char		*buf;

  ret = write(client->fd, client->buffout, client->sizeout);
  if (ret < 0)
    {
      del_the_client(client);
      return ;
    }
  if (ret < client->sizeout)
    {
      buf = (char *)xmalloc(client->sizeout - ret);
      memcpy(buf, client->buffout + ret, client->sizeout - ret);
      free(client->buffout);
      client->buffout = buf;
      client->sizeout -= ret;
    }
  else
    {
      free(client->buffout);
      client->sizeout = 0;
      client->buffout = NULL;
    }
  if (client->action == ACTION_DONE)
    del_the_client(client);
  return ;
}

char		read_from_client(t_client *client)
{
  int		size;
  int		res;

  if (xioctl(client, client, &size))
    return (1);
  if (size == 0)
    {
      del_the_client(client);
      return (1);
    }
  client->buffin = xmalloc(size);
  if ((res = read(client->fd, client->buffin, size)) < 0)
    {
      del_the_client(client);
      return (1);
    }
  client->sizein += size;
  return (0);
}

int	init_fdset(fd_set *rfds, fd_set *wfds, t_context *context)
{
  int		max;
  t_client	*tmp;

  FD_ZERO(rfds);
  FD_ZERO(wfds);
  max = -1;

  FD_SET(context->fd, rfds);
  if (context->fd > max)
    max = context->fd;
  tmp = context->clients;
  while (tmp)
    {
      if (tmp->fd != -1)
	{
	  if (set_my_fd(tmp, rfds, wfds, SET_READ_WRITE) > max)
	    max = tmp->fd;
	}
      tmp = tmp->next;
    }
  return (max);
}

void	check_fdset(fd_set *rfds, fd_set *wfds, t_context *context)
{
  t_client	*tmp;

  if (FD_ISSET(context->fd, rfds))
    accept_new_client(context);
  tmp = context->clients;
  while (tmp)
    {
      if (tmp->fd >= 0)
	{
	  if (FD_ISSET(tmp->fd, rfds))
	    if (read_from_client(tmp))
	      return ;
	  if (tmp->buffout != NULL && FD_ISSET(tmp->fd, wfds))
	    write_to_client(tmp);
	}
      tmp = tmp->next;
    }
  return ;
}

char	*check_buffer(t_client *client)
{
  char	*line;
  int	i;

  line = NULL;
  for (i = 0; i <= client->sizein - 1; i++)
    {
      if (!(strncmp(client->buffin + i, "\n", 1)))
	{
	  line = (char *)xmalloc(i + 1);
	  memcpy(line, client->buffin, i);
	  line[i] = '\0';
	  client->buffin = remove_by_offset(client->buffin, client->sizein,
					    i + 1);
	  client->sizein -= i + 1;
	  return (line);
	}
    }
  return (NULL);
}

char		*remove_by_offset(char *original, int size, int offset)
{
  char		*tmp;

  if (size == offset)
    {
      free(original);
      return (NULL);
    }
  tmp = (char *)xmalloc(size - offset);
  memcpy(tmp, original + offset, size - offset);
  free(original);
  return (tmp);
}

int			create_tcp_server(int *port)
{
  struct sockaddr_in	server_sock;
  int			server_fd;
  int			opt;
  struct protoent	*pe;

  pe = getprotobyname("tcp");
  server_fd = xsocket(pe);
  opt = 1;
  setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR,
	     (void *)&opt, sizeof(opt));
  server_sock.sin_family = AF_INET;
  server_sock.sin_addr.s_addr = INADDR_ANY;
  if (*port)
    {
      server_sock.sin_port = htons(*port);
      if ((xbind(server_fd, (struct sockaddr *)&server_sock)) < 0)
	return (-1);
    }
  else
    return (-1);
  if (xlisten(server_fd) < 0)
    return (-1);
  return (server_fd);
}

/* Usefull for init_fdset */
/* SET_ONLY_READ	2 */
/* SET_ONLY_WRITE	4 */
/* SET_READ_WRITE	6 */
int	set_my_fd(struct s_client *client, fd_set *rfds, fd_set *wfds, int flag)
{
  if ((flag & SET_ONLY_READ))
    FD_SET(client->fd, rfds);
  if ((flag & SET_ONLY_WRITE)
      && client->sizeout)
    FD_SET(client->fd, wfds);
  return (client->fd);
}
