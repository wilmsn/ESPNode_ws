#ifndef _SWITCHDEV_H_
#define _SWITCHDEV_H_
/***************************************************************************************
 * Ein Objekt zur Verwaltung eines Gerätes mit folgenden Eigenschaften:
 * - HTML Steuerung als Schalter (typ = 1), Schieberegler (typ = 2), Farbeinsteller (typ = 3) 
 * - Statusverwaltung (ein/aus => typ 1), Intensität/Volumen => typ2, Farbwert => typ3
 * - Aktivierbare Zusatzfunktionen:
 * 
 * 
 * - Integrierte Funktionen zur Steuerung:
 *   - stat_json: Gibt alle relevanten Stati als JSON-Teilstring aus
 *   - init_json: Gibt alle initialisierungs Stati als JSON-Teilstring aus
 * 
 ***************************************************************************************/
#include <stdlib.h>
#include <string.h>
#include "Arduino.h"
#define VARLEN 8

class Switchdev {
public:
    /**********************************
     * Initialisierung:
     * lfdnr: Laufende Nummer dieses Devices (1...4) MUSS VON 1 AUFSTEIGEND VERGEBEN WERDEN !!!
     * typ: 1: nur Schalter, 2: Volumeregler, 3: RGB Regler
     */
    Switchdev(uint8_t lfdnr, uint8_t typ);
    /* 
     *  Hier wird das Label für den Schalter übergeben
     */
    void begin(char*);

    /*
     * TRUE if something on the settings is changed; FALSE if not
     */
    bool changed();
     
    /*
     *  Erstellt ein Teil-JSON zur initialen Konfiguration der Webseite
     */
    char* web_json(void);

    /*
     *  Erstellt ein Teil-JSON zur aktuellen Status-Konfiguration
     */
    char* stat_json(bool);

    void set_state(uint8_t);
    void set_vol(uint8_t);
    void set_R(uint8_t);
    void set_G(uint8_t);
    void set_B(uint8_t);
    void set_sw_mqtt(char*);
    void set_vol_mqtt(char*);
    void set_R_mqtt(char*);
    void set_G_mqtt(char*);
    void set_B_mqtt(char*);

    bool get_state();
    uint8_t get_typ();
    uint8_t get_vol();
    uint8_t get_R();
    uint8_t get_G();
    uint8_t get_B();
        
private:
    uint8_t sw_no;
    uint8_t sw_typ;
    bool    sw_state;
    uint8_t sw_vol;
    uint8_t sw_R;
    uint8_t sw_G;
    uint8_t sw_B;
    bool    sw_changed;
    char*   label;
    char*   sw_mqtt;
    char*   sl0_mqtt;
    char*   sl1_mqtt;
    char*   sl2_mqtt;
    char*   sl3_mqtt;
    char*   json;
    size_t  json_len;
};



#endif
