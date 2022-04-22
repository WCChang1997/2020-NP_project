
#define _USE_BSD
#include <sys/types.h>
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

using namespace std;

/*------------------------------------------------------------------------
 * reaper - clean up zombie children
 *------------------------------------------------------------------------
 */
void reaper(int sig) {
	int	status;
	while (waitpid(-1, &status, WNOHANG) >= 0)
		/* empty */;
}

int errexit(const char *format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    exit(1);
}

int errno;
unsigned short portbase = 0;   /* port base, for non-root servers      */


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
void cmd_reg(int fd, const vector<string> &command);
void cmd_login(int fd, const vector<string> &command, int & userID, string & username);
void cmd_logout(int fd, int & userID, string & username);
void cmd_who(int fd, int & userID, string & username);
void cmd_exit(int fd);

int reg_check(const vector<string> &command);
int login_check(const vector<string> &command);
static int callback(void *data, int argc, char **argv, char **azColName);
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
    while (getline(istr, substr, ' ')) {
        if(substr.length() == 0)    continue;
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
    //cout << command_head << endl;
    /*** OP ***/
    if(command_head.compare("register") == 0){  cmd_reg(fd, command);       }
    if(command_head.compare("login") == 0){     cmd_login(fd, command, userID, username);     }
    if(command_head.compare("logout") == 0){    cmd_logout(fd, userID, username);    }
    if(command_head.compare("whoami") == 0){    cmd_who(fd, userID, username);       }
    if(command_head.compare("exit") == 0){      cmd_exit(fd);      }
    return 0;
}

char not_login[100] = {"Please login first.\n"};
char not_logout[100] = {"Please logout first.\n"};
char reg_suc[100] = {"Please login first.\n"};
char reg_fail[100] = {"Please login first.\n"};

void cmd_reg(int fd, const vector<string> &command){
    //printf("cmd_reg\n");
    int cmd_errno;
    string msg;
    if(command.size() != 4){
        msg = "Usage: register <username> <email> <password>\n";
        if (cmd_errno=write(fd, msg.c_str(), strlen(msg.c_str())) < 0){
            errexit("echo write: %s\n", strerror(cmd_errno));
        }
    }
    else{
        /********************
        *   register check  *
        ********************/
        int result = reg_check(command);
        if(result == 0){
            msg = "Register successfully.\n";
        }
        else if(result == 1){
            msg = "Username is already used.\n";
        }
        else{
            msg = "Database Connection Failed.\n";
        }
        
        if (cmd_errno=write(fd, msg.c_str(), strlen(msg.c_str())) < 0){
            errexit("echo write: %s\n", strerror(cmd_errno));
        }
    }      
    return;
}

void cmd_login(int fd, const vector<string> &command, int & userID, string & username){
    int cmd_errno;
    string msg;
    if(command.size() != 3){
        msg = "Usage: login <username> <password>\n";
        if (cmd_errno=write(fd, msg.c_str(), strlen(msg.c_str())) < 0){
            errexit("echo write: %s\n", strerror(cmd_errno));
        }
    }
    else{
        if(userID != -1){
            msg = "Please logout first.\n";
            if (cmd_errno=write(fd, msg.c_str(), strlen(msg.c_str())) < 0){
                errexit("echo write: %s\n", strerror(cmd_errno));
            }
        }
        else{
            /********************
            *   login check     *
            ********************/
            int result = login_check(command);
            if(result == 0){
                userID = 1;
                username = command.at(1);
                msg = "Welcome, " + username + ".\n";
            }
            else if(result == 1){
                msg = "Login Failed.\n";
            }
            else{
                msg = "Database Connection Failed.\n";
            }
            if (cmd_errno=write(fd, msg.c_str(), strlen(msg.c_str())) < 0){
                errexit("echo write: %s\n", strerror(cmd_errno));
            }
        }
    } 
    return;
}


void cmd_logout(int fd, int & userID, string & username){
    int cmd_errno;
    string msg;
    if(userID == -1){
        msg = "Please login first.\n";
        if (cmd_errno=write(fd, msg.c_str(), strlen(msg.c_str())) < 0){
            errexit("echo write: %s\n", strerror(cmd_errno));
        }
    }
    else{
        //login success
        userID = -1;
        msg = "Bye, " + username + ".\n";
        username = "";
        if (cmd_errno=write(fd, msg.c_str(), strlen(msg.c_str())) < 0){
            errexit("echo write: %s\n", strerror(cmd_errno));
        }
    }
    return;
}

void cmd_who(int fd, int & userID, string & username){
    int cmd_errno;
    string msg;
    if(userID == -1){
        msg = "Please login first.\n";
        if (cmd_errno=write(fd, msg.c_str(), strlen(msg.c_str())) < 0){
            errexit("echo write: %s\n", strerror(cmd_errno));
        }
    }
    else{
        msg = username + "\n";
        if (cmd_errno=write(fd, msg.c_str(), strlen(msg.c_str())) < 0){
            errexit("echo write: %s\n", strerror(cmd_errno));
        }
    }
    return;
}

void cmd_exit(int fd){
    close(fd);
    printf("[pid : %d] exit!\n", getpid());
    exit(0);
    // exit code
    return;
}

int reg_check(const vector<string> &command){
    sqlite3 *db;
    sqlite3_stmt *stmt = NULL;
    char *zErrMsg = 0;
    int rc;
    char *sql;
    const char* data = "Callback function called";
    string str_name = command[1];
    string str_email = command[2];
    string str_password = command[3];
    
    /*** Open database ***/
    rc = sqlite3_open("NP_PJ1.db", &db);
    if( rc ){
        fprintf(stderr, "[PID:%d] Can't open database: %s\n", getpid(), sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return -1;
    }else{
        fprintf(stdout, "[PID:%d] Opened database successfully\n", getpid());
    }
    
    /*** Check if the user is already existed ***/
    string str;
    str = "SELECT COUNT(*) FROM USERS WHERE Username = ?;";
    sqlite3_prepare_v2(db, str.c_str(), -1, &stmt, NULL);
    sqlite3_bind_text(stmt, 1, str_name.c_str(), str_name.length(), SQLITE_TRANSIENT);
    rc = sqlite3_step(stmt);
    
    if( rc != SQLITE_ROW ){
        fprintf(stderr, "[PID:%d] SQL error: %s\n", getpid(), sqlite3_errmsg(db));
        //sqlite3_free(zErrMsg);
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return -1;
    }else{
        fprintf(stderr, "[PID:%d] Search successfully\n", getpid());
    }
    int count = sqlite3_column_int (stmt, 0);
    str = "";
    sqlite3_reset(stmt);
    //cout << count << endl;
    
    if(count != 0){ // username is already exist
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return 1;
    }
    
    /*** Insert user's info to table ***/
    str = "INSERT INTO USERS (Username,Email,Password) VALUES (?,?,?);";
    sqlite3_prepare_v2(db, str.c_str(), -1, &stmt, NULL);
    sqlite3_bind_text(stmt, 1, command[1].c_str(), command[1].length(), SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, command[2].c_str(), command[2].length(), SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, command[3].c_str(), command[3].length(), SQLITE_TRANSIENT);
    rc = sqlite3_step(stmt);
    // cout << rc << endl;
    if( rc != SQLITE_OK && rc!= SQLITE_DONE ){
        fprintf(stderr, "[PID:%d] SQL error: %s\n", getpid(), sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return -1;
    }else{
        fprintf(stdout, "[PID:%d] Insertion successfully\n", getpid());
    }
    str = "";
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return 0;
}

int login_check(const vector<string> &command){
    sqlite3 *db;
    sqlite3_stmt *stmt = NULL;
    char *zErrMsg = 0;
    int rc;
    char *sql;
    const char* data = "Callback function called";
    
    /*** Open database ***/
    rc = sqlite3_open("NP_PJ1.db", &db);
    if( rc ){
        fprintf(stderr, "[PID:%d] Can't open database: %s\n", getpid(), sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return -1;
    }else{
        fprintf(stdout, "[PID:%d] Opened database successfully\n", getpid());
    }
    
    /*** Check if the user is already existed ***/
    string str;
    str = "SELECT COUNT(*) FROM USERS WHERE Username = ? AND Password = ?;";
    sqlite3_prepare_v2(db, str.c_str(), -1, &stmt, NULL);
    sqlite3_bind_text(stmt, 1, command[1].c_str(), command[1].length(), SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, command[2].c_str(), command[2].length(), SQLITE_TRANSIENT);
    rc = sqlite3_step(stmt);
    
    int count = sqlite3_column_int(stmt, 0);
    
    //cout << rc << endl;
    if( rc != SQLITE_ROW ){
        fprintf(stderr, "[PID:%d] SQL error: %s\n", getpid(), sqlite3_errmsg(db));
        //sqlite3_free(zErrMsg);
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return -1;
    }else{
        fprintf(stdout, "[PID:%d] Search successfully\n", getpid());
    }
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    
    if(count == 1){ return 0;   }
    else{           return 1;   }
    
    return -1;
}

static int callback(void *data, int argc, char **argv, char **azColName){
    int i;
    fprintf(stderr, "%s: ", (const char*)data);
    for(i=0; i<argc; i++){
        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }
    printf("\n");
    return 0;
}