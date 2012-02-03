#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "server.h"

int	cmd_null(t_client *client, char *line)
{
  (void)client;
  (void)line;
  return (1);
}

int	cmd_debug(t_client *client, char *line)
{
  (void)client;
  (void)line;
  VERBOSE(printf("\033[%dm[%03d] < %s\033[m\n",
		 COLOR(client->fd), client->fd, line));
  return (1);
}

static struct s_cmd		cmds[] =
  {
    {"GET",			cmd_debug},
    {"HOST",			cmd_debug},

    {"ACCEPT",			cmd_null},
    {"ACCEPT-LANGUAGE",		cmd_null},
    {"ACCEPT-ENCODING",		cmd_null},
    {"ACCEPT-CHARSET",		cmd_null},
    {"KEEP-ALIVE",		cmd_null},
    {"CONNECTION",		cmd_null},
    {"USER-AGENT",		cmd_null},
    {"REFERER",			cmd_null},
    {NULL,			NULL}
  };

int	parser(t_client *client, char *line)
{
  int	i;

  client->session->last = client->fd;
  client->session->time = time_get();
  client->time = client->session->time;
  for (i = 0; line[i] && line[i] != ' '; i++)
    if (line[i] >= 'a' && line[i] <= 'z')
      line[i] = line[i] - 'a' + 'A';
  if (line[strlen(line) - 1] == '\r')
    line[strlen(line) - 1] = 0;
  for (i = 0; cmds[i].command != NULL; i++)
    if (!(strncmp(cmds[i].command, line, strlen(cmds[i].command))))
      return (cmds[i].action(client, line));
  if (!strlen(line)) /* FIX: ptet un bug dans le cas du \r */
    {
      client->action = ACTION_READY;
      return (session_do_step(client));
    }
  VERBOSE(printf("Unknown command : [%s]\n", line));
  return (0);
}
