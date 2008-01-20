/*
 * libuci - Library for the Unified Configuration Interface
 * Copyright (C) 2008 Felix Fietkau <nbd@openwrt.org>
 *
 * this program is free software; you can redistribute it and/or modify
 * it under the terms of the gnu lesser general public license version 2.1
 * as published by the free software foundation
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

/* initialize a list head/item */
static inline void uci_list_init(struct uci_list *ptr)
{
	ptr->prev = ptr;
	ptr->next = ptr;
}

/* inserts a new list entry between two consecutive entries */
static inline void __uci_list_add(struct uci_list *prev, struct uci_list *next, struct uci_list *ptr)
{
	prev->next = ptr;
	next->prev = ptr;
	ptr->prev = prev;
	ptr->next = next;
}

/* inserts a new list entry at the tail of the list */
static inline void uci_list_add(struct uci_list *head, struct uci_list *ptr)
{
	/* NB: head->prev points at the tail */
	__uci_list_add(head->prev, head, ptr);
}

static inline void uci_list_del(struct uci_list *ptr)
{
	struct uci_list *next, *prev;

	next = ptr->next;
	prev = ptr->prev;

	prev->next = next;
	next->prev = prev;
}

static void uci_drop_option(struct uci_option *option)
{
	if (!option)
		return;
	if (option->name)
		free(option->name);
	if (option->value)
		free(option->value);
	free(option);
}

static struct uci_option *uci_add_option(struct uci_section *section, const char *name, const char *value)
{
	struct uci_config *cfg = section->config;
	struct uci_context *ctx = cfg->ctx;
	struct uci_option *option = NULL;

	UCI_TRAP_SAVE(ctx, error);
	option = (struct uci_option *) uci_malloc(ctx, sizeof(struct uci_option));
	option->name = uci_strdup(ctx, name);
	option->value = uci_strdup(ctx, value);
	uci_list_add(&section->options, &option->list);
	UCI_TRAP_RESTORE(ctx);

error:
	uci_drop_option(option);
	UCI_THROW(ctx, ctx->errno);
	return NULL;
}

static void uci_drop_section(struct uci_section *section)
{
	struct uci_option *opt;

	if (!section)
		return;

	uci_foreach_entry(option, &section->options, opt) {
		uci_list_del(&opt->list);
		uci_drop_option(opt);
	}

	if (section->name)
		free(section->name);
	if (section->type)
		free(section->type);
	free(section);
}

static struct uci_section *uci_add_section(struct uci_config *cfg, const char *type, const char *name)
{
	struct uci_section *section = NULL;
	struct uci_context *ctx = cfg->ctx;

	UCI_TRAP_SAVE(ctx, error);
	section = (struct uci_section *) uci_malloc(ctx, sizeof(struct uci_section));
	section->config = cfg;
	uci_list_init(&section->list);
	uci_list_init(&section->options);
	section->type = uci_strdup(ctx, type);
	if (name)
		section->name = uci_strdup(ctx, name);
	uci_list_add(&cfg->sections, &section->list);
	UCI_TRAP_RESTORE(ctx);

	return section;

error:
	uci_drop_section(section);
	UCI_THROW(ctx, ctx->errno);
	return NULL;
}

static void uci_drop_file(struct uci_config *cfg)
{
	struct uci_section *s;

	if(!cfg)
		return;

	uci_foreach_entry(section, &cfg->sections, s) {
		uci_list_del(&s->list);
		uci_drop_section(s);
	}

	if (cfg->name)
		free(cfg->name);
	free(cfg);
}


static struct uci_config *uci_alloc_file(struct uci_context *ctx, const char *name)
{
	struct uci_config *cfg = NULL;

	UCI_TRAP_SAVE(ctx, error);
	cfg = (struct uci_config *) uci_malloc(ctx, sizeof(struct uci_config));
	uci_list_init(&cfg->list);
	uci_list_init(&cfg->sections);
	cfg->name = uci_strdup(ctx, name);
	cfg->ctx = ctx;
	UCI_TRAP_RESTORE(ctx);
	return cfg;

error:
	uci_drop_file(cfg);
	UCI_THROW(ctx, ctx->errno);
	return NULL;
}
