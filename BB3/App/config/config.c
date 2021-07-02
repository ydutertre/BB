/*
 * config.cc
 *
 *  Created on: May 4, 2020
 *      Author: horinek
 */
#include "config.h"
#include "gui/widgets/widgets.h"

#include "fatfs.h"


void config_set_bool(cfg_entry_t * entry, bool val)
{
	entry->value.b = val;
    config_process_cb(entry);
}

bool config_get_bool(cfg_entry_t * entry)
{
	return entry->value.b;
}

void config_set_select(cfg_entry_t * entry, uint8_t val)
{
	cfg_entry_param_select_t * s;
	for(uint8_t i = 0; ; i++)
	{
		s = &entry->params.list[i];

		if (s->value == SELECT_END_VALUE)
		{
			//not found set first
			entry->value.u8[0] = entry->params.list[0].value;

			break;
		}

		if (s->value == val)
		{
			entry->value.u8[0] = val;
			break;
		}
	}

    config_process_cb(entry);
}

uint8_t config_get_select(cfg_entry_t * entry)
{
	return entry->value.u8[0];
}


const char * config_get_select_text(cfg_entry_t * entry)
{
    return entry->params.list[entry->value.u8[0]].name_id;
}


char * config_get_text(cfg_entry_t * entry)
{
	return entry->value.str;
}

void config_set_text(cfg_entry_t * entry, char * value)
{
    strncpy(entry->value.str, value, entry->params.u16[0]);
    entry->value.str[entry->params.u16[0] + 1] = 0;
    config_process_cb(entry);
}

uint16_t config_text_max_len(cfg_entry_t * entry)
{
	return entry->params.u16[0];
}

int16_t config_get_int(cfg_entry_t * entry)
{
	return entry->value.s16[0];
}

void config_set_int(cfg_entry_t * entry, int16_t value)
{
    //clip min - max
    if (value < entry->params.s16[0])
        value = entry->params.s16[0];

    if (value > entry->params.s16[1])
        value = entry->params.s16[1];

    entry->value.s16[0] = value;
    config_process_cb(entry);
}


int16_t config_int_max(cfg_entry_t * entry)
{
	return entry->params.s16[1];
}

int16_t config_int_min(cfg_entry_t * entry)
{
	return entry->params.s16[0];
}



int32_t config_get_big_int(cfg_entry_t * entry)
{
    return entry->value.s32;
}

void config_set_big_int(cfg_entry_t * entry, int32_t value)
{
    //clip
    if (entry->params.range != NULL)
    {
        if (value > entry->params.range->val_max.s32)
            value = entry->params.range->val_max.s32;

        if (value < entry->params.range->val_min.s32)
            value = entry->params.range->val_min.s32;
    }

    entry->value.s32 = value;
    config_process_cb(entry);
}

float config_get_float(cfg_entry_t * entry)
{
    return entry->value.flt;
}

void config_set_float(cfg_entry_t * entry, float value)
{
    //clip
    if (entry->params.range != NULL)
    {
        if (value > entry->params.range->val_max.flt)
            value = entry->params.range->val_max.flt;

        if (value < entry->params.range->val_min.flt)
            value = entry->params.range->val_min.flt;
    }

    entry->value.flt = value;
    config_process_cb(entry);
}

float config_float_max(cfg_entry_t * entry)
{
	return entry->params.range->val_max.flt;
}

float config_float_min(cfg_entry_t * entry)
{
	return entry->params.range->val_min.flt;
}


uint16_t config_structure_size(cfg_entry_t * structure)
{
    if (structure == (cfg_entry_t *)&config)
        return sizeof(config_t) / sizeof(cfg_entry_t);

    if (structure == (cfg_entry_t *)&profile)
        return sizeof(flight_profile_t) / sizeof(cfg_entry_t);

    if (structure == (cfg_entry_t *)&pilot)
        return sizeof(pilot_profile_t) / sizeof(cfg_entry_t);

    ASSERT(0);
    return 0;
}

void config_load(cfg_entry_t * structure, char * path)
{
	FIL f;
	uint8_t ret;

	config_disable_callbacks();

	ret = f_open(&f, path, FA_READ);
	INFO("Reading configuration from %s", path);

	if (ret == FR_OK)
	{
		char buff[256];
		uint16_t line = 0;

		while (f_gets(buff, sizeof(buff), &f) != NULL)
		{
			line++;

			//remove \n
			buff[strlen(buff) - 1] = 0;

			char * pos = strchr(buff, '=');
			if (pos == NULL)
			{
				WARN("line %u: invalid format '%s'", line, buff);
				continue;
			}

			char key[64];
			memcpy(key, buff, pos - buff);
			key[pos - buff] = 0;

			cfg_entry_t * e = entry_find(key, structure);
			if (e != NULL)
			{
				entry_set_str(e, pos + 1);
			}
			else
			{
				WARN("line %u: key '%s' not known", line, key);
			}
		}

		f_close(&f);
	}
	else
	{
		WARN("Unable to open");
	}
	INFO("Reading configuration done.");

	config_enable_callbacks();

	config_trigger_callbacks(structure);
}


void config_store(cfg_entry_t * structure, char * path)
{
	FIL f;
	uint8_t ret;

	ret = f_open(&f, path, FA_WRITE | FA_CREATE_ALWAYS);
	INFO("Writing configuration to %s", path);

	if (ret != FR_OK)
	{
		INFO("Unable to open file %s", path);
		return;
	}

	for (uint16_t i = 0; i < config_structure_size(structure); i++)
	{
		char buff[256];

		entry_get_str(buff, &structure[i]);
		UINT bw;
		f_write(&f, buff, strlen(buff), &bw);

	}

	f_close(&f);
}

void config_show(cfg_entry_t * structure)
{
	INFO("Configuration");

	for (uint16_t i = 0; i < config_structure_size(structure); i++)
	{
		char buff[256];

		entry_get_str(buff, &structure[i]);
		buff[strlen(buff) - 1] = 0;
		INFO("%s", buff);
	}
}

void config_load_all()
{
    config_init((cfg_entry_t *)&config);
    config_load((cfg_entry_t *)&config, PATH_DEVICE_CFG);
    pages_defragment();

    char path[64];

    sprintf(path, "%s/%s.cfg", PATH_PROFILE_DIR, config_get_text(&config.flight_profile));
    config_init((cfg_entry_t *)&profile);
    config_load((cfg_entry_t *)&profile, path);

    sprintf(path, "%s/%s.cfg", PATH_PILOT_DIR, config_get_text(&config.pilot_profile));
    config_init((cfg_entry_t *)&pilot);
    config_load((cfg_entry_t *)&pilot, path);
}


void config_store_all()
{
    char path[64];

    config_store(&config, PATH_DEVICE_CFG);
    sprintf(path, "%s/%s.cfg", PATH_PROFILE_DIR, config_get_text(&config.flight_profile));
    config_store(&profile, path);
    sprintf(path, "%s/%s.cfg", PATH_PILOT_DIR, config_get_text(&config.pilot_profile));
    config_store(&pilot, path);
}

