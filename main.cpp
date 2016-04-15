#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include <string>
#include <sys/wait.h>
#include <unistd.h>
#include <fstream>
#include <stdlib.h>
#include <sstream>
using namespace std;
//I found this on the internet
#define SSTR( x ) dynamic_cast< std::ostringstream & >( \
        ( std::ostringstream() << std::dec << x ) ).str()

void findOccurances(std::string fileName, string key){
	const char * c= fileName.c_str();
	ifstream input;
	//first checks if the file exists
	input.open(c);
	if(!input.is_open()){
		cout<<fileName<<" doesn't exist."<<endl;
		exit(EXIT_FAILURE);
	}
	//Now some setup for exec like convert the pid(int) to a string so I can append it
	//to the rest of the pieces
    int currentPID = getpid();
    string s = SSTR(currentPID);
	string toPassIn=("grep -o "+key+" "+fileName+" | wc -l > result-"+s+".txt");
    execl("/bin/sh", "/bin/sh", "-c", toPassIn.c_str(),NULL);
	exit(EXIT_SUCCESS);
}

int main(int argc, char **argv){
    string pattern = argv[1];
    int numFiles=argc-2;
    pid_t children[numFiles];
    int status;
    //makes sure there are enough command line arguements
    if(!numFiles>0){
    	cout<<"Please enter a KEY and at least one filename"<<endl;
        return 0;
        }
    //creats all the children and sends them off to check the files
    for (int i=0;i<numFiles;i++){
        pid_t pid = fork();
        children[i]=pid;
        if(pid==-1)
            cout<<"error in forking";
        else if(pid==0){
            //child process
           	findOccurances(argv[i+2],pattern);
            break;
        }
    }
    //waits for all the children
    pid_t waitID;
    for(int j = 0; j < numFiles; j++){
        waitID=waitpid(children[j],&status,0);
        if(waitID==-1)
            perror("waitpid error");
        else if(waitID==children[j]){
            //if the child has stopped, one attempt to restart it will be made
            if(WIFSTOPPED(status) || WIFSIGNALED(status)){
                cout<<children[j]<<" stopped but is being restarted."<<endl;
                pid_t again=fork();
                //if an error occured on the second try of the problem child
                if(again==-1){
                    perror("fork error on childs second try...");
                    exit(EXIT_FAILURE);
                    }
                //if the fork was successful and the child is normal
                else if(again==0){
                	findOccurances(argv[j+2],pattern);
            		break;
                }
                //if it is the parent the new child is added to the space where the 
                //problem child was
                else
                    children[j]=again;
                //waits for the replacement child to finish
                waitID=waitpid(children[j],&status,WNOHANG|WUNTRACED);
                if(waitID==-1){
                    perror("waitpid error on child's second try.");
                    exit(EXIT_FAILURE);
                }
                else if(waitID==children[j]){
                    if (WIFEXITED(status))
                        cout<<"Child's second try was successful."<<endl;
                }
            }
        }
    }
    //count all the occurances
    int counter=0;
    string outFileName;
    for (int l=2;l<argc;l++){
		outFileName = ("result-"+SSTR(children[l-2])+".txt");
    	const char * c= outFileName.c_str();
    	string line;
    	ifstream file(c);
    	if(file.is_open()){
    		while(getline (file, line)){
    			int value = atoi(line.c_str());
    			counter+= value;
			}
    	}
    	else
    		cout<<"Could not open "<<outFileName<<endl;
    }
    cout<<'\n'<<"There are "<<counter<<" '"<<pattern<<"'s"<<" in the text files. ";
    return 0;
}