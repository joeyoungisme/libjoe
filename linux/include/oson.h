#ifndef MISC_JSON_H
#define MISC_JSON_H

#include <stdbool.h>
#include <json-c/json.h>

typedef json_type oson_type;
#define oson_type_null         json_type_null
#define oson_type_boolean      json_type_boolean
#define oson_type_double       json_type_double
#define oson_type_int          json_type_int
#define oson_type_object       json_type_object
#define oson_type_array        json_type_array
#define oson_type_string       json_type_string

typedef struct json_object oson_object;

int oson_get_int(oson_object *);
bool oson_get_bool(oson_object *);
char *oson_get_str(oson_object *);
double oson_get_double(oson_object *);

oson_object *oson_obj_get(oson_object *, char *);
oson_object *oson_obj_parse(char *, char *);
oson_object *oson_arr_get(oson_object *, int);

oson_object *oson_str(char *, char *);
oson_object *oson_int(char *, int);
oson_object *oson_bool(char *, bool);
oson_object *oson_double(char *, double);
oson_object *oson_arr(char *);
oson_object *oson_obj(char *);

int oson_arr_len(oson_object *);
int oson_arr_add(oson_object *, oson_object *);
int oson_obj_add(oson_object *, char *, oson_object *);

json_type oson_obj_type(oson_object *);
void oson_show(void);
int oson_free(oson_object *obj);

#endif
