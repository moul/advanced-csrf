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

int	file_exists(char *file)
{
  return (!access(file, R_OK));
}

void	usage(char *progname)
{
  printf("usage : %s [port]\n", progname);
  return ;
}

int	xbind(int server_fd, struct sockaddr *server_sock)
{
  if (bind
      (server_fd, (struct sockaddr *)server_sock,
       sizeof(struct sockaddr_in)) < 0)
    {
      shutdown(server_fd, 2);
      close(server_fd);
      perror("bind");
      return (-1);
    }
  return (0);
}

int	xlisten(int server_fd)
{
  if (listen(server_fd, 42) < 0)
    {
      shutdown(server_fd, 2);
      close(server_fd);
      perror("listen");
      return (-1);
    }
  return (0);
}

int	xsocket(struct protoent *pe)
{
  int	server_fd;

  if ((server_fd = socket(AF_INET, SOCK_STREAM, pe->p_proto)) < 0)
    {
      perror("socket");
      return (-1);
    }
  return (server_fd);
}

int	xioctl(t_client *tmp, t_client *client, int *size)
{
  if(ioctl(tmp->fd, FIONREAD, size) < 0)
    {
      perror("ioctl");
      del_the_client(client);
      return (1);
    }
  return (0);
}

void	*xmalloc(unsigned int size)
{
  void	*tmp;

  if ((tmp = malloc(size)) == NULL)
  {
    VERBOSE(printf("Out of memory"));
    exit(1);
  }
  return (tmp);
}
