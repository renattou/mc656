#include <stdio.h>
#include <stdlib.h>
#include <glpk.h>
#include <float.h>

#define TLIM_PLI 180000

double bestDB = DBL_MIN;
/* double bgap = 100; */
int a_cnt; /* current number of active nodes */
int n_cnt; /* current number of all (active and inactive) nodes; */
int t_cnt; /* total number of nodes including those already removed */

void cb_func(glp_tree *T, void *info) {
    double aux;
    int bn;

    glp_ios_tree_size(T, &a_cnt, &n_cnt, &t_cnt);
    bn=glp_ios_best_node(T);
    aux=glp_ios_node_bound(T,bn);

    /* aux=glp_ios_mip_gap(T) ; */
    if (aux > bestDB) {
        bestDB=aux;
        /* bgap=glp_ios_mip_gap(T); */
        /* fprintf(stderr,">>> bestDB=%10.2f, gap=%10.8f,no=%d\n", */
        /*   bestDB,bgap,glp_ios_best_node(T)); */

        /* fprintf(stderr,">>>> cur_node=%d, a_cnt=%d, n_cnt=%d, t_cnt=%d\n", */
        /*         glp_ios_curr_node(T),a_cnt,n_cnt,t_cnt); */
    }

    return;
}

int main(int argc, char* argv[]) {
    glp_iocp iocp;
    glp_prob *mip;
    glp_tran *tran;
    int ret;

    /* glp_term_out(GLP_OFF); */
    mip = glp_create_prob();
    tran = glp_mpl_alloc_wksp();
    ret = glp_mpl_read_model(tran, "pli.mod", 1);
    if (ret != 0) {
        fprintf(stderr, "Error on translating model\n");
        goto skip;
    }
    ret = glp_mpl_read_data(tran,argv[1]);
    if (ret != 0) {
        fprintf(stderr, "Error on translating data\n");
        goto skip;
    }
    ret = glp_mpl_generate(tran, NULL);
    if (ret != 0) {
        fprintf(stderr, "Error on generating model\n");
        goto skip;
    }
    glp_mpl_build_prob(tran, mip);
    glp_simplex(mip, NULL);

    glp_init_iocp(&iocp);
    iocp.cb_func = cb_func;
    iocp.tm_lim = TLIM_PLI;  /* limite de tempo de execução (em ms) */
    iocp.bt_tech = GLP_BT_BPH;

    glp_intopt(mip, &iocp);
    ret = glp_mpl_postsolve(tran, mip, GLP_MIP);
    if (ret != 0)
        fprintf(stderr, "Error on postsolving model\n");

    int status = glp_mip_status(mip);
    if (status == GLP_OPT) {
        double obj = glp_mip_obj_val(mip);
        bestDB = obj;
        /* bgap = 0.0; */
    }

 skip:
    glp_mpl_free_wksp(tran);
    glp_delete_prob(mip);

    printf("%.2lf\n", bestDB);
    printf("%d\n", t_cnt);

    return 0;
}
