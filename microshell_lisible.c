#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>


void	error(char *s)
{
	while (*s)
		write(2, s++, 1);
}

int	cd(char **av, int i)
{
	if (i != 2)
	{
		error("error: cd: bad arguments\n");
		return (1);
	}
	if (chdir(av[1]) == -1)
	{
		error("error: cd: cannot change directory to ");
		error(av[1]);
		error("\n");
		return (1);
	}
	return (0);
}

void	set_pipe(int has_pipe, int *fd, int end)
{
	if (!has_pipe)
		return ;
	if (dup2(fd[end], end) == -1)
	{
		error("error: fatal\n");
		exit(1);
	}
	if (close(fd[0]) == -1 || close(fd[1]) == -1)
	{
		error("error: fatal\n");
		exit(1);
	}
}

int	exec(char **av, int i, char **envp)
{
	int		fd[2];
	int		pid;
	int		ret;
	int		has_pipe;

	has_pipe = (av[i] && !strcmp(av[i], "|"));
	if (!has_pipe && !strcmp(*av, "cd"))
		return (cd(av, i));
	if (has_pipe && pipe(fd) == -1)
	{
		error("error: fatal\n");
		exit(1);
	}
	pid = fork();
	if (pid == -1)
	{
		error("error: fatal\n");
		exit(1);
	}
	if (pid == 0)
	{
		av[i] = 0;
		set_pipe(has_pipe, fd, 1);
		if (!strcmp(*av, "cd"))
			exit(cd(av, i));
		execve(*av, av, envp);
		error("error: cannot execute ");
		error(*av);
		error("\n");
		exit(1);
	}
	waitpid(pid, &ret, 0);
	set_pipe(has_pipe, fd, 0);
	if (WIFEXITED(ret))
		return (WEXITSTATUS(ret));
	return (0);
}

int main(int ac, char **av, char **envp)
{
	int i = 0;
	int	ret = 0;
	(void)ac;

	while (av[i])
	{
		av = &av[i + 1];
		i = 0;

		while (av[i] && strcmp(av[i], "|") && strcmp(av[i], ";"))
			i++;

		if (i)
			ret = exec(av, i, envp);
	}

	return ret;
}