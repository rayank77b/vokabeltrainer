#pragma once
#include "db.hpp"
#include <httplib.h>
#include <jsoncpp/json/json.h>   // JsonCpp

void runServer(VokabelDB& db);