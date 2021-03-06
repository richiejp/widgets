//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2019 Cyril Hrubis <metan@ucw.cz>

 */

#ifndef GP_WIDGET_CHOICE_H__
#define GP_WIDGET_CHOICE_H__

struct gp_widget_choice {
	unsigned int sel;
	unsigned int max;
	char **choices;

	char payload[];
};

struct gp_widget *gp_widget_choice_new(const char *choices[],
                                       unsigned int choice_cnt,
                                       unsigned int selected,
				       int (*on_event)(gp_widget_event *self),
				       void *event_ptr);

void gp_widget_choice_set(gp_widget *self, unsigned int sel);

#endif /* GP_WIDGET_CHOICE_H__ */
