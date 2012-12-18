
/**
 *  txtset directive
 *
 * @author    Jason Kleban <jkleban@wayfair.com>
 * @copyright 2012 Wayfair, LLC - All rights reserved
 */

#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

#define TXTSET_CACHE_LIFE 10
#define TXTSET_MAX_SIZE 20

typedef struct {
    u_char *text_file;
    u_char *default_value;
    u_char *cached_value;
    time_t cached_time;
    ngx_int_t cache_life;
} txtvar_info;

static char *ngx_txtset(ngx_conf_t *cf, ngx_command_t *cmd,
    void *conf);

static ngx_command_t  ngx_http_rewrite_commands[] = {

    { ngx_string("txtset"),
      NGX_HTTP_SRV_CONF|NGX_HTTP_SIF_CONF|NGX_HTTP_LOC_CONF|NGX_HTTP_LIF_CONF
                       |NGX_CONF_TAKE3|NGX_CONF_TAKE4,
      ngx_http_rewrite_txtset,
      NGX_HTTP_LOC_CONF_OFFSET,
      0,
      NULL },

      ngx_null_command
};


static ngx_http_module_t  ngx_http_rewrite_module_ctx = {
    NULL,                                  /* preconfiguration */
    NULL,                                  /* postconfiguration */

    NULL,                                  /* create main configuration */
    NULL,                                  /* init main configuration */

    NULL,                                  /* create server configuration */
    NULL,                                  /* merge server configuration */

    NULL,                                  /* create location configuration */
    NULL,                                  /* merge location configuration */
};


ngx_module_t  ngx_http_rewrite_module = {
    NGX_MODULE_V1,
    &ngx_http_rewrite_module_ctx,          /* module context */
    ngx_http_rewrite_commands,             /* module directives */
    NGX_HTTP_MODULE,                       /* module type */
    NULL,                                  /* init master */
    NULL,                                  /* init module */
    NULL,                                  /* init process */
    NULL,                                  /* init thread */
    NULL,                                  /* exit thread */
    NULL,                                  /* exit process */
    NULL,                                  /* exit master */
    NGX_MODULE_V1_PADDING
};


int
check_string(u_char *string)
{
    int i;
    if (string == NULL || string[0] == '\0' || string[0] == '\n') {
        return 0;
    }
    for (i=0; string[i] != '\0'; i++) {
        if (!(string[i] >= '0' && string[i] <= '9') &&
            !(string[i] >= 'A' && string[i] <= 'Z') &&
            !(string[i] >= 'a' && string[i] <= 'z') &&
            !(string[i] == '-' || string[i] == '_')) {
            if (string[i] == '\n') {
                string[i] = 0;
                return 1;
            }
            return 0;
        }
    }
    return 1;
}


ngx_int_t
ngx_http_txtset_variable(ngx_http_request_t *r, ngx_http_variable_value_t *v, uintptr_t data)
{
    FILE *fp;
    time_t now = time(NULL);
    txtvar_info *element = (txtvar_info *) data;
    int valid = 1;

    if (element->cached_time == 0 || now - element->cached_time > element->cache_life) {
        if ((fp = fopen(element->text_file, "r")) == NULL) {
            valid = 0;
        }
        if ((v->data = ngx_pnalloc(r->pool, TXTSET_MAX_SIZE + 1)) == NULL) {
            return NGX_ERROR;
        }
        v->data[TXTSET_MAX_SIZE] = '\0';

        if (valid == 1) {
            fgets(v->data, TXTSET_MAX_SIZE, fp);
            valid = check_string(v->data);
            fclose(fp);
        }

        if (valid == 0) {
            memcpy(v->data, element->default_value, TXTSET_MAX_SIZE + 1);
        }

        memcpy(element->cached_value, v->data, TXTSET_MAX_SIZE + 1);
        element->cached_time = now;
    } else {
        v->data = element->cached_value;
    }
    v->len = ngx_strlen(v->data);
    v->valid = 1;
    v->no_cacheable = 0;
    v->not_found = 0;
    return NGX_OK;
}


static char *
ngx_txtset(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_str_t                           *value;
    ngx_http_variable_t                 *v;
    txtvar_info                             *element;

    value = cf->args->elts;

    if (value[1].data[0] != '$') {
        ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                           "invalid variable name \"%V\"", &value[1]);
        return NGX_CONF_ERROR;
    }

    value[1].len--;
    value[1].data++;

    v = ngx_http_add_variable(cf, &value[1], NGX_HTTP_VAR_CHANGEABLE);
    if (v == NULL) {
        return NGX_CONF_ERROR;
    }

    v->get_handler = ngx_http_txtset_variable;

    if ((element = ngx_pcalloc(cf->pool, sizeof(txtvar_info))) == NULL) {
        return NGX_CONF_ERROR;
    }

    if ((element->text_file = ngx_pcalloc(cf->pool, value[2].len)) == NULL) {
        return NGX_CONF_ERROR;
    }
    memcpy(element->text_file, value[2].data, value[2].len);
    if ((element->default_value = ngx_pcalloc(cf->pool, value[3].len)) == NULL) {
        return NGX_CONF_ERROR;
    }
    memcpy(element->default_value, value[3].data, value[3].len);

    if (cf->args->nelts > 4) {
        element->cache_life = (ngx_int_t) atoi(value[4].data);
        if (element->cache_life == 0) {
            element->cache_life = TXTSET_CACHE_LIFE;
        }
    } else {
        element->cache_life = TXTSET_CACHE_LIFE;
    }

    if ((element->cached_value = ngx_pcalloc(cf->pool, TXTSET_MAX_SIZE + 1)) == NULL) {
        return NGX_CONF_ERROR;
    }

    element->cached_time == 0;

    v->data = (uintptr_t) element;

    return NGX_CONF_OK;
}
