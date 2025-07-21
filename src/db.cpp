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

bool VokabelDB::isSpanDeut(const std::string& table) {
    if(table=="spanisch_deutsch_erster_versuch" || table == "spanisch_deutsch_zweiter_versuch")
        return true;
    return false;
}

// Tabelle anlegen, falls nicht vorhanden
void VokabelDB::createTable(const std::string& tablename, const bool span_first) { 
    try {
        if(!isValidTable(tablename)) 
            throw("Fehler nicht valide table name");
        std::string createTableSQL;
        if(span_first) {
            createTableSQL =
            "CREATE TABLE IF NOT EXISTS " + tablename + " ("
            "  id   INTEGER PRIMARY KEY AUTOINCREMENT,"
            "  span TEXT    NOT NULL,"
            "  deut TEXT    NOT NULL);";
        } else {
            createTableSQL =
            "CREATE TABLE IF NOT EXISTS " + tablename + " ("
            "  id   INTEGER PRIMARY KEY AUTOINCREMENT,"
            "  deut TEXT    NOT NULL,"
            "  span TEXT    NOT NULL);";
        }
    
        exec(createTableSQL);
    } catch (...) {
        throw std::runtime_error("Fehler beim Anlegen der Tabelle");
    }
}


void VokabelDB::createDatabaseFromCSV(const std::string& csvPath) {

    std::cout << "[+] create tables\n";
    createTable("spanisch_deutsch_erster_versuch", true);
    createTable("spanisch_deutsch_zweiter_versuch", true);
    createTable("deutsch_spanisch_erster_versuch", false);
    createTable("deutsch_spanisch_zweiter_versuch", false);

    char* err = nullptr;
    std::cout << "[+] begin transaction\n";
    // Transaktion starten
    if (sqlite3_exec(db_, "BEGIN TRANSACTION;", nullptr, nullptr, &err) != SQLITE_OK) {
        std::string msg = err; 
        sqlite3_free(err);
        throw std::runtime_error("Konnte Transaktion nicht starten: " + msg);
    }

    std::cout << "[+] prepare insert statement\n";
    // Prepared Statement für INSERT
    const char* insertSQL =
        "INSERT INTO spanisch_deutsch_erster_versuch (span, deut) VALUES (?, ?);";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_, insertSQL, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error("Konnte INSERT-Statement nicht erstellen");
    }

    std::cout << "[+] open csv file\n";
    // CSV einlesen
    std::ifstream file(csvPath);
    if (!file.is_open()) {
        sqlite3_finalize(stmt);
        throw std::runtime_error("Konnte CSV-Datei nicht öffnen: " + csvPath);
    }

    std::cout << "[+] insert words ....\n";
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

    std::cout << "[+] insertion loop is ok\n";
    // Statement und Transaktion abschließen
    sqlite3_finalize(stmt);
    if (sqlite3_exec(db_, "COMMIT;", nullptr, nullptr, &err) != SQLITE_OK) {
        std::string msg = err; sqlite3_free(err);
        throw std::runtime_error("Konnte Transaktion nicht abschließen: " + msg);
    }
    std::cout << "[+] commit done\n";
}

int VokabelDB::count(const std::string& table) {
    if(!isValidTable(table)) 
        throw("Fehler nicht valide table name");

    sqlite3_stmt* stmt;
    std::string sql = "SELECT COUNT(*) FROM " + table;
    sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr);
    sqlite3_step(stmt);
    int cnt = sqlite3_column_int(stmt,0);
    sqlite3_finalize(stmt);
    return cnt;
}

std::tuple<int,std::string,std::string> VokabelDB::next(const std::string& table) {
    if(!isValidTable(table)) 
        throw("Fehler nicht valide table name");
        
    sqlite3_stmt* stmt;
    std::string sql = "SELECT id,deut,span FROM " + table + " ORDER BY RANDOM() LIMIT 1";
    if(isSpanDeut(table))
        sql = "SELECT id,span,deut FROM " + table + " ORDER BY RANDOM() LIMIT 1";
    sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr);
    if (sqlite3_step(stmt) != SQLITE_ROW)
        throw std::runtime_error("keine Vokabeln");
    int id = sqlite3_column_int(stmt,0);
    std::string f = reinterpret_cast<const char*>(sqlite3_column_text(stmt,1));
    std::string a = reinterpret_cast<const char*>(sqlite3_column_text(stmt,2));
    sqlite3_finalize(stmt);
    return {id,f,a};
}

std::tuple<int,std::string,std::string> VokabelDB::nextWhere(const std::string& table, const int id) {
    if(!isValidTable(table)) 
        throw("Fehler nicht valide table name");
        
    sqlite3_stmt* stmt;
    std::string sql = "SELECT deut,span FROM " + table + " WHERE id="+std::to_string(id);
    if(isSpanDeut(table))
        sql = "SELECT span,deut FROM " + table + " WHERE id="+std::to_string(id);
    sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr);
    if (sqlite3_step(stmt) != SQLITE_ROW)
        throw std::runtime_error("keine Vokabeln");
    std::string f = reinterpret_cast<const char*>(sqlite3_column_text(stmt,0));
    std::string a = reinterpret_cast<const char*>(sqlite3_column_text(stmt,1));
    sqlite3_finalize(stmt);
    return {id,f,a};
}

void VokabelDB::moveWord(const std::string& from, const std::string& to, int id) {
    if(!isValidTable(from)) 
        throw("Fehler nicht valide table name");
    if(!isValidTable(to)) 
        throw("Fehler nicht valide table name");
    
    auto [_, f, a] = nextWhere(from,id); 
    std::string sql = "INSERT INTO "+to+" (span,deut) VALUES('"+f+"','"+a+"')";
    if(to=="deutsch_spanisch_zweiter_versuch")
        sql = "INSERT INTO "+to+" (deut,span) VALUES('"+f+"','"+a+"')";
    exec(sql);
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