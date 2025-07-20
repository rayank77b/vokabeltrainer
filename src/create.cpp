#include "create.hpp"

#include <sqlite3.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <stdexcept>

void createDatabaseFromCSV(const std::string& csvPath, const std::string& dbPath) {
    // DB öffnen (erstellt Datei, wenn nötig)
    sqlite3* db = nullptr;
    if (sqlite3_open(dbPath.c_str(), &db) != SQLITE_OK) {
        throw std::runtime_error("Konnte DB nicht öffnen: " + std::string(sqlite3_errmsg(db)));
    }

    // Tabelle anlegen, falls nicht vorhanden
    const char* createTableSQL =
        "CREATE TABLE IF NOT EXISTS spanisch_deutsch_erster_versuch ("
        "  id   INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  span TEXT    NOT NULL,"
        "  deut TEXT    NOT NULL"
        ");";
    char* err = nullptr;
    if (sqlite3_exec(db, createTableSQL, nullptr, nullptr, &err) != SQLITE_OK) {
        std::string msg = err;
        sqlite3_free(err);
        sqlite3_close(db);
        throw std::runtime_error("Fehler beim Anlegen der Tabelle: " + msg);
    }

    // Transaktion starten
    if (sqlite3_exec(db, "BEGIN TRANSACTION;", nullptr, nullptr, &err) != SQLITE_OK) {
        std::string msg = err; sqlite3_free(err);
        sqlite3_close(db);
        throw std::runtime_error("Konnte Transaktion nicht starten: " + msg);
    }

    // Prepared Statement für INSERT
    const char* insertSQL =
        "INSERT INTO spanisch_deutsch_erster_versuch (span, deut) VALUES (?, ?);";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, insertSQL, -1, &stmt, nullptr) != SQLITE_OK) {
        sqlite3_close(db);
        throw std::runtime_error("Konnte INSERT-Statement nicht erstellen");
    }

    // CSV einlesen
    std::ifstream file(csvPath);
    if (!file.is_open()) {
        sqlite3_finalize(stmt);
        sqlite3_close(db);
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

        // ausführen
        if (sqlite3_step(stmt) != SQLITE_DONE) {
            std::cerr << "Warnung: Insert fehlgeschlagen für '"
                      << span << ";" << deut << "': "
                      << sqlite3_errmsg(db) << "\n";
        }

        // Statement zurücksetzen für nächsten Durchlauf
        sqlite3_reset(stmt);
    }

    // Statement und Transaktion abschließen
    sqlite3_finalize(stmt);
    if (sqlite3_exec(db, "COMMIT;", nullptr, nullptr, &err) != SQLITE_OK) {
        std::string msg = err; sqlite3_free(err);
        sqlite3_close(db);
        throw std::runtime_error("Konnte Transaktion nicht abschließen: " + msg);
    }

    sqlite3_close(db);
}