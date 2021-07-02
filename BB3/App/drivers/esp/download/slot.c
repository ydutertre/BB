/*
 * slot.h
 *
 *  Created on: Apr 19, 2021
 *      Author: horinek
 */
//#define DEBUG_LEVEL DBG_DEBUG
#include "slot.h"

#include "drivers/psram.h"

download_slot_t * download_slot[DOWNLOAD_SLOT_NUMBER] = {NULL};
osSemaphoreId_t download_slot_access;

#define DOWNLOAD_TIMEOUT            (5 * 1000)

static void download_slot_free(uint8_t data_id)
{
    if (download_slot[data_id] == NULL)
        return;

    if (download_slot[data_id]->type == DOWNLOAD_SLOT_TYPE_FILE)
    {
        if (download_slot[data_id]->data != NULL)
        {
            f_close(&((download_slot_file_data_t *)download_slot[data_id]->data)->f);
            free(download_slot[data_id]->data);
        }
    }

    if (download_slot[data_id]->type == DOWNLOAD_SLOT_TYPE_PSRAM)
    {
        if (download_slot[data_id]->data != NULL)
            ps_free(download_slot[data_id]->data);
    }

    free(download_slot[data_id]);

    download_slot[data_id] = NULL;
}

static uint8_t download_slot_get_free()
{
    for (uint8_t i = 0; i < DOWNLOAD_SLOT_NUMBER; i++)
    {
        if (download_slot[i] == NULL)
            return i;
    }

    return DOWNLOAD_SLOT_NONE;
}

static inline void download_slot_lock()
{
    DBG("DS acquire");
    osSemaphoreAcquire(download_slot_access, WAIT_INF);
    DBG("DS lock");
}

static inline void download_slot_unlock()
{
    DBG("DS un-lock");
    osSemaphoreRelease(download_slot_access);
}

static download_slot_t * download_slot_get(uint8_t data_id)
{
    if (download_slot[data_id] != NULL)
    {
        if (!download_slot[data_id]->canceled)
        {
            return download_slot[data_id];
        }
        else
        {
            return NULL;
        }
    }
    else
    {
        return NULL;
    }
}

void download_slot_init()
{
    download_slot_access = osSemaphoreNew(1, 0, NULL);
    vQueueAddToRegistry(download_slot_access, "download_slot_access");
    download_slot_unlock();
}

void download_slot_reset()
{
    download_slot_lock();

    for (uint8_t i = 0; i < DOWNLOAD_SLOT_NUMBER; i++)
    {
        if (download_slot[i] != NULL)
        {
            download_slot[i]->cb(DOWNLOAD_SLOT_TIMEOUT, &download_slot[i]);

            download_slot_free(i);
        }
    }

    download_slot_unlock();
}

uint8_t download_slot_cancel(uint8_t data_id)
{
    if (download_slot[data_id] != NULL)
    {
        download_slot[data_id]->canceled = true;
    }
}

uint8_t download_slot_create(uint8_t type, download_slot_cb_t cb)
{
    download_slot_lock();

    uint8_t i = download_slot_get_free();

    ASSERT(cb != NULL);

    if (i == DOWNLOAD_SLOT_NONE)
    {
        cb(DOWNLOAD_SLOT_NO_SLOT, NULL);
    }
    else
    {
        download_slot_t * ds = (download_slot_t *)malloc(sizeof(download_slot_t));

        ds->cb = cb;
        ds->type = type;
        ds->size = 0;
        ds->pos = 0;
        ds->data = NULL;
        ds->timestamp = HAL_GetTick();
        ds->canceled = false;

        download_slot[i] = ds;
    }

    download_slot_unlock();
    return i;
}

void download_slot_process_info(proto_download_info_t * info)
{
    download_slot_lock();

    download_slot_t * ds = download_slot_get(info->end_point);

    switch(info->result)
    {
        case (PROTO_DOWNLOAD_OK):
        {
            if (ds != NULL)
            {
                ds->timestamp = HAL_GetTick();

                switch(ds->type)
                {
                    case(DOWNLOAD_SLOT_TYPE_PSRAM):
                        ds->size = info->size;
                        ds->pos = 0;
                        ds->data = ps_malloc(ds->size + 1);
                    break;

                    case(DOWNLOAD_SLOT_TYPE_FILE):
                    {
                        ds->size = info->size;
                        ds->pos = 0;

                        download_slot_file_data_t * data = (download_slot_file_data_t *) malloc(sizeof(download_slot_file_data_t));
                        get_tmp_filename(data->name);
                        f_open(&data->f, data->name, FA_WRITE | FA_CREATE_NEW);

                        ds->data = (uint8_t *)data;
                    }
                    break;
                }

            }
        }
        break;

        case (PROTO_DOWNLOAD_NOT_FOUND):
            ds->cb(DOWNLOAD_SLOT_NOT_FOUND, ds);
            download_slot_free(info->end_point);
        break;

        case (PROTO_DOWNLOAD_NO_CONNECTION):
            ds->cb(DOWNLOAD_SLOT_NO_CONNECTION, ds);
            download_slot_free(info->end_point);
        break;
    }

    download_slot_unlock();
}

void download_slot_process_data(uint8_t data_id, uint8_t * data, uint16_t len)
{
    download_slot_lock();

    download_slot_t * ds = download_slot_get(data_id);

    if (ds != NULL)
    {
        if (ds->pos + len > ds->size)
        {
            len = ds->size - ds->pos;
            WARN("Data are larger than expected!");
        }
        
        switch (ds->type)
        {
            case(DOWNLOAD_SLOT_TYPE_PSRAM):
                memcpy(ds->data + ds->pos, data, len);
            break;

            case(DOWNLOAD_SLOT_TYPE_FILE):
            {
                UINT bw;
                f_write(&((download_slot_file_data_t *)ds->data)->f, data, len, &bw);
                ASSERT(bw == len);
            }
            break;
        }

        ds->pos += len;
        DBG("file %u position %lu/%lu %u%%", data_id, ds->pos, ds->size, (ds->pos * 100)/ ds->size);
        ds->timestamp = HAL_GetTick();

        ds->cb(DOWNLOAD_SLOT_PROGRESS, ds);
        
        if (ds->size == ds->pos)
        {
            if (ds->type == DOWNLOAD_SLOT_TYPE_FILE)
                f_close(&((download_slot_file_data_t *)ds->data)->f);

            if (ds->type == DOWNLOAD_SLOT_TYPE_PSRAM)
                ((char *)ds->data)[ds->size] = 0;

            ds->cb(DOWNLOAD_SLOT_COMPLETE, ds);

            download_slot_free(data_id);
        }

    }
    else
    {
        WARN("Ignoring downloaded data with id %u", data_id);
    }

    download_slot_unlock();
}

void download_slot_step()
{
    static uint32_t next = 0;
    if (next > HAL_GetTick())
        return;
    next = HAL_GetTick() + 200;

    download_slot_lock();

    for (uint8_t i = 0; i < DOWNLOAD_SLOT_NUMBER; i++)
    {
        if (download_slot[i] != NULL)
        {
            if (download_slot[i]->canceled)
            {
                download_slot[i]->cb(DOWNLOAD_SLOT_CANCEL, &download_slot[i]);
                download_slot_free(i);
            }
            else if (download_slot[i]->timestamp + DOWNLOAD_TIMEOUT < HAL_GetTick())
            {
                download_slot[i]->cb(DOWNLOAD_SLOT_TIMEOUT, &download_slot[i]);
                download_slot_free(i);
            }

        }
    }

    download_slot_unlock();
}