# 網路程式設計概論 Project：BBS System
- [Course Information](https://timetable.nycu.edu.tw/?r=main/crsoutline&Acy=108&Sem=2&CrsNo=1266&lang=zh-tw)<br>
- [GitHub Repository](https://github.com/WCChang1997/NP_project.git)

## Project 摘要
- Project 分成 4 次作業，將分別**新增**不同的功能，意即每次作業都必須包含前一次作業能完成的功能。
- HW1 及 HW2 主要是設計 Bulletin Board System (BBS) 的 server 端程式，讓使用者能夠透過 telnet 連上該 server，並且進行討論版的基本操作。
    - [HW1](/HW1) 主要是能讓使用者註冊、登入登出
    - [HW2](/HW2) 要能建立、刪除、更改板塊或貼文，以及對貼文留言
- **（尚未更新）** HW3 及 HW4 則需要在 HW1 及 HW2 的基礎下，額外設計 Client 端的程式，讓使用者透過該程式連入 server 端進行操作。
    - HW3 中，需要將本地端的 SQLite 資料庫（儲存使用者、板塊、貼文等）移至 Amazon S3 (Amazon Simple Storage Service) 架構下來儲存。<br>
        此外，將加入郵件收發功能。
    - HW4 中，需要另外再建立 Apache Kafka server 來實現板塊訂閱通知、推播訊息等功能。
