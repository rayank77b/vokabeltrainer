simple vokabel trainer
spanisch deutsch.


ein Web-Server Anwendung  um spanische Woerter zu lernen.
Der Server sollte in C++ entwickelt werden (mit C++-14, g++ und Makefile unter Linux).
Die Datenbank soll ein SQLite DB sein.
Es Beinhaltet 4 Tabellen:
 - spanisch_deutsch_erster_versuch
 - spanisch_deutsch_zweiter_versuch
 - deutsch_spanisch_erster_versuch
 - deutsch_spanisch_zweiter_versuch
Am Anfang sind die Woerter in spanisch_deutsch_erster_versuch. 
Werden die Woerter richtig beantwortet wandern sie nach spanisch_deutsch_zweiter_versuch.
Falls die falsch beantwortet werden, verbleiben sie in spanisch_deutsch_erster_versuch.
bei immer richtigen Antworten, wandern sie immer weiter in nächste Tabelle.
Bis sie ganz gelöscht werden.
Im Browser bekommt man zuerst die Wahl, was man lernen möchte (welche Tabelle) 
und ob noch Woerter in der Tabelle vorhanden sind.
Danach bekommt man den Wort angezeigt und bei klicken auf ok, die Antwort dazu.
Auf Button "Wusste es" wird der Wort als richtig weitergereicht.
Auf Button "Oje" verbleibt der Wort in der Tabelle und es wird der nächste Wort angezeigt.
