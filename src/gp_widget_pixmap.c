//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2019 Cyril Hrubis <metan@ucw.cz>

 */

#include <string.h>
#include <json-c/json.h>

#include <gp_widgets.h>
#include <gp_widget_ops.h>
#include <gp_widget_render.h>
#include <gp_widget_json.h>

static unsigned int min_w(gp_widget *self)
{
	return self->pixmap->w;
}

static unsigned int min_h(gp_widget *self)
{
	return self->pixmap->h;
}

static void render(gp_widget *self,
                   struct gp_widget_render *render, int flags)
{
	unsigned int x = self->x;
	unsigned int y = self->y;

	(void)flags;

	if (!self->pixmap->pixmap) {
		gp_pixmap *p = gp_pixmap_alloc(self->pixmap->w, self->pixmap->h,
		                               render->buf->pixel_type);

		if (!p)
			return;

		self->pixmap->pixmap = p;

		gp_widget_send_event(self->pixmap->on_event, self,
		                     self->pixmap->event_ptr,
		                     GP_WIDGET_EVENT_REDRAW);
	}

	gp_blit_xywh(self->pixmap->pixmap, 0, 0,
	             self->pixmap->w, self->pixmap->h,
	             render->buf, x, y);
}

static int event(gp_widget *self, gp_event *ev)
{
	int ret;

	ev->cursor_x -= self->x;
	ev->cursor_y -= self->y;

	ret = gp_widget_send_event(self->pixmap->on_event, self,
	                           self->pixmap->event_ptr,
	                           GP_WIDGET_EVENT_INPUT,
	                           (long)ev);

	ev->cursor_x += self->x;
	ev->cursor_y += self->y;

	return ret;
}

static gp_widget *json_to_pixmap(json_object *json, void **uids)
{
	const char *on_event = NULL;
	unsigned int w = 0;
	unsigned int h = 0;
	void *on_event_fn = NULL;

	(void)uids;

	json_object_object_foreach(json, key, val) {
		if (!strcmp(key, "on_event"))
			on_event = json_object_get_string(val);
		else if (!strcmp(key, "w"))
			w = json_object_get_int(val);
		else if (!strcmp(key, "h"))
			h = json_object_get_int(val);
		else
			GP_WARN("Invalid pixmap key '%s'", key);
	}

	if (on_event) {
		on_event_fn = gp_widget_callback_addr(on_event);

		if (!on_event_fn)
			GP_WARN("No on_event function '%s' defined", on_event);
	}

	if (w == 0 || h == 0) {
		GP_WARN("Invalid pixmap size %ux%u\n", w, h);
		return NULL;
	}

	return gp_widget_pixmap_new(w, h, on_event_fn, NULL);
}

struct gp_widget_ops gp_widget_pixmap_ops = {
	.min_w = min_w,
	.min_h = min_h,
	.render = render,
	.event = event,
	.from_json = json_to_pixmap,
	.id = "pixmap",
};

struct gp_widget *gp_widget_pixmap_new(unsigned int w, unsigned int h,
                                       int (*on_event)(gp_widget_event *ev),
                                       void *event_ptr)
{
	gp_widget *ret;

	ret = gp_widget_new(GP_WIDGET_PIXMAP, sizeof(struct gp_widget_pixmap));
	if (!ret)
		return NULL;

	ret->pixmap->on_event = on_event;
	ret->pixmap->event_ptr = event_ptr;
	ret->pixmap->w = w;
	ret->pixmap->h = h;
	ret->pixmap->pixmap = NULL;

	gp_widget_send_event(on_event, ret, event_ptr, GP_WIDGET_EVENT_NEW);

	return ret;
}
