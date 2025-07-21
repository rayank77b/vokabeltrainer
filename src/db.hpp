#pragma once
#include <string>
#include <vector>
#include <tuple>
#include <sqlite3.h>

class VokabelDB {
public:
  VokabelDB(const std::string& path);
  ~VokabelDB();

  // prueft ob tabelle-name in ordnung ist.
  bool isValidTable(const std::string& table);
  bool isSpanDeut(const std::string& table);
  // Anzahl übrig in Tabelle
  int count(const std::string& table);
  // nächstes Wort (id, frage, antwort)
  std::tuple<int,std::string,std::string> next(const std::string& table);
  std::tuple<int,std::string,std::string> nextWhere(const std::string& table, const int id);
  // verschieben bei richtig/falsch
  void moveWord(const std::string& from, const std::string& to, int id);
  void deleteWord(const std::string& table, int id);

  void createTable(const std::string& tablename, const bool span_first);
  void createDatabaseFromCSV(const std::string& csvPath);
  

private:
  sqlite3* db_;
  void exec(const std::string& sql);
};