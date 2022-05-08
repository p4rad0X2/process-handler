#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include<cstring>
#include <readline/readline.h>
#include <readline/history.h>
#include <sys/types.h>
#include <sys/wait.h>

using namespace std;

int ct = 0; //to count no of process ids used


pid_t totpid[128]; //store details of all processes
char* fpath[128]; //store details of filepaths
string stat[128]; //store the status of the process, i.e, if a program is running / stopped / terminated / completed
char type[128]; //store the type of the process - if it is a foreground process or a background process

pid_t pid; //store the value of a process id
char* val[] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}; //store values inputted by the user

void update(pid_t pid, int status)  //function to check the completion of the process and will update status
{
  if (pid == -1) //checking if process id is invalid
    return;

  for(int i=0; i<ct; i++) //checking all the process id's against this process id to find the correct process
  {
    if(totpid[i]==pid)
    {
      if (status == 0) //checking to see if the status of the process is returned as completed
      {
        cout<<"Process "<<totpid[i]<<" completed"<<endl;  //printing output to the user
        stat[i] = "completed"; //setting status of the process to completed

      }

      break;
    }
  }

}

void bg() //function to handle background processes
{
  char filename[50] = "./";  //adding ./ to the filename
  strcat(filename, val[0]);  //concating the file name to get an output like ./test

  pid = fork();  //forking the process

  setpgid(0,0); //ensuring that the child process is moved to a new process group so that there is only one process in the foreground shell

  //cout << pid << endl;
  totpid[ct] = pid; //adding the bg process's pid to the list of all pids
  fpath[ct] = val[0]; //adding the bg process's file path to list of all file paths
  type[ct] = 'b'; //setting the type of the process as a bg process
  stat[ct] = "running"; //setting the status of the process as running
  ++ct; //incrementing the counter



  if (pid == 0)  //checking to see if the process was created
  {
    if (execvp(filename, val) < 0)
    {
      cout << "child proc" << endl;
    }
  }
  else if (pid < 0) //error handling
  {
    cout<<"Fork Failed"<<endl;
  }
}

void fg()  //foreground function
{
  char filename[50] = "./"; //adding ./ to the filename
  strcat(filename, val[0]);  //concating the file name to get an output like ./test

  pid = fork(); //forking the process
  setpgid(0,0); //ensuring that the child process is moved to a new process group so that there is only one process in the foreground shell


  if(pid > 0)  //checking for valid pid
  {
    //cout << pid << endl;
    totpid[ct] = pid; //adding values to the list of all values
    fpath[ct] = val[0];
    type[ct] = 'f'; //assigning type as fg
    stat[ct] = "running";
    ++ct; //incrementing counter
    int status;
    pid_t id = waitpid(-1, &status, WUNTRACED); //getting process id
    update(id, status); //updating the status of the process using the update function

  }
  else if (pid == 0)  //checking to see if the process id is 0
  {
    if (execvp(filename, val) < 0) //calling the execvp function for the child process
    {
      cout << "child proc" << endl;
    }
  }

  else //error handling
  {
    cout<<"Fork Failed"<<endl;
  }
}

void list() //list function
{
  for (int i=0; i<ct; i++)
  {
    if(stat[i] == "running"|| stat[i]== "stopped") //checking to see if the status of processes are running or stopped
      cout<<totpid[i]<<": "<<stat[i]<<" "<<fpath[i] <<endl; //listing details of running/stopped processes
  }
}

void exit() //exit function
{
  for(int i=0; i<ct; i++)
  {
    if(stat[i]=="running"|| stat[i]=="stopped")  //checking if the status of the processes is running or stopped
    {
      kill(totpid[i], SIGTERM); //killing said processes
      stat[i] = "terminated"; //setting their status to terminated
      cout<<"Process "<<totpid[i]<<" terminated"<<endl; //sending output that the processes have been terminated
    }
  }
}

void stop(int sig) //stop function
{
  if (type[ct - 1] == 'f' && stat[ct - 1] == "running") //checking if the type is foreground and if the process is running
  {
    cout<<"Process "<<totpid[ct - 1]<<" stopped"<<endl; //sending output that the process has been stopped
    stat[ct - 1] = "stopped"; //updating the status of the process to stopped
    kill(totpid[ct - 1],SIGTERM); //killing the process
  }
}

void terminate(int sig) //terminate function
{
  if (type[ct - 1] == 'f' && stat[ct - 1] == "running") //checking if the type is foreground and if the process is running
  {
    cout<<"Process "<<totpid[ct - 1]<<" terminated"<<endl; //sending output that the process is terminated
    stat[ct - 1] = "terminated"; //setting the status to terminated
    kill(totpid[ct - 1],SIGTERM); //killing the process
  }
}

void handler(int sig) //handler function to check for completion
{
  int status;
  pid_t temp = waitpid(-1, &status, WNOHANG | WUNTRACED);
  update(temp, status); //calling the update function to check process completion
}

int main()
{

  signal(SIGINT, terminate); // Ctrl+C interrupt
  signal(SIGTSTP, stop); // Ctrl+Z interrupt
  signal(SIGCHLD, handler); // Exit command

  while(true)
  {
    char* input = NULL;

    input = readline("sh >"); //reading in input
    char* p = strtok(input, " \t");

    if(!p)
      continue;

    else if(strcmp(p, "fg")==0) //checking if it is a fg process
    {
      *val = NULL; //setting all values to null
      int i=0;
      while(p = strtok(NULL, " \t"))
      {
        val[i++] = p; //getting values from the input
      }
      fg(); //calling fg function
    }

    else if(strcmp(p, "bg")==0) //checking if it is a bg process
    {
      *val = NULL;
      // val[] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}; //setting all values to null
      int i=0;
      while(p = strtok(NULL, " \t"))
      {
        val[i++] = p;
      }
      bg(); //calling bg function
    }

    else if(strcmp(p, "list")==0) //if input is list then calling the list function
    {
      list();
    }

    else if(strcmp(p, "exit")==0) //if input is exit then calling the exit function
    {
      exit();
      break;
    }

    else //error handling
    {
      cout<<"Incorrect command"<<endl;
    }
  }
  return 0;
}
