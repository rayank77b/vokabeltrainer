#pragma once

#include <string>

/**
 * @brief Liest eine CSV-Datei mit "spanisch;deutsch"-Paaren ein,
 *        erstellt (falls nicht vorhanden) die DB unter dbPath
 *        und füllt die Tabelle spanisch_deutsch_erster_versuch.
 *
 * @param csvPath Pfad zur CSV-Datei (Trennzeichen ';')
 * @param dbPath  Pfad zur SQLite‑Datenbank (z.B. "db/vokablen.db")
 * @throws std::runtime_error bei Fehlern
 */
void createDatabaseFromCSV(const std::string& csvPath, const std::string& dbPath);
