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

#include "bbs2_sub.h"

using namespace std;

/*** Decleration ***/

void cmd_create_board(int fd, const vector<string> &command, int & userID, string & username);
void cmd_list_board(int fd, const vector<string> &command, int & userID, string & username);

void cmd_create_post(int fd, const vector<string> &command, int & userID, string & username);
void cmd_list_post(int fd, const vector<string> &command, int & userID, string & username);
void cmd_read_post(int fd, const vector<string> &command, int & userID, string & username);
void cmd_delete_post(int fd, const vector<string> &command, int & userID, string & username);
void cmd_update_post(int fd, const vector<string> &command, int & userID, string & username);
void cmd_comment_post(int fd, const vector<string> &command, int & userID, string & username);

/*** Main Function ***/
/********************************************
*   1. Check "Command Format"               *
*   2. Check if "Login"                     *
*   3. Check if the board or post "Exists"  *
*   4. Do something                         *
********************************************/
void cmd_create_board(int fd, const vector<string> &command, int & userID, string & username){
    string msg;
    /*** 1. ***/
    if(command.size() != 2){
        msg = "Usage: create-board <name>\n";
        send_msg(fd, msg);
        return;
    }
    
    /*** 2. ***/
    if(userID == -1){
        msg = "Please login first.\n";
        send_msg(fd, msg);
        return;
    }
    
    /*** 3. ***/
    string board_name = command.at(1);
    int board_id = -1;
    int result = board_check(fd, board_name, board_id);
    if(result == 0){
        cout << "Creating Board...\n";
    }
    else if(result == 1){
        msg = "Board is already exist.\n";
    }
    else{ 
        msg = "Database Connection Failed.\n"; 
    }
    if(result != 0){
        send_msg(fd, msg);
        return;
    }
    
    /*** 4. ***/
    result = board_create(fd, board_name, userID, username);
    if(result == 0){    msg = "Create board successfully.\n";   }
    else{               msg = "Database Connection Failed.\n";  }
    send_msg(fd, msg);
    return;
}

void cmd_list_board(int fd, const vector<string> &command, int & userID, string & username){
    string msg;
    /*** 1. ***/
    int cmd_num = command.size();
    if(cmd_num == 1){
        msg = "";
    }
    else if(cmd_num == 2){
        string key_check = command.at(1).substr(0, 2);
        if(key_check.compare("##") != 0){
            msg = "Usage: list-board ##<key>\n";
            send_msg(fd, msg);
            return;
        }
    }
    else{
        msg = "Usage: list-board ##<key>\n";
        send_msg(fd, msg);
        return;
    }
    
    /*** 2 ***/
    // No login check
    
    /*** 3 ***/
    // No board exist check
    
    /*** 4 ***/
    string key;
    if(cmd_num == 1){
        key = "";
    }
    else if(cmd_num == 2){
        key = command.at(1).substr(2);
    }
    else{
        cout << "Something error\n";
        return;
    }
    int result = board_list(fd, key);
    if(result == 0){
        cout << "List board successfully.\n";
    }
    else{
        msg = "Database Connection Failed.\n";
        send_msg(fd, msg);
    }
    return;
}

void cmd_create_post(int fd, const vector<string> &command, int & userID, string & username){
    string msg;
    /*** 1. ***/
    if(command.size() != 6){
        msg = "Usage: create-post <board-name> --title <title> --content <content>\n";
        send_msg(fd, msg);
        return;
    }
    if(command.at(2).compare("--title") != 0 || command.at(4).compare("--content")){
        msg = "Usage: create-post <board-name> --title <title> --content <content>\n";
        send_msg(fd, msg);
        return;
    }
    
    /*** 2. ***/
    if(userID == -1){
        msg = "Please login first.\n";
        send_msg(fd, msg);
        return;
    }
    
    /*** 3. ***/
    string board_name = command.at(1);
    int board_id = -1;
    int result = board_check(fd, board_name, board_id);
    if(result == 0){        msg = "Board is not exist.\n";      }
    else if(result == 1){   cout << "Creating Post...\n";           }
    else{                   msg = "Database Connection Failed.\n";  }
    if(result != 1){
        send_msg(fd, msg);
        return;
    }
    
    /*** 4 ***/
    string title = command.at(3);
    string content = command.at(5);
    result = post_create(fd, board_id, userID, username, title, content);
    if(result == 0){    msg = "Create post successfully.\n";   }
    else{               msg = "Database Connection Failed.\n";  }
    send_msg(fd, msg);
    return;
}

void cmd_list_post(int fd, const vector<string> &command, int & userID, string & username){
    string msg;
    /*** 1. ***/
    int cmd_num = command.size();
    if(cmd_num == 2){
        msg = "";
    }
    else if(cmd_num == 3){
        string key_check = command.at(2).substr(0, 2);
        if(key_check.compare("##") != 0){
            msg = "Usage: list-post <board-name> ##<key>\n";
            send_msg(fd, msg);
            return;
        }
    }
    else{
        msg = "Usage: list-post <board-name> ##<key>\n";
        send_msg(fd, msg);
        return;
    }
    
    /*** 2 ***/
    // No login check
    
    /*** 3 ***/
    string board_name = command.at(1);
    int board_id = -1;
    int result = board_check(fd, board_name, board_id);
    if(result == 0){
        msg = "Board is not exist.\n";
    }
    else if(result == 1){
        cout << "Board found.\n";
    }
    else{ 
        msg = "Database Connection Failed.\n"; 
    }
    if(result != 1){
        send_msg(fd, msg);
        return;
    }
    
    /*** 4 ***/
    string key;
    if(cmd_num == 2){
        key = "";
    }
    else if(cmd_num == 3){
        key = command.at(2).substr(2);
    }
    else{
        cout << "Something error\n";
        return;
    }
    result = post_list(fd, board_id, key);
    if(result == 0){
        cout << "List post successfully.\n";
    }
    else{
        msg = "Database Connection Failed.\n";
        send_msg(fd, msg);
    }
    return;
}

void cmd_read_post(int fd, const vector<string> &command, int & userID, string & username){
    string msg;
    /*** 1. ***/
    if(command.size() != 2){
        msg = "Usage: read <post-id>\n";
        send_msg(fd, msg);
        return;
    }
    
    /*** 2. ***/
    // No login check
    
    /*** 3. ***/
    string post_id = command.at(1);
    int post_id_int = -1;
    int post_owner_id = -1;
    int result = post_check(fd, post_id, post_id_int, post_owner_id);
    if(result == 0){        msg = "Post is not exist.\n";           }
    else if(result == 1){   cout << "Post found.\n";                }
    else{                   msg = "Database Connection Failed.\n";  }
    if(result != 1){
        send_msg(fd, msg);
        return;
    }
    
    /*** 4. ***/
    result = post_read(fd, post_id, post_id_int);
    if(result == 0){
        cout << "Read post successfully.\n";
    }
    else{
        cout << "Read post Failed.\n";
    }
    return;
}

void cmd_delete_post(int fd, const vector<string> &command, int & userID, string & username){
    string msg;
    /*** 1. ***/
    if(command.size() != 2){
        msg = "Usage: delete-post <post-id>\n";
        send_msg(fd, msg);
        return;
    }
    
    /*** 2. ***/
    if(userID == -1){
        msg = "Please login first.\n";
        send_msg(fd, msg);
        return;
    }
    
    /*** 3. ***/
    // check if post exists
    string post_id = command.at(1);
    int post_id_int = -1;
    int post_owner_id = -1;
    int result = post_check(fd, post_id, post_id_int, post_owner_id);
    if(result == 0){        msg = "Post is not exist.\n";           }
    else if(result == 1){   cout << "Post found.\n";                }
    else{                   msg = "Database Connection Failed.\n";  }
    if(result != 1){
        send_msg(fd, msg);
        return;
    }
    // check post owner
    if(post_owner_id >= 0 && post_owner_id == userID){
        cout << "Post owner checked.\n";
    }
    else{
        msg = "Not the post owner.\n";
        send_msg(fd, msg);
        return;
    }
    
    /*** 4. ***/
    // database
    result = post_delete(fd, post_id, post_id_int);
    if(result == 0){
        msg = "Delete successfully.\n";
        send_msg(fd, msg);
    }
    else{
        cout << "Delete post Failed.\n";
    }
    return;
}

void cmd_update_post(int fd, const vector<string> &command, int & userID, string & username){
    string msg;
    /*** 1. ***/
    string title = "";
    string content = "";
    bool format_error = 0;
    if(command.size() == 4){
        if(command.at(2).compare("--title") == 0){
            title = command.at(3);
        }
        else{
            if(command.at(2).compare("--content") == 0){
                content = command.at(3);
            }
            else{
                format_error = 1;
            }
        }
    }
    else if(command.size() == 6){
        if(command.at(2).compare("--title") == 0){
            if(command.at(4).compare("--content") == 0){
                title = command.at(3);
                content = command.at(5);
            }
            else{
                format_error = 1;
            }
        }
        else{
            if(command.at(2).compare("--content") == 0){
                if(command.at(4).compare("--title") == 0){
                    content = command.at(3);
                    title= command.at(5);
                }
                else{
                    format_error = 1;
                }
            }
            else{
                format_error = 1;
            }
        }
    }
    else{
        format_error = 1;
    }
    
    if(format_error){
        msg = "Usage: update-post <post-id> --title/content <new>\n";
        send_msg(fd, msg);
        return;
    }
    
    /*** 2. ***/
    if(userID == -1){
        msg = "Please login first.\n";
        send_msg(fd, msg);
        return;
    }
    
    /*** 3. ***/
    string post_id = command.at(1);
    int post_id_int = -1;
    int post_owner_id = -1;
    int result = post_check(fd, post_id, post_id_int, post_owner_id);
    if(result == 0){        msg = "Post is not exist.\n";           }
    else if(result == 1){   cout << "Post found.\n";                }
    else{                   msg = "Database Connection Failed.\n";  }
    if(result != 1){
        send_msg(fd, msg);
        return;
    }
    // check post owner
    cout << "user = " << userID << " / post owner = " << post_owner_id << endl;
    if((post_owner_id >= 0) && (post_owner_id == userID)){
        cout << "Post owner checked.\n";
    }
    else{
        msg = "Not the post owner.\n";
        send_msg(fd, msg);
        return;
    }
    
    /*** 4 ***/
    result = 0;
    // update title
    if(title.size() > 0){
        int tmpt = post_update_title(fd, post_id_int, title);
        if(tmpt == -1){
            cout << "Update Title Failed.\n";
        }
        else{
            result += 0;
        }
    }
    // update content
    if(content.size() > 0){
        int tmpt = post_update_content(fd, post_id, content);
        if(tmpt == -1){
            cout << "Update Content Failed.\n";
        }
        else{
            result += 0;
        }
    }
    
    if(result == 0){
        msg = "Update successfully.\n";
        send_msg(fd, msg);
    }
    else{
        cout << "Update Failed.\n";
    }
    return;
}

void cmd_comment_post(int fd, const vector<string> &command, int & userID, string & username){
    string msg;
    /*** 1. ***/
    if(command.size() != 3){
        msg = "Usage: comment <post-id> <comment>\n";
        send_msg(fd, msg);
        return;
    }
    
    /*** 2. ***/
    if(userID == -1){
        msg = "Please login first.\n";
        send_msg(fd, msg);
        return;
    }
    
    /*** 3. ***/
    string post_id = command.at(1);
    int post_id_int = -1;
    int post_owner_id = -1;
    int result = post_check(fd, post_id, post_id_int, post_owner_id);
    if(result == 0){        msg = "Post is not exist.\n";           }
    else if(result == 1){   cout << "Post found.\n";                }
    else{                   msg = "Database Connection Failed.\n";  }
    if(result != 1){
        send_msg(fd, msg);
        return;
    }
    
    /*** 4. ***/
    string comment = command.at(2);
    result = post_comment(fd, post_id, username, comment);
    if(result == 0){    msg = "Comment successfully.\n";    }
    else{               msg = "Comment Failed.\n";          }
    send_msg(fd, msg);
    return;
}