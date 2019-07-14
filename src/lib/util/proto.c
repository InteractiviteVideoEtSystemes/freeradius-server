/*
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

/** Protocol encoder/decoder support functions
 *
 * @file src/lib/util/proto.c
 *
 * @copyright 2015 The FreeRADIUS server project
 */
#include "proto.h"

static unsigned int proto_log_indent = 30;
static char spaces[] = "                                                 ";

void fr_proto_print(char const *file, int line, char const *fmt, ...)
{
	va_list		ap;
	size_t		len;
	char		prefix[256];

	len = snprintf(prefix, sizeof(prefix), "%s:%i", file, line);
	if (len > proto_log_indent) proto_log_indent = len;

	fr_log(&default_log, L_DBG, file, line, "msg: %s%.*s: ", prefix, (int)(proto_log_indent - len), spaces);

	va_start(ap, fmt);
	fr_vlog(&default_log, L_DBG, file, line, fmt, ap);
	va_end(ap);
}

DIAG_OFF(format-nonliteral)
void fr_proto_print_hex_data(char const *file, int line, uint8_t const *data, size_t data_len, char const *fmt, ...)
{
	va_list		ap;
	size_t		len;
	char		prefix[256];
	char		msg[256];

	len = snprintf(prefix, sizeof(prefix), "%s:%i", file, line);
	if (len > proto_log_indent) proto_log_indent = len;

	if (fmt) {
		va_start(ap, fmt);
		fr_vlog(&default_log, L_DBG, file, line, fmt, ap);
		va_end(ap);
		fr_log(&default_log, L_DBG, file, line, "hex: %s%.*s: -- %s --\n",
			prefix, (int)(proto_log_indent - len), spaces, msg);
	}

	fr_log(&default_log, L_DBG, file, line, "hex: ");
	fr_log_hex(&default_log, L_DBG, file, line, data, data_len);
}
DIAG_ON(format-nonliteral)

void fr_proto_tlv_stack_print(char const *file, int line, char const *func, fr_dict_attr_t const **tlv_stack, unsigned int depth)
{
	int		i;
	char		prefix[256];
	size_t		len;

	len = snprintf(prefix, sizeof(prefix), "%s:%i", file, line);
	if (len > proto_log_indent) proto_log_indent = len;

	for (i = 0; (i < FR_DICT_MAX_TLV_STACK) && tlv_stack[i]; i++);
	if (!i) return;

	fr_log(&default_log, L_DBG, file, line, "stk: %s%.*s: Currently in %s",
		prefix, (int)(proto_log_indent - len), spaces, func);
	for (i--; i >= 0; i--) {
		fr_log(&default_log, L_DBG, file, line,
		       "stk: %s%.*s: %s [%i] %s: %s, vendor: 0x%x (%u), attr: 0x%x (%u)",
		       prefix, (int)(proto_log_indent - len), spaces, (i == (int)depth) ? ">" : " ", i,
		       fr_int2str(fr_value_box_type_table, tlv_stack[i]->type, "?Unknown?"),
		       tlv_stack[i]->name,
		       fr_dict_vendor_num_by_da(tlv_stack[i]), fr_dict_vendor_num_by_da(tlv_stack[i]),
		       tlv_stack[i]->attr, tlv_stack[i]->attr);
	}
	fr_log(&default_log, L_DBG, file, line, "stk: %s%.*s:",
	       prefix, (int)(proto_log_indent - len), spaces);
}

void fr_proto_tlv_stack_build(fr_dict_attr_t const **tlv_stack, fr_dict_attr_t const *da)
{
	int i;
	fr_dict_attr_t const *da_p;

	memset(tlv_stack, 0, sizeof(*tlv_stack) * (FR_DICT_MAX_TLV_STACK + 1));

	if (!da) return;

	/*
	 *	We've finished encoding one nested structure
	 *	now we need to rebuild the tlv_stack and determine
	 *	where the common point is.
	 */
	for (i = da->depth, da_p = da;
	     da_p->parent && (i >= 0);
	     i--, da_p = da_p->parent) tlv_stack[i - 1] = da_p;
}