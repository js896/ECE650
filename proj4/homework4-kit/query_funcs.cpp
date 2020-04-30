#include "query_funcs.h"


void add_player(connection *C, int team_id, int jersey_num, string first_name, string last_name,
                int mpg, int ppg, int rpg, int apg, double spg, double bpg)
{   
    work W(*C);
    stringstream ss;
    ss << "INSERT INTO PLAYER (TEAM_ID,UNIFORM_NUM,FIRST_NAME,LAST_NAME,MPG,PPG,RPG,APG,SPG,BPG) VALUES (" << team_id << ", " << jersey_num << ", " << W.quote(first_name) << ", " << W.quote(last_name) << ", " << mpg << ", " << ppg << ", " << rpg << ", " << apg << ", " << spg << ", " << bpg << ");";
    W.exec(ss.str());
    W.commit();
}


void add_team(connection *C, string name, int state_id, int color_id, int wins, int losses)
{
    work W(*C);
    stringstream ss;
    ss << "INSERT INTO TEAM (NAME,STATE_ID,COLOR_ID,WINS,LOSSES) VALUES (" << W.quote(name) << ", " << state_id << ", " << color_id << ", " << wins << ", " << losses << ");";
    W.exec(ss.str());
    W.commit();
}


void add_state(connection *C, string name)
{
    work W(*C);
    stringstream ss;
    ss << "INSERT INTO STATE (NAME) VALUES (" << W.quote(name) << ");";
    W.exec(ss.str());
    W.commit();
}


void add_color(connection *C, string name)
{
    work W(*C);
    stringstream ss;
    ss << "INSERT INTO COLOR (NAME) VALUES (" << W.quote(name) << ");";
    W.exec(ss.str());
    W.commit();
}


void query1(connection *C,
	    int use_mpg, int min_mpg, int max_mpg,
            int use_ppg, int min_ppg, int max_ppg,
            int use_rpg, int min_rpg, int max_rpg,
            int use_apg, int min_apg, int max_apg,
            int use_spg, double min_spg, double max_spg,
            int use_bpg, double min_bpg, double max_bpg
            )
{
    nontransaction N(*C);
    stringstream ss;
    ss << "SELECT * FROM PLAYER";
    if (use_mpg+use_ppg+use_rpg+use_apg+use_spg+use_bpg > 0) {
        ss << " WHERE ";
        bool use = false;
        if (use_mpg) {
            if (use) {
                ss << " AND ";
            }
            ss << " MPG >= " << min_mpg << " AND MPG <= " << max_mpg;
            use = true;
        }
        if (use_ppg) {
            if (use) {
                ss << " AND ";
            }
            ss << " PPG >= " << min_ppg << " AND PPG <= " << max_ppg;
            use = true;
        }
        if (use_rpg) {
            if (use) {
                ss << " AND ";
            }
            ss << " RPG >= " << min_rpg << " AND RPG <= " << max_rpg;
            use = true;
        }
        if (use_apg) {
            if (use) {
                ss << " AND ";
            }
            ss << " APG >= " << min_apg << " AND APG <= " << max_apg;
            use = true;
        }
        if (use_spg) {
            if (use) {
                ss << " AND ";
            }
            ss << " SPG >= " << to_string(min_spg) << " AND SPG <= " << to_string(max_spg);
            use = true;
        }
        if (use_bpg) {
            if (use) {
                ss << " AND ";
            }
            ss << " BPG >= " << to_string(min_bpg) << " AND BPG <= " << to_string(max_bpg);
            use = true;
        }
    }
    ss << ";";
    result r;
    r = N.exec(ss.str());
    cout << "PLAYER_ID TEAM_ID UNIFORM_NUM FIRST_NAME LAST_NAME MPG PPG RPG APG SPG BPG" << endl;
    for (auto row:r) {
        for (auto field: row) std::cout << field.c_str() << " ";
        cout << endl;
    }
}


void query2(connection *C, string team_color)
{
    nontransaction N(*C);
    stringstream ss;
    ss << "SELECT TEAM.NAME FROM TEAM, COLOR WHERE TEAM.COLOR_ID = COLOR.COLOR_ID AND COLOR.NAME = " << N.quote(team_color) << ";";
    result r;
    r = N.exec(ss.str());
    cout << "NAME" << endl;
    for (auto row:r) {
        for (auto field: row) std::cout << field.c_str() << " ";
        cout << endl;
    }
}


void query3(connection *C, string team_name)
{
    nontransaction N(*C);
    stringstream ss;
    ss << "SELECT FIRST_NAME, LAST_NAME FROM PLAYER, TEAM WHERE PLAYER.TEAM_ID = TEAM.TEAM_ID AND NAME = " << N.quote(team_name) << "ORDER BY PPG DESC;";
    result r;
    r = N.exec(ss.str());
    cout << "FIRST_NAME LAST_NAME" << endl;
    for (auto row:r) {
        for (auto field: row) std::cout << field.c_str() << " ";
        cout << endl;
    }
}


void query4(connection *C, string team_state, string team_color)
{
    nontransaction N(*C);
    stringstream ss;
    ss << "SELECT FIRST_NAME, LAST_NAME, UNIFORM_NUM FROM PLAYER, TEAM, STATE, COLOR WHERE PLAYER.TEAM_ID = TEAM.TEAM_ID AND TEAM.STATE_ID = STATE.STATE_ID AND TEAM.COLOR_ID = COLOR.COLOR_ID AND STATE.NAME = " << N.quote(team_state) << " AND COLOR.NAME = " << N.quote(team_color) << ";";
    result r;
    r = N.exec(ss.str());
    cout << "FIRST_NAME LAST_NAME UNIFORM_NUM" << endl;
    for (auto row:r) {
        for (auto field: row) std::cout << field.c_str() << " ";
        cout << endl;
    }
}


void query5(connection *C, int num_wins)
{
    nontransaction N(*C);
    stringstream ss;
    ss << "SELECT FIRST_NAME, LAST_NAME, NAME, WINS FROM PLAYER, TEAM WHERE PLAYER.TEAM_ID = TEAM.TEAM_ID AND WINS > " << num_wins << ";";
    result r;
    r = N.exec(ss.str());
    cout << "FIRST_NAME LAST_NAME NAME WINS" << endl;
    for (auto row:r) {
        for (auto field: row) std::cout << field.c_str() << " ";
        cout << endl;
    }
}
