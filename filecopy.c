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
	//If the input over 3 in commandlines, if will close.
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
	if(srcfd == -1) //if it is failed, print out error and exit
	{
		printf("Error: Unable to open source file '%s'.\n",source);
		exit(1);
	}
	
	//WRONLY : Write only
	//CREAT : for creating file if it does not exit.
	//TRUNC : if the file exits,  its content should be truncated (emptied) to a length of zero.
	//0644 : if file does not exits, creating it with permissions 0644 (owner can read/write, group and others can read)
	int dstfd = open(destination, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if(dstfd == -1) //if it is failed, print out error and exit
	{
		printf("Error: Unable to open destination file '%s'.\n", destination);
    	close(srcfd);
    	exit(1);
	}
	
	//Creating pipes.
	int pipefd[2];
	if(pipe(pipefd) == -1)
	{
		printf("Pipe failed.\n"); //if it is failed, closed file and exit.
		close(srcfd); close(dstfd);
		exit(1);
	}

	//Creating fork()
	pid_t pid = fork();
	if(pid < 0)
	{
		perror("Fork failed.\n"); 
		close(pipefd[READ_END]); //if it is failed, closed pipe and exit
    	close(pipefd[WRITE_END]);
    	close(srcfd); //if it is failed, closed files and exit
    	close(dstfd);
		exit(1);
	}
	
	//Children : Consumer
	else if(pid == 0)
	{
		//Children will read from pipe, and it won't touch write end of pipe or source file.
		//Therefore, closing them.
		close(pipefd[WRITE_END]);
		close(srcfd);
		
		//Copying with buffer size characters.
		char buf[BUFFER_SIZE];
		
		//Used for a count of bytes or an error indication
		//whenever you would return either a size in bytes or a (negative) error value
		ssize_t n; 
		
		//read function will read from the pipe, and return the bytes read from the pipe.
		//So n can be:
		// n < 0: Error issue maybe with files, n > 0: bytes read from the pipe, and n == 0: Done, end of file.
		while((n = read(pipefd[READ_END], buf, BUFFER_SIZE)) > 0)
		{
			ssize_t index = 0;
			
			//Reading each in buffer with 25 each time.
			//Writing them in destination file.
			while(index < n)
			{
				//write will return size of n after reading and writing to destination file.
				ssize_t m = write(dstfd, buf + index, n - index);
				if(m < 0)
				{
					perror("Child write");
					close(pipefd[READ_END]);
					close(dstfd);
					exit(1);
				}
				
				//index will + buffersize in m to compare writing done when it is equal n.
				index += m;
			}
		}
		
		//If reading got error.
		if (n < 0) 
		{ // error on read
			perror("child read");
			close(pipefd[READ_END]);
			close(dstfd);
			exit(1);
    	}
		
		//Close when done
		close(pipefd[READ_END]);
		close(dstfd);
		printf("File successfully copied from '%s' to '%s'\n", source, destination);
		exit(0);
	}
	
	//Parent : Producer
	else
	{
		//Parent will write from source, and it won't touch read end of pipe or destination file.
		//Therefore, closing them.
		close(pipefd[READ_END]);
		close(dstfd);
		
		//Copying with buffer size characters.
		char buf[BUFFER_SIZE];
		
		//Used for a count of bytes or an error indication
		//whenever you would return either a size in bytes or a (negative) error value
		ssize_t n;

		//read function will read from the source, and return the bytes read from the file.
		//So n can be:
		// n < 0: Error issue maybe with files, n > 0: bytes read from the file, and n == 0: Done, end of file.
		while((n = read(srcfd, buf, BUFFER_SIZE)) > 0)
		{
			ssize_t index = 0;
			
			//Reading each in buffer with 25 each time.
			//Writing them in pipe.
			while(index < n)
			{
				//write will return size of n after reading and writing from source file to pipe.
				ssize_t m = write(pipefd[WRITE_END], buf + index, n - index);
				if(m < 0)
				{
					perror("Parent read");
					close(pipefd[WRITE_END]);
					close(srcfd);
					exit(1);
				}
				//index will + buffersize in m to compare writing done when it is equal n.
				index += m;
			}
		}
		//If reading got error.
		if (n < 0) 
		{ // error on read
			perror("Parent write");
			close(pipefd[WRITE_END]);
			close(srcfd);
			exit(1);
		}

		//Close when done
		close(pipefd[WRITE_END]);
		close(srcfd);
		
		//Pause here until child with PID = pid exits;
		waitpid(pid,NULL,0);
		exit(0);
	}
	
	return 0;
}
