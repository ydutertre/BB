
#include "settings.h"
#include "gui/gui_list.h"

#include "drivers/esp/protocol.h"
#include "etc/format.h"
#include "gui/bluetooth.h"
#include "fc/fc.h"

#include "gui/dialog.h"

REGISTER_TASK_ILS(bluetooth,
	lv_obj_t * info;
);



static void bluetooth_unpair_cb(dialog_result_t res, void * data)
{
	if (res == dialog_res_yes)
	{
		protocol_send(PROTO_BT_UNPAIR, NULL, 0);
	}
}

static bool bluetooth_unpair(lv_obj_t * obj, lv_event_t event)
{
	if (event == LV_EVENT_CLICKED)
	{
		dialog_show(_("Unbind?"),
				_("Do you want to unpair from all paired devices?\n"),
				dialog_yes_no, bluetooth_unpair_cb);

		//supress default handler
		return false;
	}

	return true;
}

static bool bluetooth_help(lv_obj_t * obj, lv_event_t event)
{
    if (event == LV_EVENT_CLICKED)
    {
        char help[256];

        snprintf(help, sizeof(help),
                _("You need to be in this menu to enable pairing.\n\n'%s'\n if for Audio and Telemetry (classic bluetooth), pair via phone settings.\n\n'%s LE'\n is for Telemetry (Bluetooth low energy), pair via XCTrack or other application."),
                config_get_text(&config.device_name), config_get_text(&config.device_name));

        dialog_show(_("Help"), help, dialog_confirm, NULL);

        //supress default handler
        return false;
    }

    return true;
}

static lv_obj_t * bluetooth_init(lv_obj_t * par)
{
    help_set_base("Bluetooth");

    lv_obj_t * list = gui_list_create(par, _("Bluetooth"), &gui_settings, NULL);

	local-> info = gui_list_info_add_entry(list, "", 0, "");

	gui_list_auto_entry(list, _h("Enable"), &profile.bluetooth.enabled, NULL);
	gui_list_auto_entry(list, _h("A2DP Audio"), &profile.bluetooth.a2dp, NULL);
	gui_list_auto_entry(list, _h("SPP Telemetry"), &profile.bluetooth.spp, NULL);
	gui_list_auto_entry(list, _h("BLE Telemetry"), &profile.bluetooth.ble, NULL);
    gui_list_auto_entry(list, _h("Help"), CUSTOM_CB, bluetooth_help);
    gui_list_auto_entry(list, _h("Telemetry protocol"), &profile.bluetooth.protocol, NULL);
    gui_list_auto_entry(list, _h("Forward GNSS"), &profile.bluetooth.forward_gnss, NULL);
    gui_list_auto_entry(list, _h("Forward FANET"), &profile.bluetooth.forward_fanet, NULL);

	gui_list_auto_entry(list, _h("Device name"), &config.device_name, NULL);
	gui_list_auto_entry(list, _h("PIN code"), &config.bluetooth.pin, NULL);

	gui_list_auto_entry(list, _h("Unpair all"), CUSTOM_CB, bluetooth_unpair);


	bluetooth_discoverable(true);
	return list;
}

static void bluetooth_loop()
{
	if (fc.esp.state & ESP_STATE_BT_ON)
	{
 		gui_list_info_set_name(local->info, _("Now visible as"));
 		gui_list_info_set_value(local->info, config_get_text(&config.device_name));
	}
	else
	{
 		gui_list_info_set_name(local->info, _("Turn on Bluetoth"));
 		gui_list_info_set_value(local->info, _("to enable pairing"));
	}
}

static void bluetooth_stop()
{
	bluetooth_discoverable(false);
}

