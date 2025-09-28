/**
 * filecopy.c
 * 
 * This program copies files using a pipe.
 *
 */

#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <stdlib.h>
#include <sys/wait.h>

#define READ_END 0
#define WRITE_END 1
#define BUFFER_SIZE 25

int main(int argc, char *argv[])
{
	if (argc != 3)
	{
		printf("ERROR! Usage: filecopy <src> <dst>\n");
		exit(1);
	}

	//Taking input and output files from commanline.
	const char *source = argv[1];
	const char *destination = argv[2];
	
	//Convert to file descriptors for use with pipe/read/write
	int srcfd = open(source, O_RDONLY);
	if(srcfd == -1)
	{
		printf("Error: Unable to open source file '%s'.\n",source);
		exit(1);
	}

	int dstfd = open(destination, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if(dstfd == -1)
	{
		printf("Error: Unable to open destination file '%s'.\n", destination);
    	close(srcfd);
    	exit(1);
	}
	
	//Creating pipes.
	int pipefd[2];
	if(pipe(pipefd) == -1)
	{
		printf("Pipe failed.\n");
		close(srcfd); close(dstfd);
		exit(1);
	}

	//Creating fork()
	pid_t pid = fork();
	if(pid < 0)
	{
		perror("Fork failed.\n");
		close(pipefd[READ_END]);
    	close(pipefd[WRITE_END]);
    	close(srcfd);
    	close(dstfd);
		exit(1);
	}
	else if(pid == 0)
	{
		close(pipefd[WRITE_END]);
		close(srcfd);

		char buf[BUFFER_SIZE];
		ssize_t n;

		while((n = read(pipefd[READ_END], buf, BUFFER_SIZE)) > 0)
		{
			ssize_t index = 0;
			while(index < n)
			{
				ssize_t m = write(dstfd, buf + index, n - index);
				if(m < 0)
				{
					perror("Child write");
					close(pipefd[READ_END]);
					close(dstfd);
					exit(1);
				}
				index += m;
			}
		}
		if (n < 0) 
		{ // error on read
			perror("child read");
			close(pipefd[READ_END]);
			close(dstfd);
			exit(1);
    	}

		// Clean up
		close(pipefd[READ_END]);
		close(dstfd);
		printf("File successfully copied from '%s' to '%s'\n", source, destination);
		exit(0);
	}
	else
	{
		close(pipefd[READ_END]);
		close(dstfd);
		ssize_t n;
		char buf[BUFFER_SIZE];

		while((n = read(srcfd, buf, BUFFER_SIZE)) > 0)
		{
			ssize_t index = 0;
			while(index < n)
			{
				ssize_t m = write(pipefd[WRITE_END], buf + index, n - index);
				if(m < 0)
				{
					perror("Parent read");
					close(pipefd[WRITE_END]);
					close(srcfd);
					exit(1);
				}
				index += m;
			}
		}
		if (n < 0) 
		{ // error on read
			perror("Parent write");
			close(pipefd[WRITE_END]);
			close(srcfd);
			exit(1);
		}

		// Clean up
		close(pipefd[WRITE_END]);
		close(srcfd);
		
		
		waitpid(pid,NULL,0);
		exit(0);
	}
	
	return 0;
}
