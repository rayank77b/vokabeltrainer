#include "db.hpp"
#include "server.hpp"
#include <iostream>
#include <string>


int main(int argc, char* argv[]) {

    VokabelDB db("db/vokabeln.db");

    if (argc >1 && argc != 3) {
        std::cerr << "Usage: " << argv[0] << " -c dateiname.csv\n";
        std::cerr << "      for fill csv-vocabels in db/vokabeln.db\n\n";
        std::cerr << "Usage: " << argv[0] << std::endl;
        std::cerr << "      to start http-server ->  http://localhost:8080/\n\n";
        return 1;
    } else if (argc == 3 && std::string(argv[1]) == "-c") {
        const std::string csvFile = argv[2];
        try {
            db.createDatabaseFromCSV(csvFile);
            std::cout << "Datenbank 'db/vokabeln.db' erfolgreich angelegt und befüllt.\n";
            return 0;
        } catch (const std::exception& ex) {
            std::cerr << "Fehler: " << ex.what() << "\n";
            return 1;
        }
    } 

    // starte den http server
    runServer(db);
    return 0;
}