/*
 * actions.c
 *
 *  Created on: 7. 10. 2022
 *      Author: horinek
 */


#include "actions.h"
#include "gui/gui.h"
#include "gui/tasks/page/pages.h"
#include "gui/dialog.h"

#include "fc/fc.h"

#include "shortcuts.h"

//actions definitions
#include "actions/actions.inc"

uint16_t shortcuts_get_number()
{
    return sizeof(shortcut_actions) / sizeof(shortcut_item_t);
}

void shortcut_update_icon(lv_obj_t * label, shortcut_item_t * shrt)
{
    char icon[SHORTCUT_ICON_LEN];
    char text[SHORTCUT_LABEL_LEN];
    shrt->icon(icon, text);

    lv_label_set_text(label, icon);
}

shortcut_item_t * shortcuts_get_from_name(char * name)
{
    for (uint16_t i = 0; i < shortcuts_get_number(); i++)
    {
        if (strcmp(shortcut_actions[i].name, name) == 0)
            return &shortcut_actions[i];
    }

    return ACTION_NO_ACTION;
}
