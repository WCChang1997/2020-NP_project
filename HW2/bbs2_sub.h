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

/*** Decleration ***/
int board_check(int & fd, string & board_name, int & board_id);
int board_create(int & fd, string & board_name, int & userID, string & username);
int board_list(int & fd, string & key);

int post_check(int & fd, string & post_id, int & post_id_int, int & post_owner_id);
int post_create(int & fd, int & board_id, int & userID, string & username, string & title, string & content);
int post_list(int & fd, int & board_id, string & key);
int post_read(int & fd, string & post_id, int & post_id_int);
int post_delete(int & fd, string & post_id, int & post_id_int);
int post_update_title(int & fd, int & post_id_int, string & title);
int post_update_content(int & fd, string & post_id, string & content);
int post_comment(int & fd, string & post_id, string & username, string & comment);

/*** Sub Function ***/
/****************************
*   1. Open database        *
*   2. Prepare & Bind       *
*   3. Step & Get column    *
*   4. Return & Delete      *
****************************/
// 0 BID | 1 Board_Name | 2 Moderator_ID | 3 Moderator
int board_check(int & fd, string & board_name, int & board_id){
    /*** 1. ***/
    sqlite3 *db;
    sqlite3_stmt *stmt = NULL;
    int rc;
    rc = sqlite3_open("NP_PJ2.db", &db);
    if( rc ){
        fprintf(stderr, "[PID:%d] Can't open database: %s\n", getpid(), sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return -1;
    }else{
        fprintf(stdout, "[PID:%d] Opened database successfully\n", getpid());
    }
    
    /*** 2. ***/
    string str;
    str = "SELECT * FROM BOARD WHERE Board_Name = ?;";
    sqlite3_prepare_v2(db, str.c_str(), -1, &stmt, NULL);
    sqlite3_bind_text(stmt, 1, board_name.c_str(), board_name.length(), SQLITE_TRANSIENT);
    
    /*** 3. ***/
    int count = 0;
    while((rc = sqlite3_step(stmt)) == SQLITE_ROW){
        count++;
        board_id = sqlite3_column_int(stmt, 0);
    }
    if( rc != SQLITE_OK && rc!= SQLITE_DONE ){
        fprintf(stderr, "[PID:%d] SQL Check Board Failed: %s\n", getpid(), sqlite3_errmsg(db));
        count = -1;
    }else{
        fprintf(stdout, "[PID:%d] SQL Check Board Successfully.\n", getpid());
    }
    
    /*** 4. ***/
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return count;
}
int board_create(int & fd, string & board_name, int & userID, string & username){
    /*** 1. ***/
    sqlite3 *db;
    sqlite3_stmt *stmt = NULL;
    int rc;
    rc = sqlite3_open("NP_PJ2.db", &db);
    if( rc ){
        fprintf(stderr, "[PID:%d] Can't open database: %s\n", getpid(), sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return -1;
    }else{
        fprintf(stdout, "[PID:%d] Opened database successfully\n", getpid());
    }
    
    /*** 2. ***/
    string str;
    str = "INSERT INTO BOARD (Board_Name, Moderator_ID, Moderator) VALUES (?, ?, ?);";
    sqlite3_prepare_v2(db, str.c_str(), -1, &stmt, NULL);
    sqlite3_bind_text(stmt, 1, board_name.c_str(), board_name.length(), SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, userID);
    sqlite3_bind_text(stmt, 3, username.c_str(), username.length(), SQLITE_TRANSIENT);
    rc = sqlite3_step(stmt);
    
    /*** 3. ***/
    if( rc != SQLITE_OK && rc!= SQLITE_DONE ){
        fprintf(stderr, "[PID:%d] SQL Insert Board Failed: %s\n", getpid(), sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return -1;
    }else{
        fprintf(stdout, "[PID:%d] SQL Insert Board Successfully\n", getpid());
    }
    
    /*** 4. ***/
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return 0;
}
int board_list(int & fd, string & key){
    /*** 1. ***/
    sqlite3 *db;
    sqlite3_stmt *stmt = NULL;
    int rc;
    rc = sqlite3_open("NP_PJ2.db", &db);
    if( rc ){
        fprintf(stderr, "[PID:%d] Can't open database: %s\n", getpid(), sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return -1;
    }else{
        fprintf(stdout, "[PID:%d] Opened database successfully\n", getpid());
    }
    
    /*** 2. ***/
    string str;
    string key_like = "%" + key + "%";
    str = "SELECT BID, Board_Name, Moderator FROM BOARD";
    if(key.size() > 0){
        str = str + " WHERE Board_Name LIKE ?";
    }
    str = str + ";";
    
    sqlite3_prepare_v2(db, str.c_str(), -1, &stmt, NULL);
    if(key.size() > 0){
        sqlite3_bind_text(stmt, 1, key_like.c_str(), key_like.length(), SQLITE_TRANSIENT);
    }
    
    /*** 3. ***/
    ostringstream oss_header;
    oss_header << left << setw(10) << "Index" << " ";
    oss_header << left << setw(20) << "Name" << "\t";
    oss_header << left << setw(10) << "Moderator" << "\n";
    string line = oss_header.str();
    send_msg(fd, line);
    line.clear();
    
    while((rc = sqlite3_step(stmt)) == SQLITE_ROW){
        int BID_int = sqlite3_column_int(stmt, 0);
        string BID = to_string(BID_int);
        string Board_Name((const char *) sqlite3_column_text(stmt, 1));
        string Moderator((const char *) sqlite3_column_text(stmt, 2));
        
        ostringstream oss_row;
        oss_row << left << setw(10) << BID << " ";
        oss_row << left << setw(20) << Board_Name << "\t";
        oss_row << left << setw(10) << Moderator << "\n";
        line = oss_row.str();
        send_msg(fd, line);
        line.clear();
    }
    if( rc != SQLITE_OK && rc!= SQLITE_DONE ){
        fprintf(stderr, "[PID:%d] SQL List Board Failed: %s\n", getpid(), sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return -1;
    }else{
        fprintf(stdout, "[PID:%d] SQL List Board successfully.\n", getpid());
    }
    
    /*** 4. ***/
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return 0; 
}

// 0 PID | 1 Board_ID | 2 Title | 3 Author_ID | 4 Author | 5 Date
int post_check(int & fd, string & post_id, int & post_id_int, int & post_owner_id){
    /*** 0 ***/
    // check if int
    istringstream ss(post_id);
    int result;
    ss >> result;
    if (!ss.fail() && ss.eof()) {
        post_id_int = result;
        //printf("is int\n");
    }
    else {
        post_id_int = -1;
        return 0;
    }
    
    /*** 1. ***/
    sqlite3 *db;
    sqlite3_stmt *stmt = NULL;
    int rc;
    rc = sqlite3_open("NP_PJ2.db", &db);
    if( rc ){
        fprintf(stderr, "[PID:%d] Can't open database: %s\n", getpid(), sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return -1;
    }else{
        fprintf(stdout, "[PID:%d] Opened database successfully\n", getpid());
    }
    
    /*** 2. ***/
    string str;
    str = "SELECT * FROM POST WHERE PID = ?;";
    sqlite3_prepare_v2(db, str.c_str(), -1, &stmt, NULL);
    sqlite3_bind_int(stmt, 1, post_id_int);
    
    /*** 3. ***/
    int count = 0;
    while((rc = sqlite3_step(stmt)) == SQLITE_ROW){
        count++;
        post_owner_id = sqlite3_column_int(stmt, 3);
    }
    if( rc != SQLITE_OK && rc!= SQLITE_DONE ){
        fprintf(stderr, "[PID:%d] SQL Check Post Failed: %s\n", getpid(), sqlite3_errmsg(db));
        count = -1;
    }else{
        fprintf(stdout, "[PID:%d] SQL Check Post Successfully.\n", getpid());
    }
    
    /*** 4. ***/
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return count;
}
int post_create(int & fd, int & board_id, int & userID, string & username, string & title, string & content){
    /*** 1. ***/
    sqlite3 *db;
    sqlite3_stmt *stmt = NULL;
    int rc;
    rc = sqlite3_open("NP_PJ2.db", &db);
    if( rc ){
        fprintf(stderr, "[PID:%d] Can't open database: %s\n", getpid(), sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return -1;
    }else{
        fprintf(stdout, "[PID:%d] Opened database successfully\n", getpid());
    }
    
    /*** 2. ***/
    string str;
    str = "INSERT INTO POST (Board_ID, Title, Author_ID, Author) VALUES (?, ?, ?, ?);";
    sqlite3_prepare_v2(db, str.c_str(), -1, &stmt, NULL);
    sqlite3_bind_int(stmt, 1, board_id);
    sqlite3_bind_text(stmt, 2, title.c_str(), title.length(), SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 3, userID);
    sqlite3_bind_text(stmt, 4, username.c_str(), username.length(), SQLITE_TRANSIENT);
    
    rc = sqlite3_step(stmt);
    
    /*** 3. ***/
    // insert post
    if( rc != SQLITE_OK && rc!= SQLITE_DONE ){
        fprintf(stderr, "[PID:%d] SQL Insert Post Failed: %s\n", getpid(), sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return -1;
    }else{
        fprintf(stdout, "[PID:%d] SQL Insert Post Successfully.\n", getpid());
    }
    
    // get insert id
    int post_id_int = sqlite3_last_insert_rowid(db);
    string post_id = to_string(post_id_int);
    /*** 4. ***/
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    
    /*** 5. ***/
    string fname = "./post/" + post_id;
    ofstream fout_c;
    fout_c.open(fname.c_str(), ifstream::out);
    if(!fout_c){
        printf("%s open error.\n", fname.c_str());
        return -1;
    }
    else{
        string str = content;
        string line = "";
        string delim = "<br>";
        size_t pos = 0;
        while((pos = str.find(delim)) != string::npos){
            line = str.substr(0, pos);
            fout_c << line << "\n";
            str.erase(0, line.size() + delim.size());
            line.clear();
        }
        if(str.size() > 0){
            fout_c << str << "\n";
        }
    }
    fout_c.close();
    
    return 0; 
}
int post_list(int & fd, int & board_id, string & key){
    /*** 1. ***/
    sqlite3 *db;
    sqlite3_stmt *stmt = NULL;
    int rc;
    rc = sqlite3_open("NP_PJ2.db", &db);
    if( rc ){
        fprintf(stderr, "[PID:%d] Can't open database: %s\n", getpid(), sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return -1;
    }else{
        fprintf(stdout, "[PID:%d] Opened database successfully\n", getpid());
    }
    
    /*** 2. ***/
    string str;
    string key_like = "%" + key + "%";
    str = "SELECT PID, Title, Author, strftime('%m/%d', Date) as 'MM/DD' FROM POST WHERE Board_ID = ?";
    if(key.size() > 0){
        str = str + " AND Title LIKE ?";
    }
    str = str + ";";
    
    sqlite3_prepare_v2(db, str.c_str(), -1, &stmt, NULL);
    sqlite3_bind_int(stmt, 1, board_id);
    if(key.size() > 0){
        sqlite3_bind_text(stmt, 2, key_like.c_str(), key_like.length(), SQLITE_TRANSIENT);
    }
    
    /*** 3. ***/
    ostringstream oss_header;
    oss_header << left << setw(10) << "ID" << " ";
    oss_header << left << setw(20) << "Title" << "\t";
    oss_header << left << setw(10) << "Author" << "\t";
    oss_header << left << setw(10) << "Date" << "\n";
    string line = oss_header.str();
    send_msg(fd, line);
    line.clear();
    
    while((rc = sqlite3_step(stmt)) == SQLITE_ROW){
        int PID_int = sqlite3_column_int(stmt, 0);
        string PID = to_string(PID_int);
        string Title((const char *) sqlite3_column_text(stmt, 1));
        string Author((const char *) sqlite3_column_text(stmt, 2));
        string Date((const char *) sqlite3_column_text(stmt, 3));
        
        ostringstream oss_row;
        oss_row << left << setw(10) << PID << " ";
        oss_row << left << setw(20) << Title << "\t";
        oss_row << left << setw(10) << Author << "\t";
        oss_row << left << setw(10) << Date << "\n";
        line = oss_row.str();
        send_msg(fd, line);
        line.clear();
    }
    if( rc != SQLITE_OK && rc!= SQLITE_DONE ){
        fprintf(stderr, "[PID:%d] SQL List Post Failed: %s\n", getpid(), sqlite3_errmsg(db));
    }else{
        fprintf(stdout, "[PID:%d] SQL List Post successfully.\n", getpid());
    }
    
    /*** 4. ***/
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return 0; 
}
int post_read(int & fd, string & post_id, int & post_id_int){
    string P_author, P_title, P_date;
    /*** 1. ***/
    sqlite3 *db;
    sqlite3_stmt *stmt = NULL;
    int rc;
    rc = sqlite3_open("NP_PJ2.db", &db);
    if( rc ){
        fprintf(stderr, "[PID:%d] Can't open database: %s\n", getpid(), sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return -1;
    }else{
        fprintf(stdout, "[PID:%d] Opened database successfully\n", getpid());
    }
    
    /*** 2. ***/
    string str;
    str = "SELECT * FROM POST WHERE PID = ?;";
    sqlite3_prepare_v2(db, str.c_str(), -1, &stmt, NULL);
    sqlite3_bind_int(stmt, 1, post_id_int);
    
    /*** 3. ***/
    int count = 0;
    while((rc = sqlite3_step(stmt)) == SQLITE_ROW){
        count++;
        P_author.clear();
        P_title.clear();
        P_date.clear();
        P_author.assign((const char *) sqlite3_column_text(stmt, 4));
        P_title.assign((const char *) sqlite3_column_text(stmt, 2));
        P_date.assign((const char *) sqlite3_column_text(stmt, 5));
    }
    if( rc != SQLITE_OK && rc!= SQLITE_DONE ){
        fprintf(stderr, "[PID:%d] SQL Read Post Failed: %s\n", getpid(), sqlite3_errmsg(db));
        count = -1;
    }else{
        fprintf(stdout, "[PID:%d] SQL Read Post successfully.\n", getpid());
    }
    
    /*** 4. ***/
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    if(count != 1){
        fprintf(stdout, "[PID:%d] SQL Post Number Error.\n", getpid());
        return -1;
    }
    
    /*** 5. ***/
    string line;
// info
    line = "Author\t:" + P_author + "\n";
    send_msg(fd, line);
    line.clear();
    
    line = "Title\t:" + P_title + "\n";
    send_msg(fd, line);
    line.clear();
    
    line = "Date\t:" + P_date + "\n";
    send_msg(fd, line);
    line.clear();
    
// post
    line = "--\n";
    send_msg(fd, line);
    line.clear();
    
    string fname = "./post/" + post_id;
    // check if exist
    struct stat buf;
    if(stat(fname.c_str(), &buf) == -1){
        printf("%s does not exist.\n", fname.c_str());
        return -1;
    }
    
    ifstream fin_p;
    fin_p.open(fname.c_str(), ifstream::in);
    if(!fin_p){
        printf("%s open error.\n", fname.c_str());
    }
    else{
        while(getline(fin_p, line)){
            line = line + "\n";
            send_msg(fd, line);
            cout << line;
            line.clear();
        }
        cout << endl;
    }
    fin_p.close();
    fname.clear();
    
// comment
    line = "--\n";
    send_msg(fd, line);
    line.clear();
    
    fname = "./comment/" + post_id;
    // check if exist
    if(stat(fname.c_str(), &buf) == -1){
        printf("%s does not exist.\n", fname.c_str());
        return -1;
    }
    
    ifstream fin_c;
    fin_c.open(fname.c_str(), ifstream::in);
    if(!fin_c){
        printf("%s open error.\n", fname.c_str());
    }
    else{
        while(getline(fin_c, line)){
            line = line + "\n";
            send_msg(fd, line);
            cout << line;
            line.clear();
        }
        cout << endl;
    }
    fin_c.close();
    fname.clear();
    
    return 0;
}
int post_delete(int & fd, string & post_id, int & post_id_int){
    /*** 1. ***/
    sqlite3 *db;
    sqlite3_stmt *stmt = NULL;
    int rc;
    rc = sqlite3_open("NP_PJ2.db", &db);
    if( rc ){
        fprintf(stderr, "[PID:%d] Can't open database: %s\n", getpid(), sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return -1;
    }else{
        fprintf(stdout, "[PID:%d] Opened database successfully\n", getpid());
    }
    
    /*** 2. ***/
    string str;
    str = "DELETE FROM POST WHERE PID = ?;";
    sqlite3_prepare_v2(db, str.c_str(), -1, &stmt, NULL);
    sqlite3_bind_int(stmt, 1, post_id_int);
    rc = sqlite3_step(stmt);
    
    /*** 3. ***/
    if( rc != SQLITE_OK && rc!= SQLITE_DONE ){
        fprintf(stderr, "[PID:%d] SQL error: %s\n", getpid(), sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return -1;
    }else{
        fprintf(stderr, "[PID:%d] Delete post successfully\n", getpid());
    }
    
    /*** 4. ***/
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    
    /*** 5. ***/
    string fname;
    // post
    fname = "./post/" + post_id;
    if(remove(fname.c_str()) != 0){
        perror("Error deleting file");
    }else{
        printf("Post deleted.\n");
    }
    
    // comment
    fname = "./comment/" + post_id;
    if(remove(fname.c_str()) != 0){
        perror("Error deleting file");
    }else{
        printf("Comment deleted.\n");
    }
    
    return 0; 
}
int post_update_title(int & fd, int & post_id_int, string & title){
    /*** 1. ***/
    sqlite3 *db;
    sqlite3_stmt *stmt = NULL;
    int rc;
    rc = sqlite3_open("NP_PJ2.db", &db);
    if( rc ){
        fprintf(stderr, "[PID:%d] Can't open database: %s\n", getpid(), sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return -1;
    }else{
        fprintf(stdout, "[PID:%d] Opened database successfully\n", getpid());
    }
    
    /*** 2. ***/
    string str;
    str = "UPDATE POST SET Title = ? WHERE PID = ?;";
    sqlite3_prepare_v2(db, str.c_str(), -1, &stmt, NULL);
    sqlite3_bind_text(stmt, 1, title.c_str(), title.length(), SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, post_id_int);
    rc = sqlite3_step(stmt);
    
    /*** 3. ***/
    if( rc != SQLITE_OK && rc!= SQLITE_DONE ){
        fprintf(stderr, "[PID:%d] SQL error: %s\n", getpid(), sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return -1;
    }else{
        fprintf(stderr, "[PID:%d] Update post successfully\n", getpid());
    }
    
    /*** 4. ***/
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return 0; 
}
int post_update_content(int & fd, string & post_id, string & content){
    string fname = "./post/" + post_id;
    ofstream fout_c;
    fout_c.open(fname.c_str(), ifstream::out);
    if(!fout_c){
        printf("%s open error.\n", fname.c_str());
        return -1;
    }
    else{
        string str = content;
        string line = "";
        string delim = "<br>";
        size_t pos = 0;
        while((pos = str.find(delim)) != string::npos){
            line = str.substr(0, pos);
            fout_c << line << "\n";
            str.erase(0, line.size() + delim.length());
            line.clear();
        }
        if(str.size() > 0){
            fout_c << str << "\n";
        }
    }
    fout_c.close();
    return 0;
}
int post_comment(int & fd, string & post_id, string & username, string & comment){
    string fname = "./comment/" + post_id;
    ofstream fout_c;
    fout_c.open(fname.c_str(), ifstream::out | ifstream::app);
    if(!fout_c){
        printf("%s open error.\n", fname.c_str());
        return -1;
    }
    else{
        string line = username + ": " + comment + "\n";
        fout_c << line;
    }
    fout_c.close();
    return 0;
}