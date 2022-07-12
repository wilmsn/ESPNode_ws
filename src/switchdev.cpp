#include "switchdev.h"

Switchdev::Switchdev(uint8_t _lfdnr, uint8_t _typ) {
  sw_no = _lfdnr;
  sw_typ = _typ;
  sw_mqtt = (char*)malloc(VARLEN);
  if ( sw_typ == 1 ) {
    sprintf(sw_mqtt,"sw%u",sw_no);
    json_len = 30;
  }
  if ( sw_typ == 2 ) {
    sl0_mqtt = (char*)malloc(VARLEN);
    snprintf(sl0_mqtt,VARLEN-1,"sw%usl0",sw_no);  
    json_len = 50;
  }
  if ( sw_typ == 3 ) {
    sl1_mqtt = (char*)malloc(VARLEN);
    snprintf(sl1_mqtt,VARLEN-1,"sw%usl1",sw_no);  
    sl2_mqtt = (char*)malloc(VARLEN);
    snprintf(sl2_mqtt,VARLEN-1,"sw%usl2",sw_no);  
    sl3_mqtt = (char*)malloc(VARLEN);
    snprintf(sl3_mqtt,VARLEN-1,"sw%usl3",sw_no);  
    json_len = 70;
  }
  json = (char*)malloc(json_len);
}

void Switchdev::begin(char* _label) {
  sw_state = false;
  label = (char*)malloc(strlen(_label)+1);
  sprintf(label,"%s",_label);
}

bool Switchdev::changed() {
  bool retval = sw_changed;
  sw_changed = false;
  return retval;
}

char* Switchdev::stat_json(bool custom) {
  switch (sw_typ) {
    case 1:
      if (custom) {
        snprintf(json, json_len, "\"%s\": %s", sw_mqtt, sw_state? "1":"0");
      } else {
        snprintf(json, json_len, "\"sw%u\": %s", sw_no, sw_state? "1":"0");
      }
    break;
    case 2:
      if (custom) {
        snprintf(json, json_len, "\"%s\":\"%s\",\"%s\":%u", sw_mqtt, sw_state? "1":"0", sl0_mqtt, sw_vol);
      } else {
        snprintf(json, json_len, "\"sw%u\":\"%s\",\"sw%u\sl0\":%u", sw_no, sw_state? "1":"0", sw_no, sw_vol);
      }
    break;
    case 3:
      if (custom) {
        snprintf(json, json_len, "\"%s\":%s,\"%s\":%u,\"%s\":%u,\"%s\":%u", sw_mqtt, sw_state? "1":"0", sl1_mqtt, sw_R, sl2_mqtt, sw_G, sl3_mqtt, sw_B);
      } else {
        snprintf(json, json_len, "\"sw%u\":%s,\"sw%usl1\":%u,\"sw%usl2\":%u,\"sw%usl3\":%u", sw_no, sw_state? "1":"0", sw_no, sw_R, sw_no, sw_G, sw_no, sw_B);
      }
    break;
  }
  return json;
}

char* Switchdev::web_json(void) {
  snprintf(json, json_len, "\"init%u\":%u,\"sw%ulab\":\"%s\"", sw_no, sw_typ, sw_no, label);  
  return json;
}

void Switchdev::set_state(uint8_t _state) {
  switch (_state) {
    case 0:
      sw_state = false;
    break;
    case 1:
      sw_state = true;
    break;
    case 2:
      sw_state = ! sw_state;
    break;
  }
  sw_changed = true;
}

void Switchdev::set_vol(uint8_t _vol) {
  sw_vol = _vol;
  sw_changed = true;
}

void Switchdev::set_R(uint8_t _r) {
  sw_R = _r;
  sw_changed = true;
}

void Switchdev::set_G(uint8_t _g) {
  sw_G = _g;
  sw_changed = true;
}

void Switchdev::set_B( uint8_t _b) {
  sw_B = _b;
  sw_changed = true;
}

bool Switchdev::get_state() {
  return sw_state;
}

uint8_t Switchdev::get_typ() {
  return sw_typ;  
}

uint8_t Switchdev::get_vol() {
  return sw_vol;  
}

uint8_t Switchdev::get_R() {
  return sw_R;
}

uint8_t Switchdev::get_G() {
  return sw_G;
}

uint8_t Switchdev::get_B() {
  return sw_B;
}

void Switchdev::set_sw_mqtt(char* _sw_mqtt) {
  if (strlen(_sw_mqtt)+1 > VARLEN) {
    free(sw_mqtt);
    sw_mqtt = (char*)malloc(strlen(_sw_mqtt)+1);
  }
  sprintf(sw_mqtt,"%s",_sw_mqtt);
}

void Switchdev::set_vol_mqtt(char* _sl0_mqtt) {
  if (strlen(_sl0_mqtt)+1 > VARLEN) {
    free(sl0_mqtt);
    sl0_mqtt = (char*)malloc(strlen(_sl0_mqtt)+1);
  }
  sprintf(sl0_mqtt,"%s",_sl0_mqtt);
}

void Switchdev::set_R_mqtt(char* _sl1_mqtt) {
  if (strlen(_sl1_mqtt)+1 > VARLEN) {
    free(sl1_mqtt);
    sl1_mqtt = (char*)malloc(strlen(_sl1_mqtt)+1);
  }
  sprintf(sl1_mqtt,"%s",_sl1_mqtt);
}

void Switchdev::set_G_mqtt(char* _sl2_mqtt) {
  if (strlen(_sl2_mqtt)+1 > VARLEN) {
    free(sl2_mqtt);
    sl2_mqtt = (char*)malloc(strlen(_sl2_mqtt)+1);
  }
  sprintf(sl2_mqtt,"%s",_sl2_mqtt);
}

void Switchdev::set_B_mqtt(char* _sl3_mqtt) {
  if (strlen(_sl3_mqtt)+1 > VARLEN) {
    free(sl3_mqtt);
    sl3_mqtt = (char*)malloc(strlen(_sl3_mqtt)+1);
  }
  sprintf(sl3_mqtt,"%s",_sl3_mqtt);
}
