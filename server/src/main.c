#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "server.h"

static struct s_context	*g_context;

int	main(int argc, char **argv)
{
  t_context	context;

  if (argc < 2)
    {
      (void) usage(argv[0]);
      return (1);
    }
  VERBOSE(printf("[+] Initialisation\n"));
  init(&context);
  net_init(&context, atoi(argv[1]));
  VERBOSE(printf("[+] Server's listening...\n"));
  main_loop(&context);
  return (0);
}

void	init(t_context *context)
{
  g_context = context;
  get_context_ptr(context);
  context->fd = -1;
  context->clients = NULL;
  context->sessions = NULL;
  chdir("confs");
}

t_context		*get_context_ptr(t_context *ctx)
{
  static t_context	*ptr = (t_context *)-1;

  if (ptr == (t_context *)-1)
    ptr = ctx;
  return (ptr);
}
