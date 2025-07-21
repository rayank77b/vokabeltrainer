#include "server.hpp"
#include <httplib.h>
#include <jsoncpp/json/json.h>   // JsonCpp
using namespace httplib;

void runServer(VokabelDB& db) {
    Server svr;
    svr.set_mount_point("/", "./src/static");

    // --- Count-Endpoint ---
    svr.Get(R"(/api/count)", [&](const Request& req, Response& res) {
        auto table = req.get_param_value("table");
        int cnt = db.count(table);

        // JsonCpp: Build JSON
        Json::Value root;
        root["count"] = cnt;
        Json::StreamWriterBuilder wbuilder;
        std::string out = Json::writeString(wbuilder, root);

        res.set_content(out, "application/json; charset=utf-8");
    });

    // --- Next-Word-Endpoint ---
    svr.Get(R"(/api/next)", [&](const Request& req, Response& res) {
        auto table = req.get_param_value("table");
        try {
            auto tup = db.next(table);
            int id       = std::get<0>(tup);
            const auto& frage = std::get<1>(tup);
            const auto& antwort = std::get<2>(tup);
            // Build JSON
            Json::Value root;
            root["id"]    = id;
            root["frage"] = frage;
            root["antwort"] = antwort;
            Json::StreamWriterBuilder wbuilder;
            std::string out = Json::writeString(wbuilder, root);

            res.set_content(out, "application/json; charset=utf-8");
        } catch(...) {
            res.status = 404;
            Json::Value err;
            err["error"] = "leer";
            Json::StreamWriterBuilder wbuilder;
            res.set_content(Json::writeString(wbuilder, err), "application/json; charset=utf-8");
        }
    });

    // --- Antwort verarbeiten ---
    svr.Post(R"(/api/answer)", [&](const Request& req, Response& res) {
        // Parse JSON-Body
        Json::CharReaderBuilder rbuilder;
        std::string errs;
        Json::Value body;
        std::istringstream iss(req.body);
        if (!Json::parseFromStream(rbuilder, iss, &body, &errs)) {
            res.status = 400;
            Json::Value err; err["error"] = "invalid_json";
            res.set_content(Json::writeString(Json::StreamWriterBuilder(), err),
                            "application/json; charset=utf-8");
            return;
        }

        std::string table  = body["table"].asString();
        int         id     = body["id"].asInt();
        bool        correct= body["correct"].asBool();

        // Bestimme Ziel-Tabelle
        std::string nextTable;

        if (table == "spanisch_deutsch_erster_versuch") 
            nextTable = "spanisch_deutsch_zweiter_versuch";
        else if (table == "spanisch_deutsch_zweiter_versuch" ) 
            nextTable = "deutsch_spanisch_erster_versuch";
        else if (table == "deutsch_spanisch_erster_versuch" ) 
            nextTable = "deutsch_spanisch_zweiter_versuch";
        else if (table == "deutsch_spanisch_zweiter_versuch" ) 
            nextTable = "";

        // Move oder delete
        if (correct) {
            if (!nextTable.empty())
                db.moveWord(table, nextTable, id);
            else
                db.deleteWord(table, id);
        }
        // bei falsch: nichts tun (bleibt in derselben Tabelle)

        // Response OK
        Json::Value ok; ok["ok"] = true;
        res.set_content(Json::writeString(Json::StreamWriterBuilder(), ok),
                        "application/json; charset=utf-8");
    });

    svr.listen("0.0.0.0", 8080);
}