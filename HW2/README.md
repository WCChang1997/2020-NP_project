# HW2
## 部署步驟
- SQLite 安裝
    `$ sudo apt-get install sqlite3 libsqlite3-dev`
- 初始化資料庫（會生成 `NP_HW2.db`）
    `$ sqlite3 NP_HW2.db ".read PJ2_DB.sql"`
- 編譯 server
    `$ g++ PJ2_server.cpp -l sqlite3 -o PJ2_server`
- 執行 server
    `$ ./PJ2_server <server_port>`
- client 端連線（可同時建立多條連線）
    `$ telnet <server_IP> <server_port>`

## 指令
以下指令中的 `%` 為 BBS 系統之前綴，下指令時無須輸入。<br>
當使用者輸入的指令有誤時，系統將會顯示該指令之正確用法。
- 板塊相關
    - **建立板塊**
        `% create-board <board-name>`
        - <font color=green>板塊建立成功</font>：`Create board successfully.`
        - <font color=red>未登入</font>：`Please login first.`
        - <font color=red>板塊已存在</font>：`Board already exist.`
    - **列出板塊清單**（可根據關鍵字 `##<key>` 搜尋）
        `% list-board ##<key>`
        - <font color=green>顯示板塊搜尋結果</font>
            ```bash
            Index       Name        Moderator
            <Index1>    <Name1>     <Moderator1>
            <Index2>    <Name2>     <Moderator2>
            ...
            ```
    - **列出某一板塊中的貼文清單**（可根據關鍵字 `##<key>` 搜尋）
        `% list-post <board-name> ##<key>`
        - <font color=green>顯示貼文搜尋結果</font>
            ```bash
            ID          Title       Author      Date
            <ID1>       <Title1>    <Author1>   <Date1>
            <ID2>       <Title2>    <Author2>   <Date2>
            ...
            ```
        - <font color=red>板塊不存在</font>：`Board does not exist.`
- 貼文相關
    - **建立貼文**
        `% create-post <board-name> --title <title> --content <content>`
        - <font color=green>貼文建立成功</font>：`Create post successfully.`
        - <font color=red>未登入</font>：`Please login first.`
        - <font color=red>板塊不存在</font>：`Board does not exist.`
    - **閱讀貼文**
        `% read <post-id>`
        - <font color=green>顯示貼文內容</font>
            ```bash
            Author  : <Author1>
            Title   : <Title1>
            Date    : <Date1>
            --
            <content>
            --
            <User1> : <Comment1>
            ```
        - <font color=red>貼文不存在</font>：`Post does not exist.`
    - **刪除貼文**
        `% delete-post <post-id>`
        - <font color=green>貼文刪除成功</font>：`Delete successfully.`
        - <font color=red>未登入</font>：`Please login first.`
        - <font color=red>貼文不存在</font>：`Post does not exist.`
        - <font color=red>非發貼者</font>：`Not the post owner.`
    - **更新貼文**
        `% update-post <post-id> --title/content <new>`
        - <font color=green>貼文更新成功</font>：`Update successfully.`
        - <font color=red>未登入</font>：`Please login first.`
        - <font color=red>貼文不存在</font>：`Post does not exist.`
        - <font color=red>非發貼者</font>：`Not the post owner.`
    - **在某貼文中留言**
        `% comment <post-id> <comment>`
        - <font color=green>留言成功</font>：`Comment successfully.`
        - <font color=red>未登入</font>：`Please login first.`
        - <font color=red>貼文不存在</font>：`Post does not exist.`
