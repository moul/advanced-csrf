#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "server.h"

int	conf_cmd_302(t_client *client)
{
  http_send_302(client, client->session->conf.line + 4, 0);
  return (1);
}

int	conf_cmd_301(t_client *client)
{
  http_send_301(client, client->session->conf.line + 4, 0);
  return (1);
}

int	conf_cmd_404(t_client *client)
{
  http_send_404(client);
  return (1);
}

int	conf_cmd_file(t_client *client)
{
  char	*conf;

  fclose(client->session->conf.file);
  client->session->conf.file = 0;
  conf = client->session->conf.line + 5;
  if (file_exists(conf))
    {
      client->session->conf.file = fopen(conf, "r");
      VERBOSE(printf("\033[%d;1m<%03d> - conf : %s\033[m\n",
		     COLOR(client->session->id), client->session->id, conf));
      /*
	write(client->session->conf.fd_out, client->session->conf.line,
	strlen(client->session->conf.line));
      */
    }
  return (session_do_step(client));
}

static t_conf_cmd		conf_cmds[] =
  {
    {"301",			conf_cmd_301},
    {"302",			conf_cmd_302},
    {"404",			conf_cmd_404},
    {"FILE",			conf_cmd_file},
    {"BLIND",			conf_cmd_blind},
    {NULL,			NULL}
  };

int	session_parse_line(t_client *client)
{
  int	i;

  for (i = 0; conf_cmds[i].command != NULL; i++)
    if (!(strncmp(conf_cmds[i].command, client->session->conf.line,
		  strlen(conf_cmds[i].command))))
      return (conf_cmds[i].action(client));
  VERBOSE(printf("Unknown command : [%s]\n", client->session->conf.line));
  return (0);
}

void		session_get_line(t_client *client)
{
  char		*conf;

  if (!client->session->conf.file)
    {
      if (file_exists(client->session->host))
	conf = client->session->host;
      else
	conf = "default";
      client->session->conf.file = fopen(conf, "r");
      VERBOSE(printf("\033[%d;1m<%03d> - conf : %s\033[m\n",
		     COLOR(client->session->id), client->session->id, conf));
    }
  if (!fgets(client->session->conf.line, BUF_SIZE, client->session->conf.file))
    {
      fclose(client->session->conf.file);
      client->session->conf.file = 0;
      session_get_line(client);
    }
  else if (client->session->conf.line[strlen(client->session->conf.line) - 1]
	   == '\n')
    client->session->conf.line[strlen(client->session->conf.line) - 1] = 0;
}

int		session_do_step(t_client *client)
{
  if (client->action == ACTION_READY && !client->session->attack.in_progress)
    session_get_line(client);
  client->session->attack.step++;
  /*
    printf("%d %s\n", client->fd, client->session->conf.line);
  */
  /*
    VERBOSE(printf("\033[%d;1m<%03d> - %d attack_step\033[m\n",
    COLOR(client->session->id), client->session->id,
    client->session->attack.step));
  */
  session_parse_line(client);
  return (1);
}

t_session	*session_new(t_context *context, char *host)
{
  t_session	*session;
  t_session	*tmp;
  char		*tmp_buf;

  session = xmalloc(sizeof(*session));
  session->host = strdup(host);
  session->id = 0;

  session->nb_clients = 0;

  session->attack.time = 0;
  session->attack.in_progress = 0;
  session->attack.step = 0;
  session->attack.ptr = 0;

  tmp_buf = xmalloc(sizeof(*tmp_buf) * (strlen(host) + 5));
  memcpy(tmp_buf, "out.", 4);
  memcpy(tmp_buf + 4, host, strlen(host) + 1);
  session->conf.fd_out = open(tmp_buf, O_WRONLY | O_CREAT | O_APPEND, 0700);
  free(tmp_buf);
  session->conf.file = 0;
  session->conf.line = xmalloc(sizeof(*session->conf.line) * BUF_SIZE);

  session->next = NULL;
  if (!context->sessions)
    {
      context->sessions = session;
      return (session);
    }
  session->id++;
  for (tmp = context->sessions; tmp->next; tmp = tmp->next)
    session->id++;
  tmp->next = session;
  return (session);
}

void		session_free(t_session *session)
{
  free(session);
  return ;
}

t_session	*session_get(t_context *context, struct sockaddr_in *in)
{
  char		*host;
  t_session	*tmp;

  host = inet_ntoa(in->sin_addr);
  for (tmp = context->sessions; tmp; tmp = tmp->next)
    if (!strcmp(tmp->host, host))
      return (tmp);
  return (session_new(context, host));
}

void		session_write_time(t_session *session)
{
  static char	*buf = 0;

  if (!buf)
    buf = xmalloc(sizeof(*buf) * 20);
  snprintf(buf, 19, "\t%f\n", time_get() - session->attack.time);
  write(session->conf.fd_out, buf, strlen(buf));
}

void		session_end(t_session *session)
{
  t_client	*tmp;
  t_context	*context;

  if (session->attack.step++ && session->attack.time)
    {
      session_write_time(session);
      session->attack.time = 0;
    }
  VERBOSE(printf("\033[%d;1m<%03d> - session end (%d)\033[m\n",
		 COLOR(session->id), session->id, session->attack.step));
  context = get_context_ptr((void *)0);
  for (tmp = context->clients; tmp; tmp = tmp->next)
    if (tmp->session == session)
      {
	http_send_404(tmp);
	del_the_client(tmp);
      }
  session->active = 0;
  return ;
}

void		session_delete(t_session *session)
{
  t_context	*context;
  t_session	*tmp;
  t_session	*tmp2;

  session_end(session);
  context = get_context_ptr((void *)0);
  VERBOSE(printf("\033[%d;1m<%03d> - session delete\033[m\n",
		 COLOR(session->id), session->id));
  tmp = context->sessions;
  if (tmp == session)
    {
      context->sessions = tmp->next;
      session_free(tmp);
      return ;
    }
  tmp2 = tmp;
  tmp = tmp->next;
  for (tmp2 = tmp, tmp = tmp->next; tmp && tmp != session;
       tmp = tmp->next, tmp2 = tmp2->next)
    ;
  if (tmp == session)
    {
      tmp2->next = tmp->next;
      session_free(tmp);
    }
  return ;
}
