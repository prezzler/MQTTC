# MQTTC
MQTT Implementierung in C

MQTTC.h/MQTTC.c beinhalten die vollständige ursprüngliche PubSubClient Bibliothekt in C.

Unsere geteste Implementierung der connect und disconnect Funktion (inklsuive aller notwendigen Funktionen) befinden sich in der Test.c und bereits kompilierte ausführbare Version ist unter MQTTCv1.exe zu finden.

Zur Simulierung des Versenden und des Erhaltens der Informationen/Pakete wird jedes MQTT-Paket, welches versendet wird in eine externe Datei gespeichert (Beispiel: CNN.hex beinhaltet die Bytes eines Connectpakets.) Die Pakete, die entgegengenommen werden, wie zum Beispiel das CONNACK, werden aus einer vorgefertigten Datei ausgelesen (da wir keine Kommunikationsmöglcihkeit haben). 

Unsere Zeitmessfunktion (millis()) basiert auf dem Windowsbetriebsystem und verwendet Bibliothekeigene Funktionen.