#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pwd.h>

#define MAX_LEN 512
#define MAXARGS 10
#define ARGLEN 30

int argnum=0;
int argnum1=0;

void print(char** x,int a){
    for(int i=0;i<a;i++){
        for(int j=0;j<ARGLEN;j++){
            if(x[i][j]!='\0')
                printf("%d:%d  %c",i,j,x[i][j] );
        }
        printf("\n");
    }
}

int execute(char** arglist){
	int status;
   	int cpid = fork();
	if(cpid== -1){
    	perror("fork failed");
      	exit(1);
    }
  	if(cpid==0){	
        signal(2,SIG_DFL);
        signal(3,SIG_DFL);
        if(arglist[argnum-1][0]=='&'){
            arglist[argnum-1]=NULL;
            argnum--;
        }
  		for(int i=0;i<argnum;i++){
            if(arglist[i][0]=='<'){
        	    close(0);
  			    int fd= open(arglist[i+1],O_RDWR);
                for(int j=i;j<argnum-2;j++){
                    arglist[j]=arglist[j+2];
                }
                arglist[argnum-2]=NULL;
                arglist[argnum-1]=NULL;
                argnum-=2;
                i--;
  			}
            if(arglist[i][0]=='>'){
                close(1);
                int fd= open(arglist[i+1],O_RDWR | O_CREAT,0755);
                for(int j=i;j<argnum-2;j++){
                    arglist[j]=arglist[j+2];
                }
                arglist[argnum-2]=NULL;
                arglist[argnum-1]=NULL;
                argnum-=2;
                i--;
            }
  		}
    	// for(int l=0;l<argnum;l++){
  		// 	for(int m=0;m<ARGLEN;m++){
  		// 		if(arglist[l][m]!='\0')
  		// 			printf("%c",arglist[l][m] );
  		// 	}
  		// 	printf("\n");
  		// }
        int rv=execvp(arglist[0], arglist);
    	if(rv==-1){
    		printf("exec not successful\n");
    	}
        perror("Command not found...");
        exit(1);
    }
  	else{
        if(arglist[argnum-1][0]=='&'){
            waitpid(cpid, &status, WNOHANG);
        }
        else waitpid(cpid, &status, 0);
     	//printf("child exited with status %d \n", status >> 8);
     	return 0;
    }
}

char ** tokenizeBySemiColon(char* cmdline){
    char** arglist = (char**)malloc(sizeof(char*)* (MAXARGS+1));
    for(int j=0; j < MAXARGS+1; j++){
        arglist[j] = (char*)malloc(sizeof(char)* ARGLEN);
        bzero(arglist[j],ARGLEN);
    }
    if(cmdline[0] == '\0')//if user has entered nothing and pressed enter key
        return NULL;
    argnum1 = 0; //slots used
    char*cp = cmdline; // pos in string
    char*start;
    int len;
    while(*cp != '\0'){
        while(*cp == ' ' || *cp == '\t' ||  *cp==';') //skip leading spaces
            cp++;
        start = cp; //start of the word
        len = 1;
        //find the end of the word
        while(*++cp!='\0' && *cp != ';')
            len++;
        strncpy(arglist[argnum1], start, len);
        arglist[argnum1][len] = '\0';
        argnum1++;
    }
    arglist[argnum1] = NULL;
    return arglist;
}

char ** tokenize(char* cmdline){
	char** arglist = (char**)malloc(sizeof(char*)* (MAXARGS+1));
   	for(int j=0; j < MAXARGS+1; j++){
	   	arglist[j] = (char*)malloc(sizeof(char)* ARGLEN);
      	bzero(arglist[j],ARGLEN);
    }
   	if(cmdline[0] == '\0')//if user has entered nothing and pressed enter key
      	return NULL;
   	argnum = 0; //slots used
   	char*cp = cmdline; // pos in string
   	char*start;
   	int len;
   	while(*cp != '\0'){
      	while(*cp == ' ' || *cp == '\t') //skip leading spaces
          	cp++;
      	start = cp; //start of the word
      	len = 1;
      	//find the end of the word
      	while(*++cp != '\0' && !(*cp ==' ' || *cp == '\t'))
        	len++;
      	strncpy(arglist[argnum], start, len);
      	arglist[argnum][len] = '\0';
      	argnum++;
   	}
   	arglist[argnum] = NULL;
   	return arglist;
}

void saveInFile(char* cmdline){
    FILE* fp=fopen("history.txt","a+");
    char s[1024];
    char last[1024];
    char n[10]="\0";
    fseek(fp,0,SEEK_SET);
    int flag=0;
    while(!feof(fp)){
        flag=1;
        fgets(s,1024,fp);
        strncpy(last,s,1024);
    }
    int no=0;
    if(flag==1){
        int i=0;
        while(last[i]!=' '){
            n[i]=last[i];
            i++;
        }
        no=atoi(n);
    }
    no+=1;

    fprintf(fp, "%d    ", no);
    fprintf(fp,"%s\n",cmdline );
    fclose(fp);
}

int getCommandFromFile(char* cmdline){
    FILE* fp=fopen("history.txt","a+");
    char type='\0';
    int totalno=0;
    char n[10]="\0";
    if(cmdline[1]!='-'){
        int i=1,k=0;
        while(cmdline[i]!='\0'){
            n[k]=cmdline[i];
            i++;k++;
        }

        type='+';
    }
    else {
        int i=2,k=0;
        while(cmdline[i]!='\0'){
            n[k]=cmdline[i];
            i++;k++;
        } 
        type='-';
        fseek(fp,0,SEEK_SET);
        char last[1024],s[1024],no[10];
        while(!feof(fp)){
            fgets(s,1024,fp);
            strncpy(last,s,1024);
        }
        i=0;
        while(last[i]!=' '){
            no[i]=s[i];
            i++;
        }   
        totalno=atoi(no);
        totalno++;
    }
    int no=atoi(n);
    if(type=='-')
        no=totalno-no;
    char s[1024];
    fseek(fp,0,SEEK_SET);
    while(!feof(fp)){
        fgets(s,1024,fp);
        char n2[10]="\0";
        int i=0;
        while(s[i]!=' '){
            n2[i]=s[i];
            i++;
        }   
        int no2=atoi(n2);
        if(no2==no){
            int k=0;
            while(k<100){
                cmdline[k]='\0';
                k++;
            }
            while(s[i]==' ')
                i++;
            k=0;
            while(s[i]!='\0'){
                cmdline[k]=s[i];
                k++;i++;
            }
            cmdline[--k]='\0';
            fclose(fp);
            return 1;
        }
        //strncpy(last,s,1024);
    }
    fclose(fp);
    return -1;
}

char* read_cmd(char* prompt, FILE* fp){
   	uid_t uid = getuid();
    struct passwd *psw = getpwuid(uid);

    printf("%s @%s $",psw->pw_name, prompt);
  	int c; //input character
   	int pos = 0; //position of character in cmdline
   	char* cmdline = (char*) malloc(sizeof(char)*MAX_LEN);
   	while((c = getc(fp)) != EOF){
       	if(c == '\n')
	  		break;
       	cmdline[pos++] = c;
   	}
	//these two lines are added, in case user press ctrl+d to exit the shell
   	if(c == EOF && pos == 0) 
      	return NULL;
   	cmdline[pos] = '\0';
    if(cmdline[0]!='!' && cmdline!="\0" && cmdline!="" && cmdline!="\n")
        saveInFile(cmdline);
   	return cmdline;
}


int main(){
	signal(2,SIG_IGN);
	signal(3,SIG_IGN);
	
	char *cmdline;
    char**arglist1;
   	char** arglist;
   	char prompt[1024];
   	getcwd(prompt,1024);   
   	while((cmdline = read_cmd(prompt,stdin)) != NULL){
        if(cmdline[0]=='!'){
            int rv= getCommandFromFile(cmdline);
            if(rv==-1){
                printf("No commands in history.\n");
                continue;
            }
        }
    	if((arglist1=tokenizeBySemiColon(cmdline))!=NULL){
            int i=0;
            while(i<argnum1){
                if((arglist = tokenize(arglist1[i])) != NULL){
                    execute(arglist);
                    //  need to free arglist
                    for(int j=0; j < MAXARGS+1; j++)
                        free(arglist[j]);
                    free(arglist);
                    
                }
                i++;
            }
            for(int j=0; j < MAXARGS+1; j++)
                free(arglist1[j]);
            free(arglist1);            
        }
        free(cmdline);
  	}//end of while loop
   	printf("\n");
   	return 0;
}
