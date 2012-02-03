#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "server.h"

int		http_send_302(t_client *client, char *location, int debug)
{
  static char	*buf = 0;

  if (!buf)
    buf = xmalloc(sizeof(*buf) * BUF_SIZE);
  if (client->action != ACTION_READY)
    return (0);
  client->action = ACTION_DONE;
  sprintf(buf, "Location: %s\r\n", location);

  tunneling_line(client, "HTTP/1.1 302 Found\r\n", debug);
  tunneling_line(client, buf, debug);
  tunneling_line(client, "Cache-Control: private\r\n", 0);
  tunneling_line(client, "Content-Type: text/html; charset=UTF-8\r\n", 0);
  tunneling_line(client, "Server: toto\r\n", 0);
  tunneling_line(client, "Content-Length: 3\r\n", 0);
  tunneling_line(client, "\r\n", 0);
  tunneling_line(client, "302\r\n", 0);
  tunneling_line(client, "\r\n", 0);
  tunneling_line(client, "\r\n", 0);
  if (debug)
    write(client->session->conf.fd_out, location, strlen(location));
  client->session->attack.time = time_get();
  return (1);
}

int		http_send_301(t_client *client, char *location, int debug)
{
  static char	*buf = 0;

  if (!buf)
    buf = xmalloc(sizeof(*buf) * BUF_SIZE);
  if (client->action != ACTION_READY)
    return (0);
  client->action = ACTION_DONE;
  sprintf(buf, "Location: %s\r\n", location);
  tunneling_line(client, "HTTP/1.1 301 Moved Permanently\r\n", 1);
  tunneling_line(client, buf, 1);
  tunneling_line(client, "Cache-Control: private\r\n", 0);
  tunneling_line(client, "Content-Type: text/html; charset=UTF-8\r\n", 0);
  tunneling_line(client, "Server: toto\r\n", 0);
  tunneling_line(client, "Content-Length: 3\r\n", 0);
  tunneling_line(client, "\r\n", 0);
  tunneling_line(client, "301\r\n", 0);
  tunneling_line(client, "\r\n", 0);
  tunneling_line(client, "\r\n", 0);
  if (debug)
    write(client->session->conf.fd_out, location, strlen(location));
  client->session->attack.time = time_get();
  return (1);
}

int	http_send_404(t_client *client)
{
  if (client->action != ACTION_READY)
    return (0);
  client->action = ACTION_DONE;
  tunneling_line(client, "HTTP/1.1 404 Not Found\r\n", 1);
  tunneling_line(client, "Cache-Control: private\r\n", 0);
  tunneling_line(client, "Content-Type: text/html; charset=UTF-8\r\n", 0);
  tunneling_line(client, "Server: toto\r\n", 0);
  tunneling_line(client, "Content-Length: 3\r\n", 0);
  tunneling_line(client, "\r\n", 0);
  tunneling_line(client, "404\r\n", 0);
  tunneling_line(client, "\r\n", 0);
  tunneling_line(client, "\r\n", 0);
  return (1);
}
