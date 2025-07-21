#include "db.hpp"
#include <stdexcept>  // error handling
#include <unordered_set>
#include <fstream>
#include <sstream>
#include <iostream>

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

// Tabelle anlegen, falls nicht vorhanden
void VokabelDB::createTable(const std::string& tablename) { 
    try {
        if(!isValidTable(tablename)) 
            throw("Fehler nicht valide table name");
        std::string createTableSQL =
            "CREATE TABLE IF NOT EXISTS " + tablename + " ("
            "  id   INTEGER PRIMARY KEY AUTOINCREMENT,"
            "  span TEXT    NOT NULL,"
            "  deut TEXT    NOT NULL);";
    
        exec(createTableSQL);
    } catch (...) {
        throw std::runtime_error("Fehler beim Anlegen der Tabelle");
    }
}


void VokabelDB::createDatabaseFromCSV(const std::string& csvPath) {

    createTable("spanisch_deutsch_erster_versuch");
    createTable("spanisch_deutsch_zweiter_versuch");
    createTable("deutsch_spanisch_erster_versuch");
    createTable("deutsch_spanisch_zweiter_versuch");

    char* err = nullptr;
    // Transaktion starten
    if (sqlite3_exec(db_, "BEGIN TRANSACTION;", nullptr, nullptr, &err) != SQLITE_OK) {
        std::string msg = err; 
        sqlite3_free(err);
        throw std::runtime_error("Konnte Transaktion nicht starten: " + msg);
    }

    // Prepared Statement für INSERT
    const char* insertSQL =
        "INSERT INTO spanisch_deutsch_erster_versuch (span, deut) VALUES (?, ?);";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_, insertSQL, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error("Konnte INSERT-Statement nicht erstellen");
    }

    // CSV einlesen
    std::ifstream file(csvPath);
    if (!file.is_open()) {
        sqlite3_finalize(stmt);
        throw std::runtime_error("Konnte CSV-Datei nicht öffnen: " + csvPath);
    }

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        std::stringstream ss(line);
        std::string span, deut;
        if (!std::getline(ss, span, ';') || !std::getline(ss, deut))
            continue; // ungültige Zeile überspringen

        // Werte binden
        sqlite3_bind_text(stmt, 1, span.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, deut.c_str(), -1, SQLITE_TRANSIENT);

        std::cout<<span.c_str()<<" :  "<<deut.c_str()<<"\n";

        // ausführen
        if (sqlite3_step(stmt) != SQLITE_DONE) {
            std::cerr << "Warnung: Insert fehlgeschlagen für '"
                      << span << ";" << deut << "': "
                      << sqlite3_errmsg(db_) << "\n";
        }

        // Statement zurücksetzen für nächsten Durchlauf
        sqlite3_reset(stmt);
    }

    // Statement und Transaktion abschließen
    sqlite3_finalize(stmt);
    if (sqlite3_exec(db_, "COMMIT;", nullptr, nullptr, &err) != SQLITE_OK) {
        std::string msg = err; sqlite3_free(err);
        throw std::runtime_error("Konnte Transaktion nicht abschließen: " + msg);
    }
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