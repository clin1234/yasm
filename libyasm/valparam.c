/*
 * Value/Parameter type functions
 *
 *  Copyright (C) 2001  Peter Johnson
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND OTHER CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR OTHER CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#define YASM_LIB_INTERNAL
#include "util.h"
/*@unused@*/ RCSID("$Id$");

#include "libyasm-stdint.h"
#include "coretype.h"
#include "valparam.h"

#include "errwarn.h"
#include "expr.h"
#include "symrec.h"


void
yasm_call_directive(const yasm_directive *directive, yasm_object *object,
		    yasm_valparamhead *valparams,
		    yasm_valparamhead *objext_valparams, unsigned long line)
{
    yasm_valparam *vp;

    if ((directive->flags & (YASM_DIR_ARG_REQUIRED|YASM_DIR_ID_REQUIRED)) &&
	(!valparams || !yasm_vps_first(valparams))) {
	yasm_error_set(YASM_ERROR_SYNTAX,
		       N_("directive `%s' requires an argument"),
		       directive->name);
	return;
    }
    if (valparams) {
	vp = yasm_vps_first(valparams);
	if ((directive->flags & YASM_DIR_ID_REQUIRED) && !vp->val) {
	    yasm_error_set(YASM_ERROR_SYNTAX,
		N_("directive `%s' requires an identifier parameter"),
		directive->name);
	    return;
	}
    }
    directive->handler(object, valparams, objext_valparams, line);
}

yasm_valparam *
yasm_vp_create(/*@keep@*/ char *v, /*@keep@*/ yasm_expr *p)
{
    yasm_valparam *r = yasm_xmalloc(sizeof(yasm_valparam));
    r->val = v;
    r->param = p;
    return r;
}

/*@null@*/ /*@only@*/ yasm_expr *
yasm_vp_expr(yasm_valparam *vp, yasm_symtab *symtab, unsigned long line)
{
    if (!vp)
	return NULL;
    if (vp->val) {
	return yasm_expr_create_ident(yasm_expr_sym(
	    yasm_symtab_use(symtab, vp->val, line)), line);
    } else if (vp->param) {
	yasm_expr *e = vp->param;
	vp->param = NULL;   /* to avoid double-free */
	return e;
    } else
	return NULL;
}

void
yasm_vps_delete(yasm_valparamhead *headp)
{
    yasm_valparam *cur, *next;

    cur = STAILQ_FIRST(headp);
    while (cur) {
	next = STAILQ_NEXT(cur, link);
	if (cur->val)
	    yasm_xfree(cur->val);
	if (cur->param)
	    yasm_expr_destroy(cur->param);
	yasm_xfree(cur);
	cur = next;
    }
    STAILQ_INIT(headp);
}

void
yasm_vps_print(const yasm_valparamhead *headp, FILE *f)
{
    const yasm_valparam *vp;

    if(!headp) {
	fprintf(f, "(none)");
	return;
    }

    yasm_vps_foreach(vp, headp) {
	if (vp->val)
	    fprintf(f, "(\"%s\",", vp->val);
	else
	    fprintf(f, "((nil),");
	if (vp->param)
	    yasm_expr_print(vp->param, f);
	else
	    fprintf(f, "(nil)");
	fprintf(f, ")");
	if (yasm_vps_next(vp))
	    fprintf(f, ",");
    }
}

yasm_valparamhead *
yasm_vps_create(void)
{
    yasm_valparamhead *headp = yasm_xmalloc(sizeof(yasm_valparamhead));
    yasm_vps_initialize(headp);
    return headp;
}

void
yasm_vps_destroy(yasm_valparamhead *headp)
{
    yasm_vps_delete(headp);
    yasm_xfree(headp);
}

/* Non-macro yasm_vps_append() for non-YASM_LIB_INTERNAL users. */
#undef yasm_vps_append
void
yasm_vps_append(yasm_valparamhead *headp, /*@keep@*/ yasm_valparam *vp)
{
    if (vp)
	STAILQ_INSERT_TAIL(headp, vp, link);
}

/* Non-macro yasm_vps_first() for non-YASM_LIB_INTERNAL users. */
#undef yasm_vps_first
/*@null@*/ /*@dependent@*/ yasm_valparam *
yasm_vps_first(yasm_valparamhead *headp)
{
    return STAILQ_FIRST(headp);
}

/* Non-macro yasm_vps_next() for non-YASM_LIB_INTERNAL users. */
#undef yasm_vps_next
/*@null@*/ /*@dependent@*/ yasm_valparam *
yasm_vps_next(yasm_valparam *cur)
{
    return STAILQ_NEXT(cur, link);
}
