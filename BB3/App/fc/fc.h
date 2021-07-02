/*
 * fc.h
 *
 *  Created on: May 6, 2020
 *      Author: horinek
 */

#ifndef FC_FC_H_
#define FC_FC_H_

#include "common.h"
#include "drivers/nvm.h"
#include "drivers/esp/protocol_def.h"

//unit conversions
#define FC_METER_TO_FEET		(3.2808399)
#define FC_MPS_TO_100FPM        (1.96850394)    //100 feet per min (WTF?)


#define FC_KNOTS_TO_KPH		(1.852)				//Kilometers per hour
#define FC_KNOTS_TO_MPH		(1.15077945)		//Miles per hour
#define FC_KNOTS_TO_MPS		(0.51444444444)		//Meters per seconds
#define FC_MPS_TO_KPH		(3.6)				//Kilometers per hour
#define FC_MPS_TO_MPH		(2.23693629)		//Miles per hour
#define FC_MPS_TO_KNOTS		(1.94384449)		//Knots
#define FC_KM_TO_MILE		(0.621371)


#define GNSS_NUMBER_OF_SATS		32

#define GNSS_GPS				0
#define GNSS_GLONAS				1
#define GNSS_GALILEO			2

#define GNSS_MUL				10000000l

#define GNSS_SAT_SYSTEM_MASK	0b00000111
#define GNSS_SAT_GPS			0b00000000
#define GNSS_SAT_SBAS			0b00000001
#define GNSS_SAT_GALILEO		0b00000010
#define GNSS_SAT_BEIDOU			0b00000011
#define GNSS_SAT_IMES			0b00000100
#define GNSS_SAT_QZSS			0b00000101
#define GNSS_SAT_GLONASS		0b00000110

#define GNSS_SAT_USED			0b00001000

#define ESP_STATE_WIFI_CLIENT       0b00000001
#define ESP_STATE_WIFI_CONNECTED    0b00000010
#define ESP_STATE_WIFI_AP           0b00000100
#define ESP_STATE_WIFI_AP_CONNECTED 0b00001000
#define ESP_STATE_BT_ON             0b00010000
#define ESP_STATE_BT_AUDIO          0b00100000
#define ESP_STATE_BT_DATA           0b01000000

typedef struct
{
	uint8_t manufacturer_id;
	uint16_t user_id;
} fanet_addr_t;

#define NB_NUMBER_IN_MEMORY		50

//neighbor flag
//when flying
#define NB_AIRCRAFT_TYPE_MASK	0b00000111
#define NB_HAVE_TURNRATE		0b00001000

//when walking
#define NB_GROUND_TYPE_MASK		0b00001111

#define NB_ONLINE_TRACKING		0b00010000
#define NB_FRIEND				0b00100000
#define NB_IS_FLYING			0b01000000

#define NB_HAVE_POS				0b10000000

#define NB_NAME_LEN				19
#define NB_TOO_FAR				0xFFFF

typedef struct
{
    int32_t latitude;
    int32_t longitude;

    uint16_t alititude;
    uint16_t timestamp;
    uint16_t dist; //in m
    uint16_t max_dist;

    fanet_addr_t addr;
    uint8_t flags;

    uint8_t heading;
	char name[NB_NAME_LEN];

} neighbor_t;

typedef enum
{
	esp_off = 0,
	esp_starting,
	esp_normal,
    esp_external_auto,
    esp_external_manual,
} esp_mode_t;

typedef enum
{
    fc_dev_error = 0,
    fc_dev_init,
    fc_dev_sampling,
    fc_dev_ready,
    fc_device_not_calibrated,
    fc_dev_off,
} fc_device_status_t;

typedef enum
{
    flight_not_ready = 0,
    flight_wait_to_takeoff,
    flight_flight,
    flight_landed
} fc_flight_mode;

typedef void (* wifi_list_update_cb)(proto_wifi_scan_res_t *);

#define FC_POS_NO_GNSS	0b0000
#define FC_POS_GNSS_2D	0b0010
#define FC_POS_GNSS_3D	0b0100
#define FC_POS_HAVE_BARO	0b1000

#define FC_HISTORY_PERIOD	250
#define FC_HISTORY_TIME		30
#define FC_HISTORY_SIZE		((FC_HISTORY_TIME * 1000) / FC_HISTORY_PERIOD)

typedef struct
{
	int32_t lat;
	int32_t lon;

	int16_t baro_alt;
	int16_t gnss_alt; //above ellipsoid

	int16_t ground_hdg; //in deg
	int16_t ground_spd; //in cm/s

	int16_t vario; //in cm/s
	uint16_t flags;
} fc_pos_history_t;


typedef struct
{
	osSemaphoreId_t lock;

    struct
    {
        uint32_t start_time;
        uint32_t duration;

        int16_t start_alt;
        uint8_t mode;
        uint8_t _pad[1];
    } flight;

    struct
    {
        float altitude;
        uint32_t timestamp;
    } autostart;

	struct
	{
        uint32_t ttf; //[ms]

        int32_t latitude;   //*10^7
        int32_t longtitude; //*10^7
        float ground_speed; //[m/s]
        float heading;
        uint32_t utc_time;
        float altitude_above_ellipsiod; //[m]
        float altitude_above_msl; //[m]
        float horizontal_accuracy; //[m]
        float vertical_accuracy; //[m]

        fc_device_status_t status;
		uint8_t fix; //2 - 2D, 3 - 3D
		bool time_synced;
		uint8_t _pad[1];

		struct
		{
			uint8_t sat_total;
			uint8_t sat_used;
			uint8_t _pad[2];

			struct
			{
				uint8_t sat_id;
				int8_t elevation; // +/- 90
				uint8_t azimuth; //0-359 /2
				uint8_t snr;
				uint8_t flags;
			} sats[GNSS_NUMBER_OF_SATS];
		} sat_info;
	} gnss;

	struct
	{
        neighbor_t neighbor[NB_NUMBER_IN_MEMORY];
        fanet_addr_t addr;

        fc_device_status_t status;

		uint8_t neighbors_size;
		uint8_t neighbors_magic;

        char version[21];

	} fanet;

	struct
	{
        float pressure; //in Pa
        fc_device_status_t status;

        uint8_t retry_cnt;
        uint8_t _pad[2];
	} baro;

    struct
    {
        float pressure; //in Pa
        fc_device_status_t status;

        uint8_t _pad[3];
    } aux_baro;

	struct
	{
        char ssid[PROTO_WIFI_SSID_LEN];

        wifi_list_update_cb wifi_list_cb;

        uint8_t ip_ap[4];
        uint8_t ip_sta[4];

        uint8_t mac_ap[6];
        esp_mode_t mode;
        uint8_t state;

        uint8_t mac_sta[6];
        uint8_t _pad_1[2];

        uint8_t mac_bt[6];

        fc_device_status_t amp_status;
        fc_device_status_t server_status;
	} esp;

	struct
	{
	    imu_calibration_t calibration;
        vector_float_t acc;
        vector_float_t gyro;
        vector_float_t mag;
        quaternion_t quat;
        float acc_total;
        float acc_gravity_compensated;

        struct
        {
            vector_i16_t acc;
            vector_i16_t gyro;
            vector_i16_t mag;
        } raw;
	    fc_device_status_t status;
	    uint8_t _pad[1];
	} imu;

	struct
	{
        float altitude1;
        float altitude2;
        float pressure;
        float vario;
        float avg_vario;

        float azimuth;
        float azimuth_filtered;

        fc_device_status_t status;

        uint8_t _pad[3];
	} fused;

	struct
	{
		osTimerId timer;
        fc_pos_history_t * positions;

        uint16_t index;
        uint16_t size;
	} history;

} fc_t;

extern fc_t fc;

void fc_device_status(char * buff, fc_device_status_t status);

void fc_set_time_from_utc(uint32_t datetime);
uint64_t fc_get_utc_time();

void fc_init();
void fc_takeoff();
void fc_landing();
void fc_reset();

void fc_deinit();

float fc_alt_to_qnh(float alt, float pressure);
float fc_press_to_alt(float pressure, float qnh);
float fc_alt_to_press(float alt, float qnh);

void fc_manual_alt1_change(float val);

#endif /* FC_FC_H_ */