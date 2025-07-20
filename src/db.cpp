#include "db.hpp"
#include <stdexcept>  // error handling
#include <unordered_set>

VokabelDB::VokabelDB(const std::string& path) {
  if (sqlite3_open(path.c_str(), &db_) != SQLITE_OK)
    throw std::runtime_error("DB open failed");
}

VokabelDB::~VokabelDB() {
  sqlite3_close(db_);
}

bool VokabelDB::isValidTable(const std::string& table) {
    static const std::unordered_set<std::string> validTables = {
        "spanisch_deutsch_erster_versuch",
        "spanisch_deutsch_zweiter_versuch",
        "deutsch_spanisch_erster_versuch",
        "deutsch_spanisch_zweiter_versuch"
    };
    return validTables.find(table) != validTables.end();
}

int VokabelDB::count(const std::string& table) {
    sqlite3_stmt* stmt;
    std::string sql = "SELECT COUNT(*) FROM " + table;
    sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr);
    sqlite3_step(stmt);
    int cnt = sqlite3_column_int(stmt,0);
    sqlite3_finalize(stmt);
    return cnt;
}

std::tuple<int,std::string,std::string> VokabelDB::next(const std::string& table) {
    sqlite3_stmt* stmt;
    std::string sql = "SELECT id, span, deut FROM " + table + " ORDER BY RANDOM() LIMIT 1";
    // ggf. spalte für deutsch_spanisch anders benennen
    sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr);
    if (sqlite3_step(stmt) != SQLITE_ROW)
        throw std::runtime_error("keine Vokabeln");
    int id = sqlite3_column_int(stmt,0);
    std::string f = reinterpret_cast<const char*>(sqlite3_column_text(stmt,1));
    std::string a = reinterpret_cast<const char*>(sqlite3_column_text(stmt,2));
    sqlite3_finalize(stmt);
    return {id,f,a};
}

void VokabelDB::moveWord(const std::string& from, const std::string& to, int id) {
  auto [_, f, a] = next(from); // hier eigentlich SELECT vorab
  exec("INSERT INTO "+to+" (span,deut) VALUES('"+f+"','"+a+"')");
  deleteWord(from,id);
}

void VokabelDB::deleteWord(const std::string& table, int id) {
  exec("DELETE FROM "+table+" WHERE id="+std::to_string(id));
}

void VokabelDB::exec(const std::string& sql) {
  char* err;
  if (sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &err)!=SQLITE_OK)
    throw std::runtime_error(err);
}