#include "mod_perl.h"

void *modperl_config_dir_create(apr_pool_t *p, char *dir)
{
    modperl_config_dir_t *dcfg = modperl_config_dir_new(p);

#ifdef USE_ITHREADS
    /* defaults to per-server lifetime */
    dcfg->interp_lifetime = MP_INTERP_LIFETIME_UNDEF;
#endif

    return dcfg;
}

#define merge_item(item) \
mrg->item = add->item ? add->item : base->item

void *modperl_config_dir_merge(apr_pool_t *p, void *basev, void *addv)
{
    modperl_config_dir_t
        *base = (modperl_config_dir_t *)basev,
        *add  = (modperl_config_dir_t *)addv,
        *mrg  = modperl_config_dir_new(p);

    MP_TRACE_d(MP_FUNC, "basev==0x%lx, addv==0x%lx\n", 
               (unsigned long)basev, (unsigned long)addv);

#ifdef USE_ITHREADS
    merge_item(interp_lifetime);
#endif

    { /* XXX: should do a proper merge of the arrays */
      /* XXX: and check if Perl*Handler is disabled */
        int i;
        for (i=0; i < MP_HANDLER_NUM_PER_DIR; i++) {
            merge_item(handlers_per_dir[i]);
        }
    }

    mrg->flags = modperl_options_merge(p, base->flags, add->flags);

    return mrg;
}

modperl_config_req_t *modperl_config_req_new(request_rec *r)
{
    modperl_config_req_t *rcfg = 
        (modperl_config_req_t *)apr_pcalloc(r->pool, sizeof(*rcfg));

    MP_TRACE_d(MP_FUNC, "0x%lx\n", (unsigned long)rcfg);

    return rcfg;
}

modperl_config_srv_t *modperl_config_srv_new(apr_pool_t *p)
{
    modperl_config_srv_t *scfg = (modperl_config_srv_t *)
        apr_pcalloc(p, sizeof(*scfg));

    scfg->flags = modperl_options_new(p, MpSrvType);
    MpSrvENABLED_On(scfg); /* mod_perl enabled by default */
    MpSrvHOOKS_ALL_On(scfg); /* all hooks enabled by default */

    scfg->argv = apr_array_make(p, 2, sizeof(char *));

    modperl_config_srv_argv_push((char *)ap_server_argv0);

    MP_TRACE_d(MP_FUNC, "0x%lx\n", (unsigned long)scfg);

    return scfg;
}

modperl_config_dir_t *modperl_config_dir_new(apr_pool_t *p)
{
    modperl_config_dir_t *dcfg = (modperl_config_dir_t *)
        apr_pcalloc(p, sizeof(modperl_config_dir_t));

    dcfg->flags = modperl_options_new(p, MpDirType);

    MP_TRACE_d(MP_FUNC, "0x%lx\n", (unsigned long)dcfg);

    return dcfg;
}

#ifdef MP_TRACE
static void dump_argv(modperl_config_srv_t *scfg)
{
    int i;
    char **argv = (char **)scfg->argv->elts;
    fprintf(stderr, "modperl_config_srv_argv_init =>\n");
    for (i=0; i<scfg->argv->nelts; i++) {
        fprintf(stderr, "   %d = %s\n", i, argv[i]);
    }
}
#endif

char **modperl_config_srv_argv_init(modperl_config_srv_t *scfg, int *argc)
{
    modperl_config_srv_argv_push("-e;0");
    
    *argc = scfg->argv->nelts;

    MP_TRACE_g_do(dump_argv(scfg));

    return (char **)scfg->argv->elts;
}

void *modperl_config_srv_create(apr_pool_t *p, server_rec *s)
{
    modperl_config_srv_t *scfg = modperl_config_srv_new(p);

#ifdef USE_ITHREADS
    ap_mpm_query(AP_MPMQ_IS_THREADED, &scfg->threaded_mpm);

    scfg->interp_pool_cfg = 
        (modperl_tipool_config_t *)
        apr_pcalloc(p, sizeof(*scfg->interp_pool_cfg));

    scfg->interp_lifetime = MP_INTERP_LIFETIME_REQUEST;

    /* XXX: determine reasonable defaults */
    scfg->interp_pool_cfg->start = 3;
    scfg->interp_pool_cfg->max_spare = 3;
    scfg->interp_pool_cfg->min_spare = 3;
    scfg->interp_pool_cfg->max = 5;
    scfg->interp_pool_cfg->max_requests = 2000;
#endif /* USE_ITHREADS */

    return scfg;
}

/* XXX: this is not complete */
void *modperl_config_srv_merge(apr_pool_t *p, void *basev, void *addv)
{
    modperl_config_srv_t
        *base = (modperl_config_srv_t *)basev,
        *add  = (modperl_config_srv_t *)addv,
        *mrg  = modperl_config_srv_new(p);

    MP_TRACE_d(MP_FUNC, "basev==0x%lx, addv==0x%lx\n", 
               (unsigned long)basev, (unsigned long)addv);

#ifdef USE_ITHREADS
    merge_item(mip);
    merge_item(interp_pool_cfg);
    merge_item(interp_lifetime);
    merge_item(threaded_mpm);
#else
    merge_item(perl);
#endif

    merge_item(argv);

    { /* XXX: should do a proper merge of the arrays */
      /* XXX: and check if Perl*Handler is disabled */
        int i;
        for (i=0; i < MP_HANDLER_NUM_PER_SRV; i++) {
            merge_item(handlers_per_srv[i]);
        }
        for (i=0; i < MP_HANDLER_NUM_FILES; i++) {
            merge_item(handlers_files[i]);
        }
        for (i=0; i < MP_HANDLER_NUM_PROCESS; i++) {
            merge_item(handlers_process[i]);
        }
        for (i=0; i < MP_HANDLER_NUM_CONNECTION; i++) {
            merge_item(handlers_connection[i]);
        }
    }

    mrg->flags = modperl_options_merge(p, base->flags, add->flags);

    return mrg;
}

