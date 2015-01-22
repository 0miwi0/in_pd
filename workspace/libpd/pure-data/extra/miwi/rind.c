#include "m_pd.h"
#include <time.h>

/* -------------------------- rind -------------------------- */

static t_class *rind_class;

typedef struct _rind {
	t_object x_obj;
	t_float x_min, x_max;
	unsigned int x_state;
} t_rind;

static int rind_time(void) {
	int thym = time(0) % 31536000; // seconds in a year
	return thym + !(thym%2); // odd numbers only
}

static int rind_makeseed(void) {
	static unsigned int rind_next = 1378742615;
	rind_next = rind_next * rind_time() + 938284287;
	return (rind_next & 0x7fffffff);
}

static void rind_seed(t_rind *x, t_symbol *s, int argc, t_atom *argv) {
	x->x_state = (argc ? atom_getfloat(argv) : rind_time());
}

static void rind_peek(t_rind *x, t_symbol *s) {
	post("%s%s%u", s->s_name, (*s->s_name ? ": " : ""), x->x_state);
}

static void rind_bang(t_rind *x) {
	double min=x->x_min, n=x->x_max-min, nval;
	double range = (!n? 1:n);
	unsigned int state = x->x_state;
	x->x_state = state = state * 472940017 + 832416023;
	nval = range * state * (1./4294967296.) + min;
	outlet_float(x->x_obj.ob_outlet, nval);
}

static void *rind_new(t_symbol *s, int argc, t_atom *argv) {
	t_rind *x = (t_rind *)pd_new(rind_class);
	t_float min=0, max=1;
	switch (argc) {
	  case 2:
		max=atom_getfloat(argv+1);
		min=atom_getfloat(argv);
	  break;
	  case 1: max=atom_getfloat(argv);
	}
	x->x_min=min, x->x_max=max;
	x->x_state = rind_makeseed();
	floatinlet_new(&x->x_obj, &x->x_min);
	floatinlet_new(&x->x_obj, &x->x_max);
	outlet_new(&x->x_obj, &s_float);
	return (x);
}

void rind_setup(void) {
	rind_class = class_new(gensym("rind"),
		(t_newmethod)rind_new, 0,
		sizeof(t_rind), 0,
		A_GIMME, 0);

	class_addbang(rind_class, rind_bang);
	class_addmethod(rind_class, (t_method)rind_seed,
		gensym("seed"), A_GIMME, 0);
	class_addmethod(rind_class, (t_method)rind_peek,
		gensym("peek"), A_DEFSYM, 0);
}
