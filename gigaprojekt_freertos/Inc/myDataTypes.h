/*
 * Struktura odpowiedzilna za przekazywanie odebranej ramki do w¹tku ComRxTask
 */
struct msg_part_order {
	char slash1;
	char hundred_digit_A;
	char ten_digit_A;
	char unit_digit_A;

	char slash2;
	char hundred_digit_B;
	char ten_digit_B;
	char unit_digit_B;

	char slash3;
	char hundred_digit_C;
	char ten_digit_C;
	char unit_digit_C;

	char slash4;
	char end_of_frame;
};
struct msg_part_command {
	char slash1;
	char command;
	char slash2;
	char end_of_frame;
};

typedef struct {
	char m_start;
	char m_type;
	union{
		char echo_data[UART_FRAME_LENGTH-2];
		struct msg_part_order order;
		struct msg_part_command command;
	};
}message_recive_frame_type;



/*
 * Definicje numerów sygna³ów steruj¹cych, oraz struktura komendy nowego zamówienia
 */

#define START_SIGNAL 0x1
#define PAUSE_SIGNAL 0x2
#define RESET_SIGNAL 0x4
#define TICK_SIGNAL 0x5

struct order_type{
	uint16_t quantity_A;
	uint16_t quantity_B;
	uint16_t quantity_C;
};
typedef struct order_type new_order_type;

/*
 * Definicje sygna³ów do procesu ComTxTask oraz struktura wiadomoœci przekazywanej do procesu
 */
#define BAD_FRAME_SIGNAL 0X1
#define ECHO_SIGNAL 0X2
#define MAIL_SIGNAL 0x3

enum frame_type {
	ORDER_COMPLETE = 'Q',
	EMPTY_CONTAINER = 'E',
	SENSOR_VALUE = 'S'
};

#include <stdbool.h>

struct msg_empty_container {
	bool A;
	bool B;
	bool C;
};

enum command_type {
	PAUSE = 'w',
	START = 's',
	RESTART = 'r'
};

struct sensor_value {
	uint8_t id;
	bool value;
};

typedef struct {
	enum frame_type type;
	union {
		struct order_type order_complete;
		struct msg_empty_container empty_container;
		enum command_type command_type;
		struct sensor_value sensor_change;
	};
}message_send_frame_type;

