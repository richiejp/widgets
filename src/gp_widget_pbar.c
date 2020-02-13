//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2019 Cyril Hrubis <metan@ucw.cz>

 */

#include <string.h>
#include <json-c/json.h>

#include <gp_widgets.h>
#include <gp_widget_ops.h>
#include <gp_widget_render.h>

static unsigned int pbar_min_w(gp_widget *self, const gp_widget_render_ctx *ctx)
{
	(void)self;

	return 2 * ctx->padd + gp_text_max_width(ctx->font, 7);
}

static unsigned int pbar_min_h(gp_widget *self, const gp_widget_render_ctx *ctx)
{
	(void)self;

	return 2 * ctx->padd + gp_text_ascent(ctx->font);
}

static void pbar_render(gp_widget *self, const gp_offset *offset,
                        const gp_widget_render_ctx *ctx, int flags)
{
	unsigned int x = self->x + offset->x;
	unsigned int y = self->y + offset->y;
	unsigned int w = self->w;
	unsigned int h = self->h;

	(void)flags;

	gp_widget_ops_blit(ctx, x, y, w, h);

	unsigned int wd = self->pbar->val * w / 100;

	gp_pixmap p;

	gp_sub_pixmap(ctx->buf, &p, x, y, wd, h);
	if (p.w > 0) {
		gp_fill_rrect_xywh(&p, 0, 0, w, h, ctx->bg_color,
		                   ctx->fg2_color, ctx->text_color);
	}

	gp_sub_pixmap(ctx->buf, &p, x+wd, y, w-wd, h);
	if (p.w > 0) {
		gp_fill_rrect_xywh(&p, -wd, 0, w, h, ctx->bg_color,
		                   ctx->fg_color, ctx->text_color);
	}

	gp_print(ctx->buf, ctx->font, x + w/2, y + ctx->padd,
		 GP_ALIGN_CENTER | GP_VALIGN_BELOW | GP_TEXT_NOBG,
		 ctx->text_color, ctx->bg_color, "%.2f%%",
		 self->pbar->val);
}

static int check_val(double val)
{
	if (val < 0 || val > 100) {
		GP_WARN("Invalid progressbar value %lf", val);
		return 1;
	}

	return 0;
}

static gp_widget *json_to_pbar(json_object *json, void **uids)
{
	double val = 0;

	(void)uids;

	json_object_object_foreach(json, key, jval) {
		if (!strcmp(key, "val"))
			val = json_object_get_double(jval);
		else
			GP_WARN("Invalid int pbar '%s'", key);
	}

	if (check_val(val))
		val = 0;

	return gp_widget_pbar_new(val);
}

struct gp_widget_ops gp_widget_progress_bar_ops = {
	.min_w = pbar_min_w,
	.min_h = pbar_min_h,
	.render = pbar_render,
	.from_json = json_to_pbar,
	.id = "progressbar",
};

gp_widget *gp_widget_pbar_new(float val)
{
	gp_widget *ret;

	if (check_val(val))
		val = 0;

	ret = gp_widget_new(GP_WIDGET_PROGRESSBAR, sizeof(struct gp_widget_pbar));
	if (!ret)
		return NULL;

	ret->pbar->val = val;

	return ret;
}

void gp_widget_pbar_set(gp_widget *self, float val)
{
	GP_WIDGET_ASSERT(self, GP_WIDGET_PROGRESSBAR, );

	GP_DEBUG(3, "Setting widget (%p) progressbar val '%.2f' -> '%.2f'",
		 self, self->pbar->val, val);

	if (check_val(val))
		return;

	if (self->pbar->val != val) {
		gp_widget_redraw(self);
		self->pbar->val = val;
	}
}
