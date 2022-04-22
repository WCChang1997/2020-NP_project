
#define _USE_BSD
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/signal.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <sys/errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <stdarg.h>
#include <bits/stdc++.h>
#include <sqlite3.h>

#include "bbs1.h"
#include "bbs2.h"

using namespace std;

int errno;
unsigned short portbase = 0;   /* port base, for non-root servers      */

/****************************************************
*                                                   *    
*   enum + map : switch case for command's string   *
*                                                   *        
****************************************************/
enum str_value{
    // OP1
    REGISTER = 1, 
    LOGIN, 
    LOGOUT,
    WHOAMI,
    EXIT,
    // OP2
    CREATE_BOARD,
    LIST_BOARD,
    CREATE_POST,
    LIST_POST,
    READ,
    DELETE_POST,
    UPDATE_POST,
    COMMENT
};

static map<string, str_value> str_map;

void str_map_init(){
    str_map["register"] = REGISTER;
    str_map["login"] = LOGIN;
    str_map["logout"] = LOGOUT;
    str_map["whoami"] = WHOAMI;
    str_map["exit"] = EXIT;
    
    str_map["create-board"] = CREATE_BOARD;
    str_map["list-board"] = LIST_BOARD;
    
    str_map["create-post"] = CREATE_POST;
    str_map["list-post"] = LIST_POST;
    str_map["read"] = READ;
    str_map["delete-post"] = DELETE_POST;
    str_map["update-post"] = UPDATE_POST;
    str_map["comment"] = COMMENT;
}

/****************************************************************
*   Arguments:                                                  *
*       service   - service associated with the desired port    *
*       transport - transport protocol to use ("tcp" or "udp")  *
*       qlen      - maximum server request queue length         *
****************************************************************/
int passivesock(const char *service, const char *transport, int qlen){
    struct servent  *pse;   /* pointer to service information entry */
    struct protoent *ppe;   /* pointer to protocol information entry*/
    struct sockaddr_in sin; /* an Internet endpoint address         */
    int     s, type;        /* socket descriptor and socket type    */

    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;

/* Map service name to port number */
    if ( pse = getservbyname(service, transport) )
        sin.sin_port = htons(ntohs((unsigned short)pse->s_port) + portbase);
    else if ((sin.sin_port=htons((unsigned short)atoi(service))) == 0)
        errexit("can't get \"%s\" service entry\n", service);

/* Map protocol name to protocol number */
    if ( (ppe = getprotobyname(transport)) == 0)
        errexit("can't get \"%s\" protocol entry\n", transport);

/* Use protocol to choose a socket type */
    if (strcmp(transport, "udp") == 0)
        type = SOCK_DGRAM;
    else
        type = SOCK_STREAM;

/* Allocate a socket */
    s = socket(PF_INET, type, ppe->p_proto);
    if (s < 0)
        errexit("can't create socket: %s\n", strerror(s));

/* Bind the socket */
    if (errno=bind(s, (struct sockaddr *)&sin, sizeof(sin)) < 0)
        errexit("can't bind to %s port: %s\n", service, strerror(errno));
    if (type == SOCK_STREAM && listen(s, qlen) < 0)
        errexit("can't listen on %s port: %s\n", service, strerror(type));
    return s;
}

/************************************************************
*   Arguments:                                              *
*       service - service associated with the desired port  *
*       qlen    - maximum server request queue length       *
************************************************************/
int passiveTCP(const char *service, int qlen){
    return passivesock(service, "tcp", qlen);
}


/************************************
*    TCPechod.c - main, TCPechod    *
************************************/
#define QLEN    32    /* maximum connection queue length      */
#define BUFSIZE 4096

int TCPechod(int fd);
int CMDecho (int fd, char *input, int & userID, string & username);
int command_analyze(int fd, const vector<string> &command, int & userID, string & username);

/*------------------------------------------------------------------------
 * main - Concurrent TCP server for ECHO service
 *------------------------------------------------------------------------
 */
int main(int argc, char *argv[]){
    char    *service;      /* service name or port number  */
    struct  sockaddr_in fsin;       /* the address of a client      */
    unsigned int    alen;           /* length of client's address   */
    int     msock;                  /* master server socket         */
    int     ssock;                  /* slave server socket          */

    if (argc !=2)
        errexit("usage: %s port\n", argv[0]);
            
    service = argv[1];
    msock = passiveTCP(service, QLEN);
    (void) signal(SIGCHLD, reaper);
    
    // Initialize the enum + map
    str_map_init();
    
    // create ./post/ direction
    struct stat st_buf;
    if(stat("./post", &st_buf) == -1){
        mkdir("./post", 0700);
    }
    
    while (1) {
        alen = sizeof(fsin);
        ssock = accept(msock, (struct sockaddr *)&fsin, &alen);
        if (ssock < 0) {
            if (errno == EINTR)
                continue;
            errexit("accept: %s\n", strerror(ssock));
        }
        printf("Accept connection %d from %s:%d\n", ssock, inet_ntoa(fsin.sin_addr), (int)ntohs(fsin.sin_port));
        switch (fork()){
        case 0:
            (void) close(msock);
            //printf("echo data\n");
            TCPechod(ssock);
            close(ssock);
            exit(0);
        default:
            close(ssock);
            break;
        case -1:
            errexit("fork: %s\n", strerror(errno));
        }              
    }
    return 0;
}

/*------------------------------------------------------------------------
 * TCPechod - echo data until end of file
 *------------------------------------------------------------------------
 */
int TCPechod(int fd){
    char buf[BUFSIZ];
    char welcome[100] = {"********************************\n** Welcome to the BBS server. **\n********************************\n"};
    char cmd_head[5] = {"% "};
    char nullstr[1] = {""};
    int cc;
    /*** Welcome Message ***/
    printf("New Connection. [PID:%d]\n" , getpid());
    if (errno=write(fd, welcome, strlen(welcome)) < 0){
        errexit("echo write: %s\n", strerror(errno));
    }
    if (errno=write(fd, cmd_head, strlen(cmd_head)) < 0){
        errexit("echo write: %s\n", strerror(errno));
    }
    /*** Receive Operation & Send Message ***/
    int userID = -1;
    string username = "";
    while (cc = read(fd, buf, sizeof buf)) {
        if (cc < 0){
            errexit("echo read: %s\n", strerror(cc));
        }
        printf("---\n");
        printf("From [fd:%d] [PID:%d] : %s", fd, getpid(), buf);
        
        CMDecho(fd, buf, userID, username);
        strncpy(buf, nullstr, BUFSIZ);  // reset buf
        if (errno=write(fd, cmd_head, strlen(cmd_head)) < 0){
            errexit("echo write: %s\n", strerror(errno));
        }
    }
    return 0;
}

int CMDecho (int fd, char *input, int & userID, string & username){
    /*
    char echostr[20] = {"echo : "};
    strncpy(output, echostr, BUFSIZ);
    strncat(output, input, BUFSIZ);
    */
/*** command parsing ***/
    string cmd_in(input);
    //cout << cmd_in.size() << endl;
    cmd_in.erase(remove(cmd_in.begin(), cmd_in.end(), '\n'), cmd_in.end());
    cmd_in.erase(remove(cmd_in.begin(), cmd_in.end(), '\r'), cmd_in.end());
    cmd_in.erase(remove(cmd_in.begin(), cmd_in.end(), '\0'), cmd_in.end());
    //cout << cmd_in.size() << endl;
    vector<string> cmd_tok;
    string substr;
    istringstream istr(cmd_in);
    //getline(istr, substr);
    bool head_cmd = 1;
    bool sub_cmd = 0;
    bool is_title = 0;
    bool is_content = 0;
    bool is_storing = 0;
    
    bool is_comment = 0;
    bool is_get_comment = 0;
    
    while (getline(istr, substr, ' ')) {
        if(substr.length() == 0){
            continue;
        }
        /*** check the need of --title or --content by the head of the cmd ***/
        if(head_cmd){
            if(substr.compare("comment") == 0){
                is_comment = 1;
            }
            if(substr.compare("create-post") == 0 || substr.compare("update-post") == 0){
                sub_cmd = 1;
            }
            head_cmd = 0;
            cmd_tok.push_back(substr);
            continue;
        }
        
        /*** check if content ***/
        if(is_comment){
            if(!is_get_comment){    // first time : get post ID
                cmd_tok.push_back(substr);
                is_get_comment = 1;
            }
            else{
                if(!sub_cmd){ // first time to get comment
                    cmd_tok.push_back(substr);
                    sub_cmd = 1;
                }
                else{
                    cmd_tok.back().append(" ");
                    cmd_tok.back().append(substr);
                }
            }
            continue;
        }
        
        /*** check if meet --title or --content ***/
        if(sub_cmd){
            if(substr.compare("--title") == 0){
                is_title = 1;
                is_content = 0;
                is_storing = 1;
                cmd_tok.push_back(substr);
                continue;
            }
            if(substr.compare("--content") == 0){
                is_title = 0;
                is_content = 1;
                is_storing = 1;
                cmd_tok.push_back(substr);
                continue;
            }
        }
        
        /*** store title or content ***/
        if(sub_cmd){
            if(is_title && !is_content){
                if(is_storing){
                    is_storing = 0;
                    cmd_tok.push_back(substr);
                    continue;
                }
                if(substr.compare("--content") == 0){
                    // change from title to content
                    is_title = 0;
                    is_content = 1;
                    cmd_tok.push_back(substr);
                    continue;
                }
                else{
                    // store title
                    cmd_tok.back().append(" ");
                    cmd_tok.back().append(substr);
                    continue;
                }
            }
            if(!is_title && is_content){
                if(is_storing){
                    is_storing = 0;
                    cmd_tok.push_back(substr);
                    continue;
                }
                if(substr.compare("--title") == 0){
                    // change from title to content
                    is_title = 1;
                    is_content = 0;
                    cmd_tok.push_back(substr);
                    continue;
                }
                else{
                    // store title
                    cmd_tok.back().append(" ");
                    cmd_tok.back().append(substr);
                    continue;
                }
            }
        }
        cmd_tok.push_back(substr);
    }
    // cout << cmd_tok.size() << endl;
/*** command analyzing ***/
    command_analyze(fd, cmd_tok, userID, username);
    return 0;
}

int command_analyze(int fd, const vector<string> &command, int & userID, string & username){
    //cout << command.size() << "\n";
    if(command.size() == 0) return 0;
    string command_head = command.at(0);
    string command_result = "";
    /*
    for(int i = 0 ; i < command.size() ; i++){
        cout << command.at(i) << endl;
    }
    */
    switch(str_map[command_head]){
        // OP1
        case REGISTER:      cmd_reg(fd, command);                           break;
        case LOGIN:         cmd_login(fd, command, userID, username);       break;
        case LOGOUT:        cmd_logout(fd, userID, username);               break;
        case WHOAMI:        cmd_who(fd, userID, username);                  break;
        case EXIT:          cmd_exit(fd);                                   break;    
        // OP2
        case CREATE_BOARD:  cmd_create_board(fd, command, userID, username);break;
        case LIST_BOARD:    cmd_list_board(fd, command, userID, username);  break;
        case CREATE_POST:   cmd_create_post(fd, command, userID, username); break;
        case LIST_POST:     cmd_list_post(fd, command, userID, username);   break;
        case READ:          cmd_read_post(fd, command, userID, username);   break;
        case DELETE_POST:   cmd_delete_post(fd, command, userID, username); break;
        case UPDATE_POST:   cmd_update_post(fd, command, userID, username); break;
        case COMMENT:       cmd_comment_post(fd, command, userID, username);break;
        default:            break;
    }
    
    return 0;
}