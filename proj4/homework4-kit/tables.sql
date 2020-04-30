CREATE TABLE STATE
    (   STATE_ID        SERIAL          NOT NULL,
        NAME            VARCHAR(30)     NOT NULL,
        PRIMARY KEY (STATE_ID)
    );

CREATE TABLE COLOR
    (   COLOR_ID            SERIAL          NOT NULL,
        NAME                VARCHAR(30)     NOT NULL,
        PRIMARY KEY (COLOR_ID)
    );

CREATE TABLE TEAM
     (   TEAM_ID         SERIAL          NOT NULL,
         NAME            VARCHAR(30)     NOT NULL,
         STATE_ID        INT             NOT NULL,
         COLOR_ID        INT             NOT NULL,
         WINS            INT             NOT NULL,
         LOSSES          INT             NOT NULL,
         PRIMARY KEY (TEAM_ID),
         FOREIGN KEY (STATE_ID) REFERENCES STATE (STATE_ID),
         FOREIGN KEY (COLOR_ID) REFERENCES COLOR (COLOR_ID)
    );

CREATE TABLE PLAYER
    (   PLAYER_ID       SERIAL          NOT NULL,
        TEAM_ID         INT             NOT NULL,
        UNIFORM_NUM     INT             NOT NULL, 
        FIRST_NAME      VARCHAR(30)     NOT NULL,
        LAST_NAME       VARCHAR(30)     NOT NULL,
        MPG             INT             NOT NULL,
        PPG             INT             NOT NULL,
        RPG             INT             NOT NULL,
        APG             INT             NOT NULL,
        SPG             NUMERIC(5,1)           NOT NULL,
        BPG             NUMERIC(5,1)           NOT NULL,
        PRIMARY KEY (PLAYER_ID),
        FOREIGN KEY (TEAM_ID) REFERENCES TEAM(TEAM_ID)   
    );