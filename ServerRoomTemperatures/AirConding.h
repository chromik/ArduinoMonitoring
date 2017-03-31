#ifndef AIR_CONDING_H_
#define AIR_CONDING_H_


#define IR_SIGNAL_LENGTH 199
#define AIRCOND_MIN_TEMP 17
#define AIRCOND_MAX_TEMP 30


struct air_conditioning {
  char temp;
};

void air_cond_set_temperature(unsigned char id, char temp);
char air_cond_get_temperature(unsigned char id);
void air_cond_turn_off(unsigned char id);


#endif //AIR_CONDING_H_





