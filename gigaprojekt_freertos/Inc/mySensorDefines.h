/*
 * Definicje ID czujników wystêpuj¹cych w automacie
 * Przez ID rozumie sie dowolny znak z podstawowej tablicy ASCII
 *
 * Czujnik_Poziomu_A1,B1,C1 - Czujnik wykrywaj¹cy cukierki tu¿ przy wylocie na t³ok, Gdy tam jest 0 zbiornik jest ca³kiem pusty
 *
 * Czujnik_Poziomu_A2,B2,C2	- Czujnik w po³owie zbiornika ostrzegaj¹cy o ma³ej iloœci cukierków
 *
 * Czujnik_Wypchniêcia_A,B,C - Czujnik który upewnia siê ¿e cukierek zosta³ wypchniêty
 *
 * Czujnik_Pozycji_A,B,C - Czujnik krañcowy sprawdzaj¹cy czy pude³ko na cukierki znajduje siê pod ssypem A,B lub C
 *
 * Czujnik_Pozycji_Koncowej	- Czujnik krañcowy sprawdzaj¹cy po³o¿enie koñca taœmoci¹gu sk¹d pude³ko odbiera operator
 *
 * Przycisk_Bezpieczeñstwa - Przycisk dla operatora, w razie jakiœ nag³ych sytuacji
 *
 * Przycisk_Gotowosci - Przycisk który operator uruchamia gdy za³o¿y pust¹ skrzyniê na pocz¹tek taœmoci¹gu
 *
 */

#define CZ_POZIOMU_A1 11
#define CZ_POZIOMU_B1 12
#define CZ_POZIOMU_C1 13

#define CZ_POZIOMU_A2 21
#define CZ_POZIOMU_B2 22
#define CZ_POZIOMU_C2 23

#define CZ_WYPCHNIECIA_A 31
#define CZ_WYPCHNIECIA_B 32
#define CZ_WYPCHNIECIA_C 33

#define CZ_POZYCJI_A 41
#define CZ_POZYCJI_B 42
#define CZ_POZYCJI_C 43

#define CZ_POZYCJI_KONCOWEJ 50

#define PRZYCISK_BEZPIECZENSTWA 0

#define PRZYCISK_GOTOWOSCI 1

#define NUMBER_OF_SENSORS 15

#define NUMBER_OF_OUTPUTS 7

#define TIMER_A 20
#define TIMER_B 20
#define TIMER_C 20

#include <stdbool.h>

typedef struct {
	bool value;
	bool prev_value;
	int id :7;
	GPIO_TypeDef *port;
	uint16_t pin;
}sensor;

typedef struct {
	bool value;
	GPIO_TypeDef *port;
	uint16_t pin;
}output;
