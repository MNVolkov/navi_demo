/*
     Simple navigator demo for Amazfit Bip BipOS
	(C) Maxim Volkov  2020 <Maxim.N.Volkov@ya.ru>
	
	Простой навигатор, демонстрация использования функций геолокации Amazfit Bip
	
	v.1.0
	Описание программы:
	Главный экран навигатора содержит:
	+	показания текущих координат
	+	текущее время
	+	текущее давление (hPa, mmHg), с возможностью коррекции (TODO)
	
	TODO:
	-	высоту над уровнем моря (с возможностью калибровки)
	-	расстояние от точки старта и направление на неё (если она была назначена)
	-	текущая скорость передвижения (как получить???)
	
	Функции:
	+ После запуска приложения включается датчик давления и GPS (в прошивке они включаются вместе, как включить по отдельности я не нашел).
	+ Затем, после фиксации позиции, на экране отображаются текущие координаты в градусах.
	+ С главного экрана можно установить путевую точку нажатием на кнопку часов. 
	
	TODO:
	- Первая путевая точка будет также точкой старта. 
	- Все последующие точки будут помещены в список путевых точек.
	- Долгое нажатие кнопки завершает трек, устанавливается конечная точка трека. 
	- В память сохраняется только путевые точки, промежуточный трек не сохраняется для экономии памяти. 
	
	Для записи трека можно пользоваться используйте один из спортивных режимов, например бег, ходьба и т.д.
	
	В меню настроек (кнопка с шестиренкой в углу экрана) можно выполнить следующие действия:
	-	просмотреть список треков
	-	просмотреть один их треков
	-	удалить трек из списка
	-	продолжить запись трека из списка
	-	сохранить выбранные или все треки в лог файл часов в формате GPX для скачивания через LikeApp
	-	управлять режимом экономии батареи. В режиме экономии включение приемника GPS происходит только при установке путевой точки, при этом на повторный поиск спутников необходимо время.
	
*/

#include <libbip.h>
#include "main.h"

//	структура меню экрана - для каждого экрана своя
struct regmenu_ screen_data = {				
						8,							//	 current_screen == 8 && swipe_screen_active_number == 18 - для работы long_press 											
						18,							//	номер главного экрана, значение 0-255, для пользовательских окон лучше брать от 50 и выше
						0,							//	0
						dispatch_screen,			//	указатель на функцию обработки тача, свайпов, долгого нажатия кнопки
						key_press_screen, 			//	указатель на функцию обработки нажатия кнопки
						screen_job,					//	указатель на функцию-колбэк таймера 
						0,							//	0
						show_screen,				//	указатель на функцию отображения экрана
						0,							//	
						key_long_press				//	долгое нажатие кнопки
					};

//	структура меню экрана настроек
struct regmenu_ settings_screen_data = {				
						8,							//	 current_screen == 8 && swipe_screen_active_number == 18 - для работы long_press 											
						18,							//	номер главного экрана, значение 0-255, для пользовательских окон лучше брать от 50 и выше
						0,							//	0
						dispatch_settings_screen,	//	указатель на функцию обработки тача, свайпов, долгого нажатия кнопки
						key_press_settings_screen,	//	указатель на функцию обработки нажатия кнопки
						0,					//	указатель на функцию-колбэк таймера 
						0,							//	0
						show_settings_screen,		//	указатель на функцию отображения экрана
						0,							//	
						key_press_settings_screen,	//	долгое нажатие кнопки, тот же обработчик, что и просто при нажатии кнопки
					};
					
int main(int param0, char** argv){	//	здесь переменная argv не определена
	 (void) argv;
	show_screen((void*) param0);
	
}

void show_screen (void *param0){
struct app_data_** 	app_data_p = get_ptr_temp_buf_2(); 	//	указатель на указатель на данные экрана 
struct app_data_ *	app_data;					//	указатель на данные экрана


// проверка источника запуска процедуры
if ( (param0 == *app_data_p) ){ // возврат из оверлейного экрана (входящий звонок, уведомление, будильник, цель и т.д.) или из дочернего экрана

	app_data = *app_data_p;					//	указатель на данные необходимо сохранить для исключения 
											//	высвобождения памяти функцией reg_menu
	*app_data_p = NULL;						//	обнуляем указатель для передачи в функцию reg_menu	

	// 	создаем новый экран, при этом указатель temp_buf_2 был равен 0 и память не была высвобождена	
	reg_menu(&screen_data, 0); 				// 	menu_overlay=0
	
	*app_data_p = app_data;						//	восстанавливаем указатель на данные после функции reg_menu
	
	//   здесь проводим действия при возврате из оверлейного экрана: восстанавливаем данные и т.д.
	
	
} else { // если запуск функции произошел впервые т.е. из меню 
	
	// создаем экран (регистрируем в системе)
	reg_menu(&screen_data, 0);

	// выделяем необходимую память и размещаем в ней данные, (память по указателю, хранящемуся по адресу temp_buf_2 высвобождается автоматически функцией reg_menu другого экрана)
	*app_data_p = (struct app_data_ *)pvPortMalloc(sizeof(struct app_data_));
	app_data = *app_data_p;		//	указатель на данные
	
	// очистим память под данные
	_memclr(app_data, sizeof(struct app_data_));
	
	//	получим указатель на данные запущенного процесса структура Elf_proc_
	app_data->proc = get_proc_by_addr(main);
	
	// запомним адрес указателя на функцию в которую необходимо вернуться после завершения данного экрана
	if ( param0 && app_data->proc->elf_finish ) 			//	если указатель на возврат передан, то возвоащаемся на него
		app_data->ret_f = app_data->proc->elf_finish;
	else					//	если нет, то на циферблат
		app_data->ret_f = show_watchface;
	
	// здесь проводим действия которые необходимо если функция запущена впервые из меню: заполнение всех структур данных и т.д.
	
	// считывание записанных во флэш память опций
	read_options(&app_data->options);
	
	// включаем датчики GPS и давления
	if (app_data->options.save_bat_mode == SAVE_BAT_MODE_OFF){
		switch_gps_pressure_sensors(SENSOR_ENABLE);
		app_data->sensor_mode = SENSOR_ENABLE;
	}
	
	//	включение видимости кнопки установки путевой точки
	app_data->store_point_btn_visible = true;
	
	//	сброс призака отложенной установки путевой точки
	app_data->store_point_delayed = false;
		
	//	если запуск был из быстрого меню, не включать экран приветствия
  if ( get_left_side_menu_active() ) {
	  app_data->splash	=	0;	// выключаем экран приветствия
  } else {
	  app_data->splash	=	1;	// включаем экран приветствия
  }

}

// здесь выполняем отрисовку интерфейса, обновление (перенос в видеопамять) экрана выполнять не нужно


// экран гаснет после таймаута, загорается при нажатии на экран
set_display_state_value(8, 1);
//set_display_state_value(4, 1);	//	подсветка
set_display_state_value(2, 1);


// get_navi_data - можно не вызывать, т.к. app_data пустая, а данных навигации все-равно еще нет

draw_screen();

// ставим таймер вызова screen_job в мс в зависимости нужна ли заставка
set_update_period(1,  app_data->splash?DISPLAY_SPLASH_PERIOD:DISPLAY_UPDATE_PERIOD); // обновляем экран через DISPLAY_SPLASH_PERIOD если активен экран приветствия
}

void key_long_press(){
struct app_data_** 	app_data_p = get_ptr_temp_buf_2(); 	//	указатель на указатель на данные экрана 
struct app_data_ *	app_data = *app_data_p;				//	указатель на данные экрана

if (!shut_down_navi()){	//	завершающие операции (отключение датчиков, сохранение)
	
//	если запуск был из быстрого меню, при нажатии кнопки выходим на циферблат
	if ( get_left_side_menu_active() ) 		
		app_data->proc->ret_f = show_watchface;
	
	
	// вызываем функцию возврата (обычно это меню запуска), в качестве параметра указываем адрес функции нашего приложения
	show_menu_animate(app_data->ret_f, (unsigned int)show_screen, ANIMATE_RIGHT);	
}
};

void key_press_screen(){
//struct app_data_** 	app_data_p = get_ptr_temp_buf_2(); 	//	указатель на указатель на данные экрана 
//struct app_data_ *	app_data = *app_data_p;				//	указатель на данные экрана

put_track_point();
};

void screen_job(){
// при необходимости можно использовать данные экрана в этой функции
struct app_data_** 	app_data_p = get_ptr_temp_buf_2(); 	//	указатель на указатель на данные экрана 
struct app_data_ *	app_data = *app_data_p;				//	указатель на данные экрана

// делаем периодическое действие: анимация, увеличение счетчика, обновление экрана,
// отрисовку интерфейса, обновление (перенос в видеопамять) экрана выполнять нужно

// отключаем экран приветствия
if (app_data->splash) {
	app_data->splash = 0;
}

// получаем данные навигации
get_navi_data(&app_data->navi_data);

if (IS_NAVI_GPS_READY(app_data->navi_data.ready)){
	
	if (app_data->store_point_delayed){
		put_track_point();
		app_data->store_point_delayed = false;
		app_data->store_point_btn_visible = true;
		
		//	если включен режим экономии, необходимо выключить датчики
		if (app_data->options.save_bat_mode == SAVE_BAT_MODE_ON){
			switch_gps_pressure_sensors(SENSOR_DISABLE);
			app_data->sensor_mode = SENSOR_DISABLE;
		}	
	}
}

draw_screen();
repaint_screen_lines(0, 176);

set_update_period(1, DISPLAY_UPDATE_PERIOD); // обновляем экран через заданное время
}


int dispatch_screen (void *param){
struct app_data_** 	app_data_p = get_ptr_temp_buf_2(); 	//	указатель на указатель на данные экрана 
struct app_data_ *	app_data = *app_data_p;				//	указатель на данные экрана

// в случае отрисовки интерфейса, обновление (перенос в видеопамять) экрана выполнять нужно
app_data->last_tick = get_tick_count();	//	установим последнюю отметку времени активности на текущее время

struct gesture_ *gest = param;
int result = 0;

switch (gest->gesture){

	case GESTURE_CLICK: {	
	
		
		// при нажатии во время приветственного экрана закрываем его
		if (app_data->splash) {
			app_data->splash = 0;
			draw_screen();
			repaint_screen_lines(0, 176);
			return result;
		}

			int btn = BTN_NONE;
			
			if ( ( gest->touch_pos_y < 40 ) ) {	//	нажатие на верхнюю панель кнопок управления
								
				int btn_index = gest->touch_pos_x / (176/3);
				
				//определяем какая кнопка нажата
				if (btn_index == 0) 	//	левая кнопка
					btn = BTN_GPS_TOGGLE;
				else 
				if (btn_index == 1){	//	средняя кнопка		
					btn = BTN_TIME;
				}
				else 					//	то что осталось - правая кнопка
					btn = BTN_SETTINGS;
				
			}  else 
			if (gest->touch_pos_y < 88){								
					int btn_index = gest->touch_pos_x / (176/3);

					//определяем какая кнопка нажата
					if (btn_index == 0) {	//	левая кнопка
						//btn = BTN_NONE;
					}
					else 
					if (btn_index == 1){	//	средняя кнопка		
						//btn = BTN_NONE;
					}
					else {					//	то что осталось - правая кнопка
						btn = BTN_GPS_SET_POINT;
					}
				
			} else 
			if (gest->touch_pos_y < 140){								
					int btn_index = gest->touch_pos_x / (176/3);

					//определяем какая кнопка нажата
					if (btn_index == 0){ 	//	левая кнопка
						//btn = BTN_NONE;
					}
					else 
					if (btn_index == 1){	//	средняя кнопка		
						//btn = BTN_NONE;
					}
					else {					//	то что осталось - правая кнопка
						//btn = BTN_NONE;
					}
			} else {								
					int btn_index = gest->touch_pos_x / (176/3);

					//определяем какая кнопка нажата
					if (btn_index == 0) 	//	левая кнопка
						btn = BTN_PRESSURE_TOGGLE;
					else 
					if (btn_index == 1){	//	средняя кнопка		
						btn = BTN_PRESSURE_TOGGLE;
					}
					else {					//	то что осталось - правая кнопка
						//btn = BTN_NONE;
					}
			}  
	
			
			if (btn != BTN_NONE)
				vibrate(1,70,0);
			
			switch (btn){
				case BTN_GPS_TOGGLE:{
					if (app_data->sensor_mode != SENSOR_DISABLE){
						switch_gps_pressure_sensors(SENSOR_DISABLE);
						app_data->sensor_mode = SENSOR_DISABLE;
					} else {
						switch_gps_pressure_sensors(SENSOR_ENABLE);
						app_data->sensor_mode = SENSOR_ENABLE;
					}

					break;
				}
				
				case BTN_TIME:			{
					//	при нажатии на время
					break;
				}
				case BTN_SETTINGS:{
					//	при нажатии на настройки
					show_menu_animate(show_settings_screen, (int)app_data, ANIMATE_LEFT);
					return 0;
					break;
				}
				case BTN_GPS_SET_POINT:	{
					//	при нажатии на маршрутную точку
					
					// если не отображается кнопка установки путевой точки, обработка завершается
					if (!app_data->store_point_btn_visible){
						break;
					}
					

						//	установим признак отложенной установки путевой точки
						//	и отключаем видимость кнопки установки точки
						app_data->store_point_delayed = true;
						app_data->store_point_btn_visible = false;
	
						//	если включен режим экономии и не включены датчики, необходимо включить датчики
						if ( (app_data->options.save_bat_mode == SAVE_BAT_MODE_ON) || 
							 (app_data->sensor_mode == SENSOR_DISABLE)){
								
								switch_gps_pressure_sensors(SENSOR_ENABLE);
								app_data->sensor_mode = SENSOR_ENABLE;
							
						}
					
					
					break;
				}
				case BTN_PRESSURE_TOGGLE:	{
					//	при нажатии на нажпись давление
					switch (app_data->options.press_units){
						default:
						case PRESS_UNITS_HPA:{
							app_data->options.press_units = PRESS_UNITS_MMHG;
							break;
						}
						case PRESS_UNITS_MMHG:{
							app_data->options.press_units = PRESS_UNITS_HPA;
							break;
						}
					}
	
					break;
				}
				//	если нажатие вне тач зон
				default: break;
			}
		
		
		draw_screen();
		repaint_screen_lines(0, 176);
	
		break;
		};
		
		case GESTURE_SWIPE_RIGHT: 	//	свайп направо
		case GESTURE_SWIPE_LEFT: {	// справа налево
	
			if ( get_left_side_menu_active()){
					// в случае запуска через быстрое меню в proc->ret_f содержится dispatch_left_side_menu
					// и после отработки elf_finish (на который указывает app_data->ret_f, произойдет запуск dispatch_left_side_menu c
					// параметром структуры события тачскрина, содержащемся в app_data->proc->ret_param0
					
					// запускаем dispatch_left_side_menu с параметром param в результате произойдет запуск соответствующего бокового экрана
					// при этом произойдет выгрузка данных текущего приложения и его деактивация.
					void* show_f = get_ptr_show_menu_func();
					dispatch_left_side_menu(param);
										
					if ( get_ptr_show_menu_func() == show_f ){
						// если dispatch_left_side_menu отработал безуспешно (листать некуда) то в show_menu_func по прежнему будет 
						// содержаться наша функция show_screen, тогда просто игнорируем этот жест
					//	vibrate(1, 100, 100);
						return 0;
					}
					
					//	если dispatch_left_side_menu отработал, то завершаем наше приложение, т.к. данные экрана уже выгрузились
					// на этом этапе уже выполняется новый экран (тот куда свайпнули)
					Elf_proc_* proc = get_proc_by_addr(main);
					proc->ret_f = NULL;
					
					elf_finish(main);	//	выгрузить Elf из памяти
					return 0;
				} else { 	//	если запуск не из быстрого меню, обрабатываем свайпы по отдельности
					switch (gest->gesture){
						case GESTURE_SWIPE_RIGHT: {	//	свайп направо
							//	выход из приложения
							if (!shut_down_navi())
								return show_menu_animate(app_data->ret_f, (int)main, ANIMATE_RIGHT);
							break;
						}
						case GESTURE_SWIPE_LEFT: {	// справа налево
							//	действие при запуске из меню и дальнейший свайп влево
							//vibrate(2,50,70);
							break;
						}
					} // switch (gest->gesture)
				}

			break;
		};	//	case GESTURE_SWIPE_LEFT/RIGHT:
		
		case GESTURE_SWIPE_UP: {	// свайп вверх

		/*
			// действия при свайпе вверх
			if (app_data->theme < 1) 
				app_data->theme++;
			else 
				app_data->theme = 0;
			
			draw_screen();
			repaint_screen_lines(0, 176);
		*/
			break;
		};
		case GESTURE_SWIPE_DOWN: {	// свайп вниз
		
		/*
			// действия при свайпе вниз
			if (app_data->theme > 0) 
				app_data->theme--;
			else 
				app_data->theme = 1;
			
			draw_screen();
			repaint_screen_lines(0, 176);
		*/	
			break;
		};		
		default:{	// что-то пошло не так...
			
			break;
		};		
		
	}


	
//ставим таймер вызова screen_job в мс
set_update_period(1, 500);

return result;
};



// Отрисовка экрана 
void draw_screen(){
struct app_data_** 	app_data_p = get_ptr_temp_buf_2(); 	//	указатель на указатель на данные экрана 
struct app_data_ *	app_data = *app_data_p;				//	указатель на данные экрана
struct res_params_ res_params;							//	параметры графического реурса

char gps_state = SENSOR_GPS_DISABLED;					//	текущее состояние датчика

char lat[16]; 			//	текст широта  	N 012.345678_
char lon[16]; 			//	текст долгота 	E 012.345678_
char alt[16]; 			//	текст высота  	10000.00_м_
char prs[16]; 			//	текст давление	1123.45 гПа_
char tim[8]; 			//	текст время		12:34_


float pressure = 0;

char* hPa_en 	= "hPa";
char* mmHg_en 	= "mmHg";
char* hPa_ru 	= "гПа";
char* mmHg_ru 	= "ммРс";

char* press_unit_str = NULL;	//	переменная, содержащая указатель на строку единиц измерения давления


set_bg_color(COLOR_BLACK);
fill_screen_bg();
set_graph_callback_to_ram_1();
// подгружаем шрифты
load_font();
set_fg_color(COLOR_WHITE);

// на стадии запуска приложения отрисовываем приветственный экран
if (app_data->splash){

	//	получение параметров заставки
	int result = get_res_params(ELF_INDEX_SELF, RES_SPLASH, &res_params);
	if (result){
		text_out_center("Navigator Demo\nby Volkov Maxim\n2020", 88, 70);
		return;
	};
	
	//	отрисовка заставки
	show_elf_res_by_id(ELF_INDEX_SELF , RES_SPLASH, (176-res_params.width)/2, (176-res_params.height)/2 );
	return;
}

// определяем состояние GPS для отображения значка и данных
if (app_data->sensor_mode == SENSOR_ENABLE){	
	if (IS_NAVI_GPS_READY(app_data->navi_data.ready)){
		gps_state = SENSOR_GPS_FIXED;
	} else {
		gps_state = SENSOR_GPS_NOT_FIXED;
	}
} else {
	gps_state = SENSOR_GPS_DISABLED;
} 

//	отрисовка значка настроек
//	получение параметров ресурса
int result = get_res_params(ELF_INDEX_SELF, RES_SETTINGS, &res_params);
if (!result){
	//	отрисовка значка Настроек справа отступ на 5px
	show_elf_res_by_id(ELF_INDEX_SELF , RES_SETTINGS, 176-5-res_params.width, 5 );
};
	

switch (gps_state){
		default:
		case SENSOR_GPS_DISABLED:{
			//	отрисовка значка GPS
			show_elf_res_by_id(ELF_INDEX_SELF , RES_GPS_DISABLED, 5, 5 );
			
			_sprintf(lat, "---");
			_sprintf(lon, "---");
			_sprintf(alt, "---");
			
			break;
		}
		case SENSOR_GPS_NOT_FIXED:{
			//	отрисовка значка GPS
			show_elf_res_by_id(ELF_INDEX_SELF , RES_GPS_NOT_FIXED, 5, 5 );
			
			_sprintf(lat, "---");
			_sprintf(lon, "---");
			_sprintf(alt, "---");
			
			break;
		}
		case SENSOR_GPS_FIXED:{
			//	отрисовка значка GPS
			show_elf_res_by_id(ELF_INDEX_SELF , RES_GPS_FIXED, 5, 5 );
			
			_sprintf(lat, "%c%3d.%.6d",   (app_data->navi_data.ns)==NAVI_NORTH_HEMISPHERE?'N':'S',
									 (int) app_data->navi_data.latitude/3000000, 
									((int) app_data->navi_data.latitude%3000000)/3); 
			
			_sprintf(lon, "%c%3d.%.6d",   (app_data->navi_data.ew)==NAVI_WEST_HEMISPHERE?'W':'E',
									 (int) app_data->navi_data.longitude/3000000, 
									((int) app_data->navi_data.longitude%3000000)/3);
			
			_sprintf(alt, "%.1f",  	app_data->navi_data.altitude);
			
			
			break;
		}
		
}

//	отрисовка кнопки установки путевой точки
if (app_data->store_point_btn_visible){	
	//	отрисовка значка путевой точки
	//	получение параметров ресурса
	int result = get_res_params(ELF_INDEX_SELF, RES_GPS_ADDPOINT, &res_params);
	if (!result){
		//	отрисовка значка Настроек справа отступ на 5px
		show_elf_res_by_id(ELF_INDEX_SELF , RES_GPS_ADDPOINT, 176-10-res_params.width, 40 );
	};
};

//	расчет давления
if (IS_NAVI_PRESS_READY(app_data->navi_data.ready)){
		switch (app_data->options.press_units){
			default:
			case PRESS_UNITS_HPA:{
				
				// языковая поддержка
				switch (get_selected_locale()){
					case locale_ru_RU:{
						press_unit_str = hPa_ru;
						break;
					}
					case locale_it_IT:
					case locale_fr_FR:
					case locale_es_ES:
					case locale_el_GR:
					default:{
						press_unit_str = hPa_en;
						break;
						}
					}
				
				pressure =  (float)app_data->navi_data.pressure / (float)PRESS_TO_HPA;
				
				
					break;
				}
			
			case PRESS_UNITS_MMHG:{
						
				// языковая поддержка
				switch (get_selected_locale()){
					case locale_ru_RU:{
						press_unit_str = mmHg_ru;
						break;
					}
					case locale_it_IT:
					case locale_fr_FR:
					case locale_es_ES:
					case locale_el_GR:
					default:{
						press_unit_str = mmHg_en;
						break;
						}
					}
					
				// перевод давления в требуемые единыцы измерения
				pressure =  (float)app_data->navi_data.pressure /  (float)PRESS_TO_MMHG;
				// перевод давления в требуемые единыцы измерения
					break;
				}
}

	// применение поправочного коэффициента
	if ((int) app_data->options.press_cal){
		pressure *= app_data->options.press_cal;
	}

	_sprintf(prs, "%.2f %s", pressure, press_unit_str);
} else {
	_sprintf(prs, "---");
}


			

// отрисовка значений	
struct datetime_ dt;
get_current_date_time(&dt);
_sprintf(tim, "%02d%c%02d", dt.hour, (dt.sec%2==0)?58:32, dt.min);

set_fg_color(COLOR_WHITE);
text_out_center(tim, 88, 5);

int pos_x = 30;
int txt_height = get_text_height();

set_fg_color(COLOR_AQUA);
text_out("Координаты",  10, pos_x);
pos_x+=txt_height;

if (gps_state == SENSOR_GPS_DISABLED){
	set_fg_color(COLOR_BLUE);
} else {
	set_fg_color(COLOR_WHITE);
}

text_out(lat, 10, pos_x);
pos_x+=txt_height;

text_out(lon, 10, pos_x);
pos_x+=txt_height;

set_fg_color(COLOR_AQUA);
text_out("Высота (GPS)",  	10, pos_x);
pos_x+=txt_height;

if (gps_state == SENSOR_GPS_DISABLED){
	set_fg_color(COLOR_BLUE);
} else {
	set_fg_color(COLOR_WHITE);
}

text_out(alt, 10, pos_x);
pos_x+=txt_height;

set_fg_color(COLOR_AQUA);
text_out("Давление",    10, pos_x);
pos_x+=txt_height;

if (!IS_NAVI_PRESS_READY(app_data->navi_data.ready)){
	set_fg_color(COLOR_BLUE);
} else {
	set_fg_color(COLOR_WHITE);
}

text_out(prs, 10, pos_x);

};


int put_track_point(){
// при необходимости можно использовать данные экрана в этой функции
struct app_data_** 	app_data_p = get_ptr_temp_buf_2(); 	//	указатель на указатель на данные экрана 
struct app_data_ *	app_data = *app_data_p;				//	указатель на данные экрана// при необходимости можно использовать данные экрана в этой функции
	
struct datetime_ dt;

char msg[128];

app_data->last_tick = get_tick_count();

get_current_date_time(&dt);

// сохранение путевой точки
log_printf(5, "[%d] [Navi] Time: %02d.%02d.%04d %02d:%02d:%02d; Point: (%c%3d.%.6d; %c%3d.%.6d) Alt: %.1f\n",   
			app_data->last_tick,
			dt.day,		dt.month,	dt.year,
			dt.hour, 	dt.min, 	dt.sec,
			(app_data->navi_data.ns)==NAVI_NORTH_HEMISPHERE?'N':'S',
	   (int) app_data->navi_data.latitude/3000000, 
	  ((int) app_data->navi_data.latitude%3000000)/3,
			(app_data->navi_data.ew)==NAVI_WEST_HEMISPHERE?'W':'E',
	   (int) app_data->navi_data.longitude/3000000, 
	  ((int) app_data->navi_data.longitude%3000000)/3,
			 app_data->navi_data.altitude);

//	текст уведомления
_sprintf(msg, "Координаты %c%3d.%.6d; %c%3d.%.6d; высота: %.1f\nNType: %d", 
			(app_data->navi_data.ns)==NAVI_NORTH_HEMISPHERE?'N':'S',
	   (int) app_data->navi_data.latitude/3000000, 
	  ((int) app_data->navi_data.latitude%3000000)/3,
			(app_data->navi_data.ew)==NAVI_WEST_HEMISPHERE?'W':'E',
	   (int) app_data->navi_data.longitude/3000000, 
	  ((int) app_data->navi_data.longitude%3000000)/3,
			 app_data->navi_data.altitude, 
			 app_data->notif_type);

// создание уведомления и его отображение
create_and_show_notification(app_data->notif_type, "Путевая точка", msg, "Navigator");

// создание уведомления без его отображения
//add_notification(app_data->notif_type, get_current_timestamp(), "Путевая точка", msg, "Navigator");

// в исследовательстких целях изменим номер типа уведомления, в следующий раз уведомление будет создано с другим типом
app_data->notif_type++;

return 0;
}

int shut_down_navi(){
struct app_data_** 	app_data_p = get_ptr_temp_buf_2(); 	//	указатель на указатель на данные экрана 
struct app_data_ *	app_data = *app_data_p;				//	указатель на данные экрана


	switch_gps_pressure_sensors(SENSOR_DISABLE);	//	отключение датчиков
	app_data->sensor_mode = SENSOR_DISABLE;
	
	return 0;		//	0 - успешно, 1 - неуспешно
}

void show_settings_screen(void* param0){
struct app_data_** 	app_data_p = get_ptr_temp_buf_2(); 	//	указатель на указатель на данные экрана 
struct app_data_ *	app_data = *app_data_p;					//	указатель на данные экрана


// проверка источника запуска процедуры
if (param0 == *app_data_p) { // переход из основного экрана либо оверлейного экрана (уведомления и т.д.) с сохранением контекста

	app_data = *app_data_p;					//	указатель на данные необходимо сохранить для исключения 
											//	высвобождения памяти функцией reg_menu
	*app_data_p = NULL;						//	обнуляем указатель для передачи в функцию reg_menu	

	// 	создаем новый экран, при этом указатель temp_buf_2 был равен 0 и память не была высвобождена	
	reg_menu(&settings_screen_data, 0); 		// 	menu_overlay=0
	
	*app_data_p = app_data;						//	восстанавливаем указатель на данные после функции reg_menu
	
	//   здесь проводим действия при возврате из оверлейного экрана: восстанавливаем данные и т.д.
	
} 
else { 
	//show_screen(NULL);	//	если запуск не из основной программы, это не нормальная ситуация, провибрируем и запустим основной экран
	vibrate(4,100,100);
	return;
}
	
// Отрисовка меню настроек
	clear_menu(&app_data->menu);
	push_ret_f(&app_data->menu, show_screen);
	app_data->menu.show_func = show_settings_screen;

	switch (get_selected_locale()){
		case locale_ru_RU:{
			add_menu_item(	&app_data->menu, 				//	переменная, содержащая структуру меню
							"Ед. измерения\nдавления",		//	отображаемый текст пункта меню (не более 3 строк, 64 байта)
							show_pressure_units_screen,		//	функция, получающая управление при нажатии на пункт меню
							COLOR_WHITE, 					//	цвет текста пункта меню
							MENU_ITEM_STYLE_NORMAL);		//	отображаемый стиль пункта меню
							
			add_menu_item(	&app_data->menu, 
							"Экономия\nбатареи",	
							save_bat_mode_toggle_callback,	
							COLOR_WHITE, 
							(app_data->options.save_bat_mode == SAVE_BAT_MODE_OFF)? 
									MENU_ITEM_STYLE_TOGGLE_OFF:
									MENU_ITEM_STYLE_TOGGLE_ON);
			break;
		}
		default:{
			add_menu_item(	&app_data->menu,
							"Pressure units", 	
							show_pressure_units_screen,	
							COLOR_WHITE, 
							MENU_ITEM_STYLE_NORMAL);
							
			add_menu_item(	&app_data->menu,
							"Battery\nsave",	
							save_bat_mode_toggle_callback,	
							COLOR_WHITE, 
							(app_data->options.save_bat_mode == SAVE_BAT_MODE_OFF)? 
									MENU_ITEM_STYLE_TOGGLE_OFF:
									MENU_ITEM_STYLE_TOGGLE_ON);
			break;
		}
	}

draw_menu(&app_data->menu);
}

void key_press_settings_screen(){
	struct app_data_** 	app_data_p = get_ptr_temp_buf_2(); 	//	указатель на указатель на данные экрана 
	struct app_data_ *	app_data = *app_data_p;				//	указатель на данные экрана
	
	void* ret_f = pop_ret_f(&app_data->menu);

	if (!ret_f)
		ret_f = show_watchface;
			
	show_menu_animate(ret_f, (int)app_data, ANIMATE_RIGHT);
};


int dispatch_settings_screen (void *param){
	struct app_data_** 	app_data_p = get_ptr_temp_buf_2(); 	//	указатель на указатель на данные экрана 
	struct app_data_ *	app_data = *app_data_p;				//	указатель на данные экрана
		
	return dispatch_menu (&app_data->menu, param);
};



int show_pressure_units_screen(){
	struct app_data_** 	app_data_p = get_ptr_temp_buf_2(); 	//	указатель на указатель на данные экрана 
	struct app_data_ *	app_data = *app_data_p;				//	указатель на данные экрана
	
	// очистим меню
	clear_menu(&app_data->menu);
	
	log_printf(5, "[show_pressure_units_screen] show_func: 0x%X; ret_f[last]: 0x%X; menu_level: %d\n", 
					app_data->menu.show_func, app_data->menu.ret_f[app_data->menu.menu_level-1], app_data->menu.menu_level);
	
	
	int stored_pressure_units = app_data->options.press_units;
	
	// добавляем пункты меню
	switch (get_system_locale()){
		case locale_ru_RU:{
			add_menu_item(&app_data->menu, "гПа", 	pressure_units_select_callback,	COLOR_WHITE, (stored_pressure_units==PRESS_UNITS_HPA)?MENU_ITEM_STYLE_CHECKED:MENU_ITEM_STYLE_UNCHECKED);
			add_menu_item(&app_data->menu, "ммРс", 	pressure_units_select_callback,	COLOR_WHITE, (stored_pressure_units==PRESS_UNITS_MMHG)?MENU_ITEM_STYLE_CHECKED:MENU_ITEM_STYLE_UNCHECKED);
			break;
		}
		//case locale_fr_FR:		
		//case locale_it_IT:
		default:{
			add_menu_item(&app_data->menu, "hPa", 	pressure_units_select_callback,	COLOR_WHITE, (stored_pressure_units==PRESS_UNITS_HPA)?MENU_ITEM_STYLE_CHECKED:MENU_ITEM_STYLE_UNCHECKED);
			add_menu_item(&app_data->menu, "mmHg", 	pressure_units_select_callback,	COLOR_WHITE, (stored_pressure_units==PRESS_UNITS_MMHG)?MENU_ITEM_STYLE_CHECKED:MENU_ITEM_STYLE_UNCHECKED);
			break;
		}
	}

	draw_menu(&app_data->menu);
	
	return 0;
}

int pressure_units_select_callback(struct menu_struct *	menu, int index){
struct app_data_** 	app_data_p = get_ptr_temp_buf_2(); 	//	указатель на указатель на данные экрана 
struct app_data_ *	app_data = *app_data_p;				//	указатель на данные экрана
char pu; 	//	временное значение параметра press_units в флэш памяти
	
// изменим стиль всех пунктов на UNCHECKED
for (int i = 0; i<menu->item_count;i++){
	if (menu->items[i].item_style == MENU_ITEM_STYLE_CHECKED)
		menu->items[i].item_style =  MENU_ITEM_STYLE_UNCHECKED;
}

// стиль выбранного пункта на CHECKED
menu->items[index].item_style =  MENU_ITEM_STYLE_CHECKED;

draw_menu(menu);
repaint_screen_lines(0, 176);

switch (index){
	default:
	case 0:{	//	0 - первый пункт меню
		app_data->options.press_units = PRESS_UNITS_HPA;
		break;
	}
	case 1:{	//	1 - второй пункт меню
		app_data->options.press_units = PRESS_UNITS_MMHG;
		break;
	}
}

ELF_READ_OPTION(&pu, options_, press_units);

// запись опций во flash память
if (pu != app_data->options.press_units){
	//	если зеачение опции отличается, то запишем его в память
	ELF_WRITE_OPTION(	&app_data->options.press_units, 	//	указатель на значение переменной для записи
						options_, 							//	тип структуры данных 
						press_units							//	имя члена структуры данных
					);
}

return 0;
}


/***


***/
int save_bat_mode_toggle_callback(struct menu_struct *	menu, int index){
struct app_data_** 	app_data_p = get_ptr_temp_buf_2(); 	//	указатель на указатель на данные экрана 
struct app_data_ *	app_data = *app_data_p;				//	указатель на данные экрана
	
// сменим стиль выбранного пункта на противоположный
menu->items[index].item_style =  (menu->items[index].item_style == MENU_ITEM_STYLE_TOGGLE_OFF)	?
										MENU_ITEM_STYLE_TOGGLE_ON:
										MENU_ITEM_STYLE_TOGGLE_OFF;

// обновим меню на экране
draw_menu(menu);
repaint_screen_lines(0, 176);

// внесем данные в опции
app_data->options.save_bat_mode = (menu->items[index].item_style == MENU_ITEM_STYLE_TOGGLE_ON)?
										SAVE_BAT_MODE_ON:
										SAVE_BAT_MODE_OFF ;

// запись опций во flash память
ELF_WRITE_OPTION(&app_data->options.save_bat_mode, options_, save_bat_mode);
return 0;
}

int write_options(options_* options){
char* text;

options_ flash_opt;	//	копия опций во флэш памяти
do {

	int result = ElfReadSettings(ELF_INDEX_SELF, &flash_opt, 0, sizeof(options_));
	if (result < sizeof(options_)){
		text = "while read options";
		break;
	}
	
	//	если копия опций во флэшпамяти не отличается от текущей, то не записываем ничего, иначе производим запись
	if (_memcmp(options, &flash_opt, sizeof(options_))){
		ElfWriteSettings(ELF_INDEX_SELF, options, 0, sizeof(options_));
	}
	
	return 0;
} while (0);
	
// произошла ошибка
log_printf(5, "[write_options] Error ElfWriteSettings: %s\n", text);
vibrate(2, 100, 100);
return -1;
}

int read_options(options_* options){
char text[48];
int result; 

do {
	result = ElfGetSettingsSize(ELF_INDEX_SELF);
	
	if ( result < sizeof(options_)){
		_sprintf(text, "options len %d is too short, need %d", result, sizeof(options_));
		break;
	}
	
	result = ElfReadSettings(ELF_INDEX_SELF, options, 0, sizeof(options_));
	//if (_memcmp(&options->sig[0], "NAVI", 4)){
	if (*(int*)(&options->sig[0]) != 0x4956414E ){
		_sprintf(text, "signature %02x %02x %02x %02x incorrect", options->sig[0], options->sig[1], options->sig[2], options->sig[3]);
		//log_printf(5, "[read_options] Dump options just read from flash\n");
		dump_mem(options, sizeof(options_));
		
		// если сигнатура неверна обнуляем все данные, возвращаем указатель на пустые настройки
		_memclr(options, sizeof(options));
		_memcpy(&options->sig[0], "NAVI", 4);
		options->save_bat_mode = SAVE_BAT_MODE_OFF;
		options->press_units = PRESS_UNITS_HPA;
		options->press_cal = 1.0f;
		
		// записываем сформироанные опции в флэш память
		ElfWriteSettings(ELF_INDEX_SELF, options, 0, sizeof(options_));
		break;
	}
	
	return 0;
} while (0);
	
// произошла ошибка
log_printf(5, "[read_options] Error ElfReadSettings: %s\n", text);
vibrate(2, 100, 100);
return -1;

}