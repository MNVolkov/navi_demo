/*
	Simple navigation demo for Amazfit Bip BipOS
	(C) Maxim Volkov  2020 <Maxim.N.Volkov@ya.ru>
	
	

*/

#ifndef __APP_NAVI_H__
#define __APP_NAVI_H__

//	параметры отображения
#define DISPLAY_UPDATE_PERIOD	300		//	через каждые 100 мс обновление экрана
#define DISPLAY_SPLASH_PERIOD	2000	//	через 2000 мс убрать заставку

// режим датчиков
#define SENSOR_DISABLE			0
#define SENSOR_ENABLE			1

// состояние датчика
#define SENSOR_GPS_DISABLED		2
#define SENSOR_GPS_NOT_FIXED	3
#define SENSOR_GPS_FIXED		4

// графические ресурсы
#define RES_SPLASH				0
#define RES_GPS_DISABLED		1
#define RES_GPS_NOT_FIXED		2
#define RES_GPS_FIXED			3
#define RES_SETTINGS			4
#define RES_GPS_ADDPOINT		5

// кнопки и тач области экрана
#define BTN_NONE				0
#define BTN_GPS_TOGGLE			1
#define BTN_TIME				2
#define BTN_SETTINGS			3
#define BTN_GPS_SET_POINT		4
#define BTN_PRESSURE_TOGGLE		5
	
// единицы давления	
#define PRESS_UNITS_COUNT		2			//	количество опций
#define PRESS_UNITS_HPA			0			//	первая
#define PRESS_UNITS_MMHG		1			//	вторая

//	коэффициенты приведения из Паскалей
#define PRESS_TO_HPA			100			//	конвертирование давления в гектопаскали
#define PRESS_TO_MMHG			133.322F	//	конвертирование давления в мм ртутного столба

// варианты опций
#define SAVE_BAT_MODE_OFF	0
#define SAVE_BAT_MODE_ON	1

// определим макросы для упрощения доступа к элементам опций
#define ELF_WRITE_OPTION(o,s,m)	ElfWriteSettings(ELF_INDEX_SELF, o, offsetof(s,m), sizeof_member(s,m))
#define ELF_READ_OPTION(o,s,m)	ElfReadSettings (ELF_INDEX_SELF, o, offsetof(s,m), sizeof_member(s,m))

typedef struct {
	char sig[4];		//	сигнатура NAVI
	char save_bat_mode;	//	режим экономии батареи включен
	char press_units;	//	единицы измерения давления
	float press_cal;	//	калибровочный коэффициент давления
} options_;

// структура данных для нашего экрана
struct app_data_ {
			void* 				ret_f;			//	адрес функции возврата
			Elf_proc_* 			proc;			//	указатель на данные процесса
			char 				splash;			//	активный приветственный экран
			char				sensor_mode;	//	текущий режим сенсора
			char 				notif_type;		//	экспериментально тип уведомления
			char 	store_point_btn_visible;	//	видимость кнопки установки путевой точки
			char 	store_point_delayed;		//	флаг необходимости установить точку
			int 				last_tick;		//	метка данных времени последней отрисовки интерфейса
			options_			options;		//	настройки
			struct 	menu_struct	menu;			//	структура меню приложения
			navi_struct_		navi_data;		//	структура данных геолокации и давления
};

// main.c
void 	show_screen (void *return_screen);		//	функция запуска главного экрана приложения
void 	key_press_screen();						//	функция обработки долгого кнопки
void 	key_long_press();						//	функция обработки долгого нажатия кнопки
int 	dispatch_screen (void *param);			//	функция обработки событий сенсорного экрана
void 	screen_job();							//	функция обработки срабатывания экранного таймера приложения
void	draw_screen();							//	функция отрисовки основного экрана приложения
int		shut_down_navi();						//	отключение датчиков, сохранение перед выходом
int 	put_track_point();						//	действия по установке путевой точки
void 	show_settings_screen();					//	открывает экран настроек
void 	key_press_settings_screen();			//	функция обработки долгого кнопки
int 	dispatch_settings_screen (void *param);	//	функция обработки событий сенсорного экрана
int		show_pressure_units_screen();			//	функция отображения вариантов единиц измерения давления
int 	pressure_units_select_callback(struct menu_struct *	menu, int index);	//	функция обработки выбранного варианта чекбоксов
int		write_options(options_* options);//	функция записи опций
int 	read_options(options_* options);	//	функция чтения опций
int		save_bat_mode_toggle_callback(struct menu_struct *	menu, int index);	//	//	функция обработки выбранного варианта тогглов

#endif