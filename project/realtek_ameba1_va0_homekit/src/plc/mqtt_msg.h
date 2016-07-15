#ifndef MQTT_MSG_H
#include "config.h"
#include "basic_types.h"

#define	MQTT_MSG_H

#define PROTOCOL_NAMEv311

#define MQTT_MAX_FIXED_HEADER_SIZE 3

enum mqtt_connect_flag
{
	MQTT_CONNECT_FLAG_USERNAME = (1 << 7),
	MQTT_CONNECT_FLAG_PASSWORD = (1 << 6),
	MQTT_CONNECT_FLAG_WILL_RETAIN = (1 << 5),
	MQTT_CONNECT_FLAG_WILL = (1 << 2),
	MQTT_CONNECT_FLAG_CLEAN_SESSION = (1 << 1),
};

typedef enum mqtt_message_type
{
	MQTT_MSG_TYPE_CONNECT = 1,
	MQTT_MSG_TYPE_CONNACK = 2,
	MQTT_MSG_TYPE_PUBLISH = 3,
	MQTT_MSG_TYPE_PUBACK = 4,
	MQTT_MSG_TYPE_PUBREC = 5,
	MQTT_MSG_TYPE_PUBREL = 6,
	MQTT_MSG_TYPE_PUBCOMP = 7,
	MQTT_MSG_TYPE_SUBSCRIBE = 8,
	MQTT_MSG_TYPE_SUBACK = 9,
	MQTT_MSG_TYPE_UNSUBSCRIBE = 10,
	MQTT_MSG_TYPE_UNSUBACK = 11,
	MQTT_MSG_TYPE_PINGREQ = 12,
	MQTT_MSG_TYPE_PINGRESP = 13,
	MQTT_MSG_TYPE_DISCONNECT = 14
} mqtt_msg_type;

#pragma pack(1)
struct mqtt_connect_variable_header
{
	uint8_t lengthMsb;
	uint8_t lengthLsb;
#if defined(PROTOCOL_NAMEv31)
	uint8_t magic[6];
#elif defined(PROTOCOL_NAMEv311)
	uint8_t magic[4];
#else
#error "Please define protocol name"
#endif
	uint8_t version;
	uint8_t flags;
	uint8_t keepaliveMsb;
	uint8_t keepaliveLsb;
};
#pragma pack()

typedef struct mqtt_message
{
 uint8_t* data;
 uint16_t length;

} mqtt_message_t;

typedef struct mqtt_connection
{
 mqtt_message_t message;

 uint16_t message_id;
 uint8_t* buffer;
 uint16_t buffer_length;

} mqtt_connection_t;

typedef struct mqtt_connect_info
{
 char* client_id;
 char* username;
 char* password;
 char* will_topic;
 char* will_message;
 int keepalive;
 int will_qos;
 int will_retain;
 int clean_session;
} mqtt_connect_info_t;

void mqtt_msg_init(mqtt_connection_t* connection, uint8_t* buffer, uint16_t buffer_length);
inline int mqtt_get_type(uint8_t* buffer);
inline int mqtt_get_dup(uint8_t* buffer);
inline int mqtt_get_qos(uint8_t* buffer);
inline int mqtt_get_retain(uint8_t* buffer);
int mqtt_get_total_length(uint8_t* buffer, uint16_t length);
const char* mqtt_get_publish_topic(uint8_t* buffer, uint16_t* length);
const char* mqtt_get_publish_data(uint8_t* buffer, uint16_t* length);
uint16_t mqtt_get_id(uint8_t* buffer, uint16_t length);
bool mqtt_chg_id(uint8_t *buffer, uint16_t length, uint16_t id);

mqtt_message_t* mqtt_msg_connect(mqtt_connection_t* connection, mqtt_connect_info_t* info);
mqtt_message_t* mqtt_msg_publish(mqtt_connection_t* connection, const char* topic, const char* data, int data_length, int qos, int retain, uint16_t* message_id);
mqtt_message_t* mqtt_msg_puback(mqtt_connection_t* connection, uint16_t message_id);
mqtt_message_t* mqtt_msg_pubrec(mqtt_connection_t* connection, uint16_t message_id);
mqtt_message_t* mqtt_msg_pubrel(mqtt_connection_t* connection, uint16_t message_id);
mqtt_message_t* mqtt_msg_pubcomp(mqtt_connection_t* connection, uint16_t message_id);
mqtt_message_t* mqtt_msg_subscribe(mqtt_connection_t* connection, const char* topic, int qos, uint16_t* message_id);
mqtt_message_t* mqtt_msg_unsubscribe(mqtt_connection_t* connection, const char* topic, uint16_t* message_id);
mqtt_message_t* mqtt_msg_pingreq(mqtt_connection_t* connection);
mqtt_message_t* mqtt_msg_pingresp(mqtt_connection_t* connection);
mqtt_message_t* mqtt_msg_disconnect(mqtt_connection_t* connection);

#endif	/* MQTT_MSG_H */

