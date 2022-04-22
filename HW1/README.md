# HW1
## 部署步驟
- SQLite 安裝
    `$ sudo apt-get install sqlite3 libsqlite3-dev`
- 初始化資料庫（會生成 `NP_HW1.db`）
    `$ sqlite3 NP_HW1.db ".read PJ1_DB.sql"`
- 編譯 server
    `$ g++ PJ1_server.cpp -l sqlite3 -std=c++11 -o PJ1_server`
- 執行 server
    `$ ./PJ1_server <server_port>`
- client 端連線（可同時建立多條連線）
    `$ telnet <server_IP> <server_port>`

## 指令
以下指令中的 `%` 為 BBS 系統之前綴，下指令時無須輸入。<br>
當使用者輸入的指令有誤時，系統將會顯示該指令之正確用法。
- 使用者相關
    - **註冊**
        `% register <username> <email> <password>`
        - <font color=green>註冊成功</font>：`Register successfully.`
        - <font color=red>使用者已存在</font>：`Username is already used.`
    - **登入**
        `% login <username> <password>`
        - <font color=green>登入成功</font>：`Welcome, <username>.`
        - <font color=red>已登入</font>：`Please logout first.`
        - <font color=red>帳號或密碼錯誤</font>：`Login failed.`
    - **登出**
        `% logout`
        - <font color=green>登出成功</font>：`Bye, <username>.`
        - <font color=red>尚未登入</font>：`Please login first.`
    - **顯示使用者名稱**
        `% whoami`
        - <font color=green>已登入</font>：`<username>`
        - <font color=red>尚未登入</font>：`Please login first.`
    - **結束連線**
        `% exit`
        - 關閉 telnet 連線
        
