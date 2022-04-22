CREATE TABLE `USERS`(
    `UID`           INTEGER     NOT NULL PRIMARY KEY AUTOINCREMENT, 
    `Username`      TEXT        NOT NULL UNIQUE, 
    `Email`         TEXT        NOT NULL, 
    `Password`      TEXT        NOT NULL
);
CREATE TABLE BOARD(
    `BID`           INTEGER     NOT NULL PRIMARY KEY AUTOINCREMENT, 
    `Board_Name`    TEXT        NOT NULL UNIQUE, 
    `Moderator_ID`  INTEGER     NOT NULL, 
    `Moderator`     TEXT        NOT NULL, 
    FOREIGN KEY (`Moderator_ID`)
        REFERENCES `USERS` (`UID`), 
    FOREIGN KEY (`Moderator`)
        REFERENCES `USERS` (`Username`)
);
CREATE TABLE POST(
    `PID`           INTEGER     NOT NULL PRIMARY KEY AUTOINCREMENT, 
    `Board_ID`      INTEGER     NOT NULL, 
    `Title`         TEXT        NOT NULL, 
    `Author_ID`     INTEGER     NOT NULL, 
    `Author`        TEXT        NOT NULL, 
    `Date`          TEXT        DEFAULT (DATE('now','localtime')), 
    FOREIGN KEY (`Board_ID`)
        REFERENCES `BOARD` (`BID`), 
    FOREIGN KEY (`Author_ID`)
        REFERENCES `USERS` (`UID`), 
    FOREIGN KEY (`Author`)
        REFERENCES `USERS` (`Username`)
);