#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "server.h"

t_client	*add_the_client(t_context *context, int fd,
				struct sockaddr_in *in)
{
  t_client	*tmp;
  t_client	*tmp_;

  tmp_ = context->clients;
  tmp = xmalloc(sizeof(t_client));
  init_client(tmp);
  tmp->next = NULL;
  tmp->fd = fd;
  tmp->action = ACTION_NONE;
  tmp->session = session_get(context, in);
  tmp->session->active = 1;
  tmp->session->time = time_get();
  tmp->session->nb_clients++;
  VERBOSE(printf("\033[%d;1m<%03d> - %d clients (new)\033[m\n",
		 COLOR(tmp->session->id), tmp->session->id,
		 tmp->session->nb_clients));
  if (!tmp_)
    {
      context->clients = tmp;
      return (tmp);
    }
  while (tmp_->next)
    tmp_ = tmp_->next;
  tmp_->next = tmp;
  return (tmp);
}

void	del_client(t_client *client)
{
  close(client->fd);
  client->fd = -1;
  if (client->sizein)
    free(client->buffin);
  if (client->sizeout)
    free(client->buffout);
  client->buffin = NULL;
  client->buffout = NULL;
  return ;
}

void	init_client(t_client *client)
{
  client->fd = -1;
  client->buffin = NULL;
  client->buffout = NULL;
  client->sizein = 0;
  client->sizeout = 0;
  client->next = NULL;
}

void	del_the_client(t_client *client)
{
  t_client	*tmp;
  t_client	*tmp2;
  t_context	*context;

  client->session->nb_clients--;
  VERBOSE(printf("\033[%dm[%03d] - Disconnected\033[m\n",
		 COLOR(client->fd), client->fd));
  VERBOSE(printf("\033[%d;1m<%03d> - %d clients (del)\033[m\n",
		 COLOR(client->session->id), client->session->id,
		 client->session->nb_clients));
  context = get_context_ptr((t_context *)0);
  tmp = context->clients;
  if (tmp->fd == client->fd)
    {
      context->clients = tmp->next;
      del_client(tmp);
      free(tmp);
      return ;
    }
  tmp2 = context->clients;
  tmp = tmp2->next;
  while (tmp->fd != client->fd)
    {
      tmp = tmp->next;
      tmp2 = tmp2->next;
    }
  tmp2->next = tmp->next;
  del_client(tmp);
  free(tmp);
  return ;
}
