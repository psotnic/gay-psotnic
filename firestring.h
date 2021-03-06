/*
firestring.h - FireString string function declarations
Copyright (C) 2000 Ian Gulliver

This program is free software; you can redistribute it and/or modify
it under the terms of version 2 of the GNU General Public License as
published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#ifndef _FIRESTRING_H
#define _FIRESTRING_H

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#ifndef _FIRESTRING_C
extern const char *firestring_version;
#endif

struct firestring_conf_t {
	char *var;
	char *value;
	struct firestring_conf_t *next;
};

enum firestring_conf_parse_line_result {
	FSC_INCOMPLETE = -1,
	FSC_VALID      = 0,
	FSC_MALFORMED  = 1
};

struct firestring_conf_keyword_t {
	const char *keyword;
	enum firestring_conf_parse_line_result (*callback)(char *line, struct firestring_conf_t **conf);
};

struct firestring_estr_t {
	char *s;
	long a;
	long l;
};

void firestring_set_error_handler(void (*e)());

void *firestring_malloc(const size_t size);
void *firestring_realloc(void *old, const size_t _new);
char *firestring_strdup(const char *input);
void firestring_strncpy(char *to, const char *from, const size_t size);
void firestring_strncat(char *to, const char *from, const size_t size);
long firestring_printf(const char *format, ...);
long firestring_fprintf(FILE *stream, const char *format, ...);
long firestring_vfprintf(FILE *stream, const char *format, va_list ap);
long firestring_printfe(const struct firestring_estr_t *format, ...);
long firestring_fprintfe(FILE *stream, const struct firestring_estr_t *format, ...);
long firestring_vfprintfe(FILE *stream, const struct firestring_estr_t *format, va_list ap);
long firestring_snprintf(char *out, const long size, const char *format, ...);
int firestring_strncasecmp(const char *s1, const char *s2, const long n);
int firestring_strcasecmp(const char *s1, const char *s2);
char *firestring_concat(const char *s, ...);
char *firestring_chomp(char *s);
char *firestring_chug(char *s);
char *firestring_trim(char *s);
int firestring_hextoi(const char *input);

/* estr functions */
void firestring_estr_alloc(struct firestring_estr_t *f, const long a);
void firestring_estr_expand(struct firestring_estr_t *f, const long a);
void firestring_estr_free(struct firestring_estr_t *f);
int firestring_estr_base64_encode(struct firestring_estr_t *t, const struct firestring_estr_t *f);
int firestring_estr_base64_decode(struct firestring_estr_t *t, const struct firestring_estr_t *f);
int firestring_estr_xml_encode(struct firestring_estr_t *t, const struct firestring_estr_t *f);
int firestring_estr_xml_decode(struct firestring_estr_t *t, const struct firestring_estr_t *f);
int firestring_estr_read(struct firestring_estr_t *f, const int fd);
long firestring_estr_sprintf(struct firestring_estr_t *o, const char *format, ...);
long firestring_estr_vsprintf(struct firestring_estr_t *o, const char *format, va_list ap);
long firestring_estr_sprintfe(struct firestring_estr_t *o, const struct firestring_estr_t *format, ...);
long firestring_estr_vsprintfe(struct firestring_estr_t *o, const struct firestring_estr_t *format, va_list ap);
long firestring_estr_strchr(const struct firestring_estr_t *f, const char c, const long start);
long firestring_estr_strstr(const struct firestring_estr_t *f, const char *s, const long start);
long firestring_estr_stristr(const struct firestring_estr_t *f, const char *s, const long start);
int firestring_estr_starts(const struct firestring_estr_t *f, const char *s);
int firestring_estr_ends(const struct firestring_estr_t *f, const char *s);
int firestring_estr_estarts(const struct firestring_estr_t *f, const struct firestring_estr_t *s);
int firestring_estr_eends(const struct firestring_estr_t *f, const struct firestring_estr_t *s);
int firestring_estr_strcasecmp(const struct firestring_estr_t *f, const char *s);
int firestring_estr_strcmp(const struct firestring_estr_t *f, const char *s);
int firestring_estr_strcpy(struct firestring_estr_t *f, const char *s);
int firestring_estr_astrcpy(struct firestring_estr_t *f, const char *s);
int firestring_estr_strcat(struct firestring_estr_t *f, const char *s);
int firestring_estr_astrcat(struct firestring_estr_t *f, const char *s);
int firestring_estr_estrcasecmp(const struct firestring_estr_t *t, const struct firestring_estr_t *f, const long start);
int firestring_estr_estrncasecmp(const struct firestring_estr_t *t, const struct firestring_estr_t *f, const long length, const long start);
int firestring_estr_estrcpy(struct firestring_estr_t *t, const struct firestring_estr_t *f, const long start);
int firestring_estr_aestrcpy(struct firestring_estr_t *t, const struct firestring_estr_t *f, const long start);
int firestring_estr_munch(struct firestring_estr_t *t, const long length);
void firestring_estr_chomp(struct firestring_estr_t *s);
void firestring_estr_chug(struct firestring_estr_t *s);
void firestring_estr_ip_chug(struct firestring_estr_t *s);
void firestring_estr_trim(struct firestring_estr_t *s);
void firestring_estr_ip_trim(struct firestring_estr_t *s);
int firestring_estr_estrcmp(const struct firestring_estr_t *t, const struct firestring_estr_t *f, const long start);
int firestring_estr_estrcat(struct firestring_estr_t *t, const struct firestring_estr_t *f, const long start);
int firestring_estr_aestrcat(struct firestring_estr_t *t, const struct firestring_estr_t *f, const long start);
long firestring_estr_estrstr(const struct firestring_estr_t *haystack, const struct firestring_estr_t *needle, const long start);
long firestring_estr_estristr(const struct firestring_estr_t *haystack, const struct firestring_estr_t *needle, const long start);
int firestring_estr_replace(struct firestring_estr_t *dest, const struct firestring_estr_t *source, const struct firestring_estr_t *to, const struct firestring_estr_t *from);
int firestring_estr_areplace(struct firestring_estr_t *dest, const struct firestring_estr_t *source, const struct firestring_estr_t *to, const struct firestring_estr_t *from);
int firestring_estr_tolower(struct firestring_estr_t *dest, const struct firestring_estr_t *source, const long start);
int firestring_estr_toupper(struct firestring_estr_t *dest, const struct firestring_estr_t *source, const long start);
void firestring_estr_0(struct firestring_estr_t *f);

/* configuration system functions */
struct firestring_conf_t *firestring_conf_parse(const char *filename);
struct firestring_conf_t *firestring_conf_parse_next(const char *filename, struct firestring_conf_t *prev);
enum firestring_conf_parse_line_result firestring_conf_parse_line(const char *line, const struct firestring_conf_keyword_t keywords[], struct firestring_conf_t **conf, char **context);
struct firestring_conf_t *firestring_conf_add(struct firestring_conf_t *next, const char *var, const char *value);
struct firestring_conf_t *firestring_conf_delete(struct firestring_conf_t *conf, const char *var);
char *firestring_conf_find(const struct firestring_conf_t *config, const char *var);
char *firestring_conf_find_next(const struct firestring_conf_t *config, const char *var, const char *prev);
void firestring_conf_free(struct firestring_conf_t *config);

#endif
