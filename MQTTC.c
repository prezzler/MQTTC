
#include "fwf_defs.h"
#include "fwf_typ_defines.h"

#include "fwf_api_com.h"   // FWF_COM_CMD MULTICAST_JOIN_LEAVE_STATUS
#include "fwf_api_flash.h" // CONTROLLER_NAME
#include "fwf_dhcp.h"      // DHCP_STATE
#include "fwf_uc_status.h"

#include "mqttc.h"

// #if USE_MQTT_SERVER

//---------------------------------Definition der Funktionen-------------------------------------------------------
// #if 0
// TCP-Funktionen, die wir brauchen:
bool Client_connected();                     // Funktion, die prüft ob eine Verbindung besteht (TCP)
void Client_stop();                          // Schließt die TCP-Verbindung zum Broker
void Client_flush();                         // gibt den Empfangsbuffer vom Netzwerk Client frei (TCP)
int Client_connectIP(char *ip, UINT16 port); // stellt eine Verbindung mit dem Broker her (TCP)
UINT16 Client_write(buffer, length);         // versendet MQTT-Paket über TCP
int Client_available();                      // prüft ob Nachrichten angekommen bzw. es neue Daten gibt (TCP)
UINT8 Client_read();                         // liest 1 Byte von den empfangenen Daten
// #endif

UINT8 checkStringLength(PubSubClient *src, int len, const char *s)
{
    if (len + 2 + strnlen(s, src->bufferSize) > src->bufferSize)
    {
        // Client_stop(src->_client);
        return 0;
    }
    return 1;
}

UINT32 millis()
{ // Millisekunden seit POR
    return uC.timer_1ms;
}

void setConnectUser(MqttConnect *src, char *user, char *pwd)
{
    strcpy(src->user, user);
    strcpy(src->pwd, pwd);
}

void setConnectWill(MqttConnect *src, char *willTopic, UINT8 willQos, bool willRetain, char *willMessage)
{
    strcpy(src->willTopic, willTopic);
    src->willQos = willQos;
    src->willRetain = willRetain;
    strcpy(src->willMessage, willMessage);
}

MqttConnect *MqttConnectKonstruktor(PubSubClient *src, char *id, char *ip, UINT16 port) // setzt Standartwerte
// Parameterübergabe von notwendigen Attributen und Setter für optionale Attribute
{
    static MqttConnect tmpConnect;
    tmpConnect.PSCsrc = src;
    // notwendige Attribute
    tmpConnect.id = NULL;
    strcpy(tmpConnect.id, id);
    tmpConnect.ip = NULL;
    strcpy(tmpConnect.ip, ip);
    tmpConnect.domain = NULL;
    tmpConnect.port = 0;
    tmpConnect.port = port;
    tmpConnect.cleanSession = 1;

    // optionale Attribute
    tmpConnect.pwd = NULL;
    tmpConnect.user = NULL;

    tmpConnect.willMessage = NULL;
    tmpConnect.willQos = MQTTQOS0; // Ka
    tmpConnect.willRetain = NULL;
    tmpConnect.willTopic = NULL;

    return &tmpConnect;
}
// Client *construct_Client()
// {
//     static Client thisClient;
//     thisClient.ip = 0;
//     thisClient.port = 0;
//     thisClient.sock = 0;
//     return &thisClient;
// }
//_______________________________Beginn-PubSubClient-Funktionen_____________________________________________
PubSubClient *PubSubConstructor()
{
    static PubSubClient tmpPubSubClient;
    tmpPubSubClient._state = MQTT_DISCONNECTED;
    setCallback(&tmpPubSubClient, NULL);
    setBufferSize(&tmpPubSubClient, MQTT_MAX_PACKET_SIZE); // hier wird auch tmpPubSubClient.bufferSize = MQTT_MAX_PACKET_SIZE gesetzt
    setKeepAlive(&tmpPubSubClient, MQTT_KEEPALIVE);
    setSocketTimeout(&tmpPubSubClient, MQTT_SOCKET_TIMEOUT);

    return &tmpPubSubClient;
}

void setCallback(PubSubClient *src, MQTT_CALLBACK_SIGNATURE callback)
{
    src->callback = callback;
}

void setSocketTimeout(PubSubClient *src, UINT16 timeout)
{
    src->socketTimeout = timeout;
}

void setKeepAlive(PubSubClient *src, UINT16 keepAlive)
{
    src->keepAlive = keepAlive;
}

char setBufferSize(PubSubClient *src, UINT16 size)
{
    if (size == 0)
    {
        // Cannot set it back to 0
        return false;
    }
    if (src->bufferSize == 0)
    {
        src->buffer = (uint8_t *)malloc(size);
    }
    else
    {
        uint8_t *newBuffer = (uint8_t *)realloc(src->buffer, size);
        if (newBuffer != NULL)
        {
            src->buffer = newBuffer;
        }
        else
        {
            return false;
        }
    }
    src->bufferSize = size;
    return (src->buffer != NULL);
}

UINT16 getBufferSize(PubSubClient *src)
{
    return src->bufferSize;
}

bool connected(PubSubClient *src)
{
    bool rc;
    rc = (int)Client_connected();
    if (!rc)
    { // es besteht keine TCP-Verbindung
        if (src->_state == MQTT_CONNECTED)
        {
            src->_state = MQTT_CONNECTION_LOST
            Client_flush();
            Client_stop();
        }
    }
    else
    {
        return src->_state == MQT_CONNECTED;
    }
    return rc;
}

UINT16 writeString(const char *string, uint8_t *buf, UINT16 pos)
{
    const char *idp = string;
    UINT16 i = 0;
    pos += 2;
    while (*idp)
    {
        buf[pos++] = *idp++;
        i++;
    }
    buf[pos - i - 2] = (i >> 8);   // high Byte
    buf[pos - i - 1] = (i & 0xFF); // low Byte
    return pos;
}

char write(PubSubClient *src, uint8_t header, UINT16 length)
{
    UINT16 rc;
    UINT8 hlen = buildHeader(header, src->buffer, length);
    rc = Client_write(&(src->buffer[MQTT_MAX_HEADER_SIZE - hlen]), length + hlen);
    src->lastOutActivity = millis();
    return (rc == hlen + length);
}

char connectStart(PubSubClient *src, MqttConnect *con)
{
    if (!connected(src)) // prüft ob eine TCP UND eine MQTT-Verbindung bereits besteht
    {
        int result = 0;
        if (Client_connected(&src->_client)) // prüft ob eine TCP Verbindung besteht
        {
            result = 1;
        }
        else
        {
            if (con->domain != NULL)
            {
                result = Client_connectDomain(&con->domain, con->port);
            }
            else
            {
                result = Client_connectIp(&con->ip, con->port);
            }
        }

        if (result == 1)
        {
            src->nextMsgId = 1;
            // Leave room in the buffer for header and variable length field
            uint16_t length = MQTT_MAX_HEADER_SIZE; // 5
            unsigned int j;

#if MQTT_VERSION == MQTT_VERSION_3_1
            uint8_t d[9] = {0x00, 0x06, 'M', 'Q', 'I', 's', 'd', 'p', MQTT_VERSION};
#define MQTT_HEADER_VERSION_LENGTH 9
#elif MQTT_VERSION == MQTT_VERSION_3_1_1
            uint8_t d[7] = {0x00, 0x04, 'M', 'Q', 'T', 'T', MQTT_VERSION};
#define MQTT_HEADER_VERSION_LENGTH 7
#endif
            for (j = 0; j < MQTT_HEADER_VERSION_LENGTH; j++)
            {
                src->buffer[length++] = d[j]; // schreiben ab Index 5 das 0x00,0x04,'M','Q','T','T',MQTT_VERSION hin und landen an stelle 12
            }
            // Als nächstes werden die Connect Flags (8Bit) gesetzt ---------------
            // Erinnerung:
            // 0... .... = User Name Flag: Not set
            //.0.. .... = Password Flag: Not set
            //..0. .... = Will Retain: Not set
            //...0 0... = QoS Level: At most once delivery (Fire and Forget) (0)
            //.... .0.. = Will Flag: Not set
            //.... ..1. = Clean Session Flag: Set
            //.... ...0 = (Reserved): Not set
            // | ist der bitweise OR Operator d.h. 0100 0000 | 0000 01000 = 0100 0100

            uint8_t v;
            if (con->willTopic != NULL)
            {
                v = 0x04 | (con->willQos << 3) | (con->willRetain << 5);
            }
            else
            {
                v = 0x00;
            }
            if (con->cleanSession)
            {
                v = v | 0x02;
            }

            if (con->user != NULL)
            {
                v = v | 0x80;

                if (con->pass != NULL)
                {
                    v = v | (0x80 >> 1);
                }
            }
            // die Connect Flags werden als ein 1Byte Element mit an den Buffer gehangen
            src->buffer[length++] = v;

            // keepAlive wird an den Buffer angefuegt. keepAlive ist 16 bit daher obere 8 Bits als erstes und dann untere 8+8=16
            src->buffer[length++] = ((src->keepAlive) >> 8);
            src->buffer[length++] = ((src->keepAlive) & 0xFF);

            // ID Größe wird geprueft und in Buffer hinzugefuegt
            checkStringLength(src, length, con->id); // bei &con->id ist es wichtig die Adresse also mit & weiterzugeben, da con->id
            length = writeString(con->id, src->buffer, length);

            // WillTopic wird geprueft und in Buffer hinzugefuegt
            if (con->willTopic)
            {
                checkStringLength(src, length, con->willTopic);
                length = writeString(con->willTopic, src->buffer, length);
                checkStringLength(src, length, con->willMessage);
                length = writeString(con->willMessage, src->buffer, length);
            }

            // Username und Password wird geprueft und in Buffer hinzugefuegt
            if (con->user != NULL)
            {
                checkStringLength(src, length, con->user);
                length = writeString(con->user, src->buffer, length);
                if (con->pass != NULL)
                {
                    checkStringLength(src, length, con->pass);
                    length = writeString(con->pass, src->buffer, length);
                }
            }

            write(src, MQTTCONNECT, length - MQTT_MAX_HEADER_SIZE);

            src->lastInActivity = src->lastOutActivity = millis();

            while (!Client_available(src->_client)) // schaut ob Nachrichten angekommen sind (Daten verfügbar sind)
            {
                unsigned long t = millis();
                // Zeit wird gestoppt
                if (t - src->lastInActivity >= ((int32_t)src->socketTimeout * 1000UL))
                {
                    src->_state = MQTT_CONNECTION_TIMEOUT;
                    Client_stop(src->_client);
                    return false;
                }
            }
            // weil die while Schleife verlassen wurde, gehen wir davon aus, dass eine einkommende Nachricht vorliegt
            uint16_t llen;
            uint32_t len = readPacket(src, &llen);

            if (len == 4)
            { // wenn die Länge des angekommenen Pakets 4 ist, handelt es sich um ein CONACK
                if (src->buffer[3] == 0)
                { // letzter Byte eines CONACK ist der RC, wenn der 0 ist --> erfolgreiche Verbindung
                    src->lastInActivity = millis();
                    src->pingOutstanding = false;
                    src->_state = MQTT_CONNECTED;
                    return true;
                }
                else
                {
                    src->_state = src->buffer[3]; // sonst wird der RC Wert des CONACKS in State geschrieben
                }
            }
            Client_stop(src->_client);
        }
        else
        {
            src->_state = MQTT_CONNECT_FAILED;
        }
        return false;
    }
    return true;
}

UINT8 *disconnect(PubSubClient *src)
{
    src->buffer[0] = MQTTDISCONNECT;
    src->buffer[1] = 0;
    src->lastInActivity = src->lastOutActivity = millis(); // millis ist  fuer arduino, kann man aber auch mit time.h bib realisieren
    return &src->buffer[0];
    /*_client->write(this->buffer, 2);
    _state = MQTT_DISCONNECTED;
    _client->flush();
    _client->stop();*/
}

char publish(PubSubClient *src, const char *topic, const char *payload)
{
    return 2;
}

// Header wird in die ersten 4 Stellen des Buffers geschrieben
size_t buildHeader(uint8_t header, uint8_t *buf, uint16_t length)
{
    uint8_t lenBuf[4];
    uint8_t llen = 0;
    uint8_t digit;
    uint8_t pos = 0;
    uint16_t len = length;
    do
    {
        digit = len & 127; // digit = len %128
        len >>= 7;         // len = len / 128
        if (len > 0)
        {
            digit |= 0x80;
        }
        lenBuf[pos++] = digit;
        llen++;
    } while (len > 0);

    buf[4 - llen] = header;
    for (int i = 0; i < llen; i++)
    {
        buf[MQTT_MAX_HEADER_SIZE - llen + i] = lenBuf[i];
    }
    return llen + 1; // Full header size is variable length bit plus the 1-byte fixed header
}

// reads a byte into src->buffer and increments index
bool readByteIntoBuff(PubSubClient *src, uint16_t *index)
{
    uint16_t current_index = *index;
    uint8_t *write_address = &(src->buffer[current_index]);

    if (readByte(write_address))
    {
        *index = current_index + 1;
        return true;
    }
    return false;
}

// reads a byte into result
bool readByte(uint8_t *result)
{
    uint32_t previousMillis = millis();
    while (!Client_available())
    { // solange keine einkommende Nachricht verfuegbar ist
        uint32_t currentMillis = millis();

        if (currentMillis - previousMillis >= ((int32_t)src->socketTimeout * 1000))
        {
            return false;
        }
    }
    *result = Client_read(); // lesen von byte (TCP)
    return true;
}

uint32_t readPacket(PubSubClient *src, uint16_t *lengthLength)
{
    uint16_t len = 0;
    if (!readByteIntoBuff(src, &len))
        return 0;
    bool isPublish = (src->buffer[0] & 0xF0) == MQTTPUBLISH;
    uint32_t multiplier = 1;
    uint32_t length = 0;
    uint8_t digit = 0;
    // uint16_t skip 	= 0;
    uint32_t start = 0;

    do
    {
        if (len == 5)
        {
            // Check for invalid remaining length encoding
            src->_state = MQTT_DISCONNECTED;
            Client_stop();
            return 0;
        }
        if (!readByte(&digit))
            return 0;
        src->buffer[len++] = digit;

        // Update the length using variable length encoding
        length += (digit & 127) * multiplier;
        multiplier <<= 7; // multiplier *= 128
    } while ((digit & 128) != 0);
    *lengthLength = len - 1;

    uint32_t idx = len;

    for (uint32_t i = start; i < length; i++)
    {
        // lesen eines Bytes
        if (!readByte(&digit))
            return 0;

        // Speichern des Bytes in den Buffer
        if (len < src->bufferSize)
        {
            src->buffer[len] = digit;
            len++;
        }
        idx++;
    }

    if (idx > src->bufferSize)
    {
        len = 0; // This will cause the packet to be ignored.
    }

    return len;
}

//_______________________________Ende-PubSubClient-Funktionen__________________________________________________

// Return value: erklaeren! // Kaever: erklaeren
UINT8 MQTT_loop(PubSubClient *pub, MqttConnect *con)
{
    uint16_t llen;
    uint16_t len;
    uint16_t msgId;
    uint8_t *payload;

    if (connected())
    { // Kaever: erklaeren
        unsigned long t = millis();
        if ((t - pub->lastInActivity > pub->keepAlive * 1000UL) || (t - pub->lastOutActivity > pub->keepAlive * 1000UL))
        {
            if (pub->pingOutstanding)
            {
                pub->_state = MQTT_CONNECTION_TIMEOUT;
                Client_stop(pub->_client); // client stop funktion
                return false;
            }
            else
            {
                pub->buffer[0] = MQTTPINGREQ;
                pub->buffer[1] = 0;
                write(pub, this.buffer, 2); // client write function
                pub->lastOutActivity = t;
                pub->lastInActivity = t;
                pub->pingOutstanding = true;
            }
        }

        if (Client_available(pub->_client))
        { // Client_available muesste von rossendorf uebernommen werden // Kaever: erklaeren
            len = readPacket(pub, &llen);
            msgId = 0;
            if (len > 0)
            {
                pub->lastInActivity = t;
                uint8_t type = pub->buffer[0] & 0xF0;
                if (type == MQTTPUBLISH)
                {
#if 0
                    if (pub->callback)  {
                        uint16_t tl = (pub->buffer[llen+1]<<8)+pub->buffer[llen+2]; /* topic length in bytes */
                        memmove(pub->buffer+llen+2, pub->buffer+llen+3,tl); 		/* move topic inside buffer 1 byte to front */
                        pub->buffer[llen+2+tl] = 0; 								/* end the topic as a 'C' string with \x00 */
                        char *topic = (char*) pub->buffer +llen +2;
                        // msgId only present for QOS>0
                        if ((pub->buffer[0]&0x06) == MQTTQOS1) 
                        {
                            msgId   = (pub->buffer[llen+3+tl]<<8)+pub->buffer[llen+3+tl+1];
                            payload =  pub->buffer+llen+3+tl+2;
                            pub->callback(topic, payload,len-llen-3-tl-2);

                            pub->buffer[0] = MQTTPUBACK;
                            pub->buffer[1] = 2;
                            pub->buffer[2] = (msgId >> 8);
                            pub->buffer[3] = (msgId & 0xFF);
                            write(pub, this.buffer,4);
                            pub->lastOutActivity = t;

                        }  else    {
                            payload = this.buffer +llen +3 +tl;
                            pub->callback(topic, payload, len-llen-3-tl);
                        }
                    }
#endif
                }
                else if (type == MQTTPINGREQ)
                {
                    pub->buffer[0] = MQTTPINGRESP;
                    pub->buffer[1] = 0;
                    write(pub, this.buffer, 2);
                }
                else if (type == MQTTPINGRESP)
                {
                    pub->pingOutstanding = false;
                }
            }
            else if (!connected())
            {
                // readPacket has closed the connection
                return false;
            }
        }
        return true;
    }
    return false;
}

//---------------------------------Ende Definition der Funktionen-------------------------------------------------------

#endif // USE_MQTT_SERVER
