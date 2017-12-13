#include <stdlib.h>

typedef union t_instr {
	t_affect affectation;
	t_instr* bloc;
	t_return _return;
	t_ite _ite;
}
