#ifndef DISPLAY_H_
#define DISPLAY_H_

bool screen_show_proud(struct str_obrazovka *o);
void screen_show_curr_limits(struct str_obrazovka *o);
struct str_obrazovka * create_screen_proudove_limity_glob(void);
bool screen_show(struct str_obrazovka *o);
void lcd_setup(void);
struct str_obrazovka * create_current_screens(int pocet);
struct str_obrazovka *create_menu_screen(void);
struct str_obrazovka * create_screen_set_lcd(void);
struct str_obrazovka * create_screen_set_eth(void);
struct str_obrazovka * create_screen_set_time(void);
struct str_obrazovka * create_screen_set_time_manual(void);
struct str_obrazovka * create_screen_edit_address(int cislo, char **ip_array);
struct str_obrazovka * create_screen_factory_reset(void);

#endif
