/*
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
*/

#ifndef _ERR_H
#define _ERR_H
#include "err.h"
#endif

using namespace std;

void cmd_reg(int fd, const vector<string> &command);
void cmd_login(int fd, const vector<string> &command, int & userID, string & username);
void cmd_logout(int fd, int & userID, string & username);
void cmd_who(int fd, int & userID, string & username);
void cmd_exit(int fd);

int reg_check(const vector<string> &command);
int login_check(const vector<string> &command, int & userID);
static int callback(void *data, int argc, char **argv, char **azColName);


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
            int result = login_check(command, userID);
            if(result == 1){
                username = command.at(1);
                msg = "Welcome, " + username + ".\n";
            }
            else if(result == -1){
                msg = "Database Connection Failed.\n";
            }
            else{
                msg = "Login Failed.\n";
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
    rc = sqlite3_open("NP_PJ2.db", &db);
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

int login_check(const vector<string> &command, int & userID){
    sqlite3 *db;
    sqlite3_stmt *stmt = NULL;
    char *zErrMsg = 0;
    int rc;
    char *sql;
    const char* data = "Callback function called";
    
    /*** Open database ***/
    rc = sqlite3_open("NP_PJ2.db", &db);
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
    str = "SELECT * FROM USERS WHERE Username = ? AND Password = ?;";
    sqlite3_prepare_v2(db, str.c_str(), -1, &stmt, NULL);
    sqlite3_bind_text(stmt, 1, command[1].c_str(), command[1].length(), SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, command[2].c_str(), command[2].length(), SQLITE_TRANSIENT);
    
    int count = 0;
    while((rc = sqlite3_step(stmt)) == SQLITE_ROW){
        count++;
        userID = sqlite3_column_int(stmt, 0);
    }
    
    if( rc != SQLITE_OK && rc!= SQLITE_DONE ){
        fprintf(stderr, "[PID:%d] SQL error: %s\n", getpid(), sqlite3_errmsg(db));
        count = -1;
    }else{
        fprintf(stdout, "[PID:%d] SQL Search successfully\n", getpid());
    }
    
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    
    return count;
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