#include <iostream>
#include <pqxx/pqxx>
#include <fstream>
#include <sstream>
#include <string>

#include "exerciser.h"

using namespace std;
using namespace pqxx;

void dropTables(connection &C) {
  string drop_sql = "DROP TABLE IF EXISTS PLAYER, TEAM, STATE, COLOR;";
  work W(C);

    W.exec(drop_sql);
    W.commit();
}

void createTables(string fl, connection &C) {
  string create_player_sql, line;
  ifstream ifs;
  ifs.open(fl.c_str(), ifstream::in);
  while (getline(ifs, line)) {
    if (line.empty()) {
      continue;
    }
    create_player_sql += line;
  }
  ifs.close();
  //cout << create_player_sql << endl;
  work W(C);
  W.exec(create_player_sql);
  W.commit();
}

void loadPlayer(string fl, connection *C) {
  ifstream ifs;
  string line, first_name, last_name;
  int player_id, team_id, uniform_num, mpg, ppg, rpg, apg;
  double spg, bpg;
  ifs.open(fl.c_str(), ifstream::in);
  while (getline(ifs, line)) {
    stringstream ss;
    ss << line;
    ss >> player_id >> team_id >> uniform_num >> first_name >> last_name >> mpg >> ppg >> rpg >> apg >> spg >> bpg;
    add_player(C, team_id, uniform_num, first_name, last_name, mpg, ppg, rpg, apg, spg, bpg);
  }
  ifs.close();
}

void loadColor(string fl, connection *C){
  ifstream ifs;
  string line, name;
  int color_id;
  ifs.open(fl.c_str(), ifstream::in);
  while (getline(ifs, line)) {
    stringstream ss;
    ss << line;
    ss >> color_id >> name;
    add_color(C, name);
  }
  ifs.close();
}
void loadTeam(string fl, connection *C) {
  ifstream ifs;
  int team_id, state_id, color_id, wins, losses;
  string line, name;
  ifs.open(fl.c_str(), ifstream::in);
  while(getline(ifs, line)) {
    stringstream ss;
    ss << line;
    ss >> team_id >> name >> state_id >> color_id >> wins >> losses;
    add_team(C, name, state_id, color_id, wins, losses);
  }
  ifs.close();
}
void loadState(string fl, connection *C) {
  ifstream ifs;
  string line, name;
  int state_id;
  ifs.open(fl.c_str(), ifstream::in);
  while (getline(ifs, line)) {
    stringstream ss;
    ss << line;
    ss >> state_id >> name;
    add_state(C, name);
  }
  ifs.close();
}

int main (int argc, char *argv[]) 
{
 
  //Allocate & initialize a Postgres connection object
  connection *C;

  try{
    //Establish a connection to the database
    //Parameters: database name, user name, user password
    C = new connection("dbname=ACC_BBALL user=postgres password=passw0rd");
    if (C->is_open()) {
      //cout << "Opened database successfully: " << C->dbname() << endl;
    } else {
      cout << "Can't open database" << endl;
      return 1;
    }
  } catch (const std::exception &e){
    cerr << e.what() << std::endl;
    return 1;
  }
  //TODO: create PLAYER, TEAM, STATE, and COLOR tables in the ACC_BBALL database
  //      load each table with rows from the provided source txt files
  dropTables(*C);
  createTables("tables.sql", *C);
  loadState("state.txt", C);
  loadColor("color.txt", C);
  loadTeam("team.txt", C);
  loadPlayer("player.txt", C);

  exercise(C);


  //Close database connection
  C->disconnect();
  delete(C);
  return 0;
}


