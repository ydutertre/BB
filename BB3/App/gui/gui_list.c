/*
 * gui_list.cc
 *
 *  Created on: May 18, 2020
 *      Author: horinek
 */
#include "gui_list.h"
#include "keyboard.h"

lv_obj_t * gui_list_get_entry(uint8_t index)
{
	lv_obj_t * child = NULL;
	while(1)
	{
	    child = lv_obj_get_child_back(gui.list.object, child);

	    if (index == 0)
	    	return child;

	    index--;

	    if (child == NULL)
	    	return NULL;
	}
}

void gui_list_event_cb(lv_obj_t * obj, lv_event_t event)
{

	if (gui.list.callback == NULL)
		return;

	//get top parent
	lv_obj_t * top = obj;

	while (top->parent != gui.list.object)
	{
		top = top->parent;
		if (top == NULL)
			return;
	}

	uint8_t index = 0;

	lv_obj_t * child = NULL;
	while (1)
	{
		child = lv_obj_get_child_back(gui.list.object, child);
		if (child == top)
			break;

		if (child == NULL)
			return;

		index++;
	}

	gui.list.callback(child, event, index);
}

lv_obj_t * gui_list_create(lv_obj_t * par, const char * title, gui_list_task_cb_t cb)
{
	lv_obj_t * win = lv_win_create(par, NULL);
	lv_win_set_title(win, title);
	lv_obj_set_size(win, LV_HOR_RES, LV_VER_RES - GUI_STATUSBAR_HEIGHT);

	gui_set_group_focus(lv_win_get_content(win));
	lv_win_set_layout(win, LV_LAYOUT_COLUMN_MID);

	//object that hold list entries
	gui.list.object = lv_obj_get_child(lv_win_get_content(win), NULL);
	gui.list.callback = cb;

	return win;
}

lv_obj_t * gui_list_text_add_entry(lv_obj_t * list, const char * text)
{
	lv_obj_t * entry = lv_cont_create(list, NULL);
	lv_obj_add_style(entry, LV_CONT_PART_MAIN, &gui.styles.list_select);
	lv_cont_set_fit2(entry, LV_FIT_PARENT, LV_FIT_TIGHT);
	lv_cont_set_layout(entry, LV_LAYOUT_COLUMN_LEFT);
	lv_page_glue_obj(entry, true);


	lv_obj_t * label = lv_label_create(entry, NULL);
	lv_label_set_text(label, text);

	lv_group_add_obj(gui.input.group, entry);

	lv_obj_set_event_cb(entry, gui_list_event_cb);

	return entry;
}

void gui_list_text_set_value(lv_obj_t * obj, char * text)
{
    lv_obj_t * label = lv_obj_get_child(obj, NULL);
    lv_label_set_text(label, text);
}



lv_obj_t * gui_list_slider_add_entry(lv_obj_t * list, const char * text, int16_t value_min, int16_t value_max, int16_t value)
{
	lv_obj_t * entry = lv_cont_create(list, NULL);
	lv_obj_add_style(entry, LV_CONT_PART_MAIN, &gui.styles.list_select);
	lv_cont_set_fit2(entry, LV_FIT_PARENT, LV_FIT_TIGHT);
	lv_cont_set_layout(entry, LV_LAYOUT_PRETTY_MID);
	lv_page_glue_obj(entry, true);

	uint16_t w = lv_obj_get_width_fit(entry);

	lv_obj_t * label = lv_label_create(entry, NULL);
	lv_label_set_text(label, text);
	lv_label_set_long_mode(label, LV_LABEL_LONG_SROLL_CIRC);
	lv_obj_set_width(label, w);

	lv_obj_t * slider = lv_slider_create(entry,  NULL);
	lv_group_add_obj(gui.input.group, slider);
	lv_obj_set_size(slider, w * 2 / 3, 14);
	lv_obj_set_focus_parent(slider, true);

	lv_slider_set_range(slider, value_min, value_max);
	lv_slider_set_value(slider, value, LV_ANIM_OFF);
	lv_slider_set_type(slider, LV_SLIDER_TYPE_SYMMETRICAL);

	lv_obj_t * val = lv_label_create(entry, NULL);
	lv_label_set_text(val, "");
	lv_label_set_align(val, LV_LABEL_ALIGN_CENTER);
	lv_obj_set_size(val, w / 3, 14);

	lv_obj_set_event_cb(slider, gui_list_event_cb);

	return entry;
}

int16_t gui_list_slider_get_value(lv_obj_t * obj)
{
	lv_obj_t * slider = lv_obj_get_child(obj, lv_obj_get_child(obj, NULL));
	return lv_slider_get_value(slider);
}

void gui_list_slider_set_label(lv_obj_t * obj, char * text)
{
	lv_obj_t * label = lv_obj_get_child(obj, NULL);
	lv_label_set_text(label, text);
}

lv_obj_t * gui_list_dropdown_add_entry(lv_obj_t * list, const char * text, const char * options, uint16_t selected)
{
	lv_obj_t * entry = lv_cont_create(list, NULL);
	lv_obj_add_style(entry, LV_CONT_PART_MAIN, &gui.styles.list_select);
	lv_cont_set_fit2(entry, LV_FIT_PARENT, LV_FIT_TIGHT);
	lv_cont_set_layout(entry, LV_LAYOUT_COLUMN_LEFT);
	lv_page_glue_obj(entry, true);

	lv_obj_t * label = lv_label_create(entry, NULL);
	lv_label_set_text(label, text);

	lv_obj_t * cont = lv_cont_create(entry, NULL);
	lv_cont_set_fit2(cont, LV_FIT_PARENT, LV_FIT_TIGHT);
	lv_cont_set_layout(cont, LV_LAYOUT_COLUMN_RIGHT);
	lv_obj_set_focus_parent(cont, true);

	lv_obj_t * dropdown = lv_dropdown_create(cont,  NULL);
	lv_dropdown_set_options(dropdown, options);
	lv_dropdown_set_selected(dropdown, selected);
	lv_group_add_obj(gui.input.group, dropdown);
	lv_obj_set_width(dropdown, 140);
	lv_obj_set_focus_parent(dropdown, true);

	lv_obj_set_event_cb(dropdown, gui_list_event_cb);

	return entry;
}

uint16_t gui_list_dropdown_get_value(lv_obj_t * obj)
{
    //dropdown widgt
    lv_obj_t * dd = lv_obj_get_child(lv_obj_get_child(obj, NULL), NULL);
    return lv_dropdown_get_selected(dd);
}

bool gui_list_switch_get_value(lv_obj_t * obj)
{
	//switch widget is last added child
	lv_obj_t * sw = lv_obj_get_child(obj, NULL);
	return lv_switch_get_state(sw);
}

void gui_list_switch_set_value(lv_obj_t * obj, bool val)
{
    //switch widget is last added child
    lv_obj_t * sw = lv_obj_get_child(obj, NULL);
    if (val)
        lv_switch_on(sw, LV_ANIM_ON);
    else
        lv_switch_off(sw, LV_ANIM_ON);
}

lv_obj_t * gui_list_switch_add_entry(lv_obj_t * list, const char * text, bool value)
{
	lv_obj_t * entry = lv_cont_create(list, NULL);
	lv_obj_add_style(entry, LV_CONT_PART_MAIN, &gui.styles.list_select);
	lv_cont_set_fit2(entry, LV_FIT_PARENT, LV_FIT_TIGHT);
	lv_page_glue_obj(entry, true);

	lv_obj_t * label = lv_label_create(entry, NULL);
	lv_label_set_text(label, text);
	lv_obj_align(label, entry, LV_ALIGN_IN_LEFT_MID, lv_obj_get_style_pad_left(entry, LV_CONT_PART_MAIN), 0);
	lv_obj_set_auto_realign(label, true);

	lv_obj_t * sw = lv_switch_create(entry, NULL);
	lv_group_add_obj(gui.input.group, sw);
	lv_obj_align(sw, entry, LV_ALIGN_IN_RIGHT_MID, -lv_obj_get_style_pad_right(entry, LV_CONT_PART_MAIN), 0);
	lv_obj_set_auto_realign(sw, true);

	lv_obj_set_focus_parent(sw, true);

	if (value)
		lv_switch_on(sw, LV_ANIM_OFF);
	else
		lv_switch_off(sw, LV_ANIM_OFF);

	lv_obj_set_event_cb(sw, gui_list_event_cb);

	return entry;
}

lv_obj_t * gui_list_checkbox_add_entry(lv_obj_t * list, const char * text)
{
	lv_obj_t * entry = lv_cont_create(list, NULL);
	lv_obj_add_style(entry, LV_CONT_PART_MAIN, &gui.styles.list_select);
	lv_cont_set_fit2(entry, LV_FIT_PARENT, LV_FIT_TIGHT);
	lv_cont_set_layout(entry, LV_LAYOUT_COLUMN_LEFT);
	lv_page_glue_obj(entry, true);

	lv_obj_t * checkbox = lv_checkbox_create(entry,  NULL);
	lv_group_add_obj(gui.input.group, checkbox);
	lv_checkbox_set_text(checkbox, text);
	lv_obj_set_focus_parent(checkbox, true);

	lv_obj_set_event_cb(checkbox, gui_list_event_cb);

	return entry;
}

lv_obj_t * gui_list_info_add_entry(lv_obj_t * list, const char * text, char * value)
{
	lv_obj_t * entry = lv_cont_create(list, NULL);
	lv_obj_add_style(entry, LV_CONT_PART_MAIN, &gui.styles.list_select);
	lv_cont_set_fit2(entry, LV_FIT_PARENT, LV_FIT_NONE);
	lv_page_glue_obj(entry, true);
	lv_group_add_obj(gui.input.group, entry);

	lv_obj_t * label = lv_label_create(entry, NULL);
	lv_label_set_text(label, text);
	lv_obj_align(label, entry, LV_ALIGN_IN_TOP_LEFT, lv_obj_get_style_pad_left(entry, LV_CONT_PART_MAIN), lv_obj_get_style_pad_left(entry, LV_CONT_PART_MAIN));
	lv_obj_set_auto_realign(label, true);

	lv_obj_t * info = lv_label_create(entry, NULL);
	lv_label_set_text(info, value);
	lv_obj_align(info, entry, LV_ALIGN_IN_BOTTOM_RIGHT, -lv_obj_get_style_pad_right(entry, LV_CONT_PART_MAIN), -lv_obj_get_style_pad_right(entry, LV_CONT_PART_MAIN));
	lv_obj_set_auto_realign(info, true);

	uint8_t height = lv_obj_get_height(label) + lv_obj_get_height(info);
	height += lv_obj_get_style_pad_top(entry, LV_CONT_PART_MAIN);
	height += lv_obj_get_style_pad_bottom(entry, LV_CONT_PART_MAIN);
	height += 2;

	lv_obj_set_height(entry, height);

	lv_obj_set_event_cb(entry, gui_list_event_cb);

	return entry;
}

void gui_list_info_set_value(lv_obj_t * obj, char * value)
{
	//switch widget is last added child
	lv_obj_t * label = lv_obj_get_child_back(obj, lv_obj_get_child_back(obj, NULL));
	lv_label_set_text(label, value);
}

void gui_list_info_set_name(lv_obj_t * obj, char * value)
{
	//switch widget is second last added child
	lv_obj_t * label = lv_obj_get_child_back(obj, NULL);
	lv_label_set_text(label, value);
}

char * gui_list_info_get_value(lv_obj_t * obj)
{
    //switch widget is last added child
    lv_obj_t * label = lv_obj_get_child_back(obj, lv_obj_get_child_back(obj, NULL));
    return lv_label_get_text(label);
}

char * gui_list_info_get_name(lv_obj_t * obj)
{
    //switch widget is second last added child
    lv_obj_t * label = lv_obj_get_child_back(obj, NULL);
    return lv_label_get_text(label);
}



lv_obj_t * gui_list_add_etc_entry(lv_obj_t * list, const char * text)
{
	lv_obj_t * entry = lv_cont_create(list, NULL);
	lv_cont_set_fit2(entry, LV_FIT_PARENT, LV_FIT_TIGHT);

	lv_obj_t * label = lv_label_create(entry, NULL);
	lv_label_set_text(label, text);

	lv_obj_t * sw = lv_spinbox_create(entry,  NULL);
	lv_group_add_obj(gui.input.group, sw);

	lv_page_glue_obj(entry, true);

	return sw;
}

lv_obj_t * gui_list_cont_add(lv_obj_t * list, uint16_t height)
{
	lv_obj_t * entry = lv_cont_create(list, NULL);
	lv_obj_add_style(entry, LV_CONT_PART_MAIN, &gui.styles.list_select);
	lv_cont_set_fit2(entry, LV_FIT_PARENT, LV_FIT_NONE);
	lv_obj_set_height(entry, height);
	lv_page_glue_obj(entry, true);

	return entry;
}

lv_obj_t * gui_list_textbox_add_entry(lv_obj_t * list, const char * text, const char * value, uint8_t max_len)
{
	lv_obj_t * entry = lv_cont_create(list, NULL);
	lv_obj_add_style(entry, LV_CONT_PART_MAIN, &gui.styles.list_select);
	lv_cont_set_fit2(entry, LV_FIT_PARENT, LV_FIT_NONE);
	lv_page_glue_obj(entry, true);

	lv_obj_t * label = lv_label_create(entry, NULL);
	lv_label_set_text(label, text);
	lv_obj_align(label, entry, LV_ALIGN_IN_TOP_LEFT, lv_obj_get_style_pad_left(entry, LV_CONT_PART_MAIN), lv_obj_get_style_pad_left(entry, LV_CONT_PART_MAIN));
	lv_obj_set_auto_realign(label, true);

	lv_obj_t * textbox = lv_textarea_create(entry, NULL);
	lv_textarea_set_text(textbox, value);
	lv_textarea_set_max_length(textbox, max_len);
	lv_textarea_set_one_line(textbox, true);
	lv_textarea_set_cursor_hidden(textbox, true);
	lv_obj_align(textbox, entry, LV_ALIGN_IN_BOTTOM_RIGHT, -lv_obj_get_style_pad_right(entry, LV_CONT_PART_MAIN), -lv_obj_get_style_pad_right(entry, LV_CONT_PART_MAIN));
	lv_obj_set_auto_realign(textbox, true);
	lv_obj_set_focus_parent(textbox, true);
	lv_group_add_obj(gui.input.group, textbox);

	uint8_t height = lv_obj_get_height(label) + lv_obj_get_height(textbox);
	height += lv_obj_get_style_pad_top(entry, LV_CONT_PART_MAIN);
	height += lv_obj_get_style_pad_bottom(entry, LV_CONT_PART_MAIN);
	height += 2;

	lv_obj_set_height(entry, height);

	lv_obj_set_event_cb(textbox, keyboard_event_cb);

	return entry;
}

const char * gui_list_textbox_get_value(lv_obj_t * obj)
{
	//switch widget is last added child
	lv_obj_t * label = lv_obj_get_child(obj, NULL);
	return lv_textarea_get_text(label);
}

void gui_list_textbox_set_value(lv_obj_t * obj, const char * value)
{
	//switch widget is last added child
	lv_obj_t * label = lv_obj_get_child(obj, NULL);
	lv_textarea_set_text(label, value);
}

lv_obj_t * gui_config_entry_create(lv_obj_t * list, cfg_entry_t * entry, char * name, void * params)
{
	lv_obj_t * obj = NULL;

	switch (entry->type)
	{
		case (ENTRY_BOOL):
			obj = gui_list_switch_add_entry(list, name, config_get_bool(entry));
			break;

		case (ENTRY_TEXT):
			obj = gui_list_textbox_add_entry(list, name, config_get_text(entry), config_text_max_len(entry));
			break;

		case (ENTRY_FLOAT):
		{
			gui_list_slider_options_t * opt = (gui_list_slider_options_t *)params;
			int16_t min = config_float_min(entry) / opt->step;
			int16_t max = config_float_max(entry) / opt->step;
			obj = gui_list_slider_add_entry(list, name, min, max, config_get_float(entry) / opt->step);
			gui_config_entry_update(obj, entry, params);
			break;
		}

		case (ENTRY_INT16):
		{
			gui_list_slider_options_t * opt = (gui_list_slider_options_t *)params;
			int16_t min = config_int_min(entry) / opt->step;
			int16_t max = config_int_max(entry) / opt->step;
			obj = gui_list_slider_add_entry(list, name, min, max, config_get_int(entry) / opt->step);
			gui_config_entry_update(obj, entry, params);
			break;
		}

		default:
			obj = gui_list_info_add_entry(list, name, "???");
	}

	return obj;
}

void gui_config_entry_update(lv_obj_t * obj, cfg_entry_t * entry, void * params)
{
	switch (entry->type)
	{
		case (ENTRY_BOOL):
			config_set_bool(entry, gui_list_switch_get_value(obj));
			break;

		case (ENTRY_TEXT):
			config_set_text(entry, gui_list_textbox_get_value(obj));
			break;

		case (ENTRY_FLOAT):
		{
			gui_list_slider_options_t * opt = (gui_list_slider_options_t *)params;
			float value = gui_list_slider_get_value(obj) * opt->step;
			config_set_float(entry, value);
			char text[16];
			opt->format(text, value * opt->disp_multi);
 			gui_list_slider_set_label(obj, text);
			break;
		}

		case (ENTRY_INT16):
		{
			gui_list_slider_options_t * opt = (gui_list_slider_options_t *)params;
			float value = gui_list_slider_get_value(obj) * opt->step;
			config_set_int(entry, value);
			char text[16];
			opt->format(text, value * opt->disp_multi);
 			gui_list_slider_set_label(obj, text);
			break;
		}
	}
}
