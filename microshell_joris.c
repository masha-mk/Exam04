#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>

void	error(char *s)
{
	while (*s)
		write(2, s++, 1);
}

int cd(char **av, int i)
{
	if (i != 2)
		return (error("error: cd: bad arguments\n"), 1);

	if (chdir(av[1]) == -1)
		return (error("error: cd: cannot change directory to "), error(av[1]), error("\n"), 1);

	return (0);
}

void	set_pipe(int has_pipe, int *fd, int end)
{
	if (has_pipe && (dup2(fd[end], end) == -1 || close(fd[0]) == -1 || close(fd[1]) == -1))
		error("error: fatal\n"), exit(1);
}

int exec(char **av, int i, char **envp)
{
	int	fd[2];
	int	pid;
	int	ret;
	int	has_pipe = av[i] && !strcmp(av[i], "|");

	//S'il n'y a pas de pipe (apres la commande actuelle) && la commande actuelle est cd
	//Appelle la fonction cd
	if (!has_pipe && !strcmp(*av, "cd"))
		return cd(av, i);
	
	//S'il y a un pipe et la fonction pipe retourne une erreur
	//Affiche le message d'erreur et exit.
	if (has_pipe && pipe(fd) == -1)
		error("error: fatal\n"), exit(1);

	//Si le fork a echoue
	//Affiche un message d'erreur et exit'
	if ((pid = fork()) == -1)
		error("error: fatal\n"), exit(1);

	//Si fork a bien fonctionne (il a retourne 0)
	if (!pid)
	{
		av[i] = 0;

		//SET_PIPE
		set_pipe(has_pipe, fd, 1);

		//Check cd et exit
		if (!strcmp(*av, "cd"))
			exit(cd(av, i));
	
		//Exec et error
		execve(*av, av, envp);
		error("error: cannot execute "), error(*av), error("\n"), exit(1);
	}

	waitpid(pid, &ret, 0);
	set_pipe(has_pipe, fd, 0);

	return WIFEXITED(ret) && WEXITSTATUS(ret);
}

int main(int ac, char **av, char **envp)
{
	int i = 0;
	int	ret = 0;
	(void)ac;

	while (av[i])
	{
		av += i + 1;
		i = 0;

		while (av[i] && strcmp(av[i], "|") && strcmp(av[i], ";"))
			i++;

		if (i)
			ret = exec(av, i, envp);
	}

	return ret;
}