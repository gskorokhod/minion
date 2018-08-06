ConstraintDef constraint_list[] = {
{ "check[assign]", CT_CHECK_ASSIGN, 1, { {read_constraint} } },
{ "check[gsa]", CT_CHECK_GSA, 1, { {read_constraint} } },
{ "table", CT_WATCHED_TABLE, 2, { {read_list,read_tuples} } },
{ "negativetable", CT_WATCHED_NEGATIVE_TABLE, 2, { {read_list,read_tuples} } },
{ "abs", CT_ABS, 2, { {read_var,read_var} } },
{ "alldiffmatrix", CT_ALLDIFFMATRIX, 2, { {read_list,read_constant} } },
{ "false", CT_FALSE, 0, { {} } },
{ "true", CT_TRUE, 0, { {} } },
{ "difference", CT_DIFFERENCE, 2, { {read_2_vars,read_var} } },
{ "element", CT_ELEMENT, 3, { {read_list,read_var,read_var} } },
{ "element_one", CT_ELEMENT_ONE, 3, { {read_list,read_var,read_var} } },
{ "element_undefzero", CT_ELEMENT_UNDEFZERO, 3, { {read_list,read_var,read_var} } },
{ "diseq", CT_DISEQ, 2, { {read_var,read_var} } },
{ "eq", CT_EQ, 2, { {read_var,read_var} } },
{ "minuseq", CT_MINUSEQ, 2, { {read_var,read_var} } },
{ "__reify_diseq", CT_DISEQ_REIFY, 3, { {read_var,read_var,read_var} } },
{ "__reify_eq", CT_EQ_REIFY, 3, { {read_var,read_var,read_var} } },
{ "__reify_minuseq", CT_MINUSEQ_REIFY, 3, { {read_var,read_var,read_var} } },
{ "frameupdate", CT_FRAMEUPDATE, 5, { {read_list,read_list,read_list,read_list,read_constant} } },
{ "gacalldiff", CT_GACALLDIFF, 1, { {read_list} } },
{ "gaceq", CT_GACEQ, 2, { {read_var,read_var} } },
{ "lexleq[rv]", CT_GACLEXLEQ, 2, { {read_list,read_list} } },
{ "gacschema", CT_GACSCHEMA, 2, { {read_list,read_tuples} } },
{ "gcc", CT_GCC, 3, { {read_list,read_constant_list,read_list} } },
{ "gccweak", CT_GCCWEAK, 3, { {read_list,read_constant_list,read_list} } },
{ "haggisgac", CT_HAGGISGAC, 2, { {read_list,read_short_tuples} } },
{ "haggisgac-stable", CT_HAGGISGAC_STABLE, 2, { {read_list,read_short_tuples} } },
{ "ineq", CT_INEQ, 3, { {read_var,read_var,read_constant} } },
{ "lexleq", CT_LEXLEQ, 2, { {read_list,read_list} } },
{ "lexless", CT_LEXLESS, 2, { {read_list,read_list} } },
{ "lighttable", CT_LIGHTTABLE, 2, { {read_list,read_tuples} } },
{ "mddc", CT_MDDC, 2, { {read_list,read_tuples} } },
{ "negativemddc", CT_NEGATIVEMDDC, 2, { {read_list,read_tuples} } },
{ "min", CT_MIN, 2, { {read_list,read_var} } },
{ "max", CT_MAX, 2, { {read_list,read_var} } },
{ "alldiff", CT_ALLDIFF, 1, { {read_list} } },
{ "nvalueleq", CT_LEQNVALUE, 2, { {read_list,read_var} } },
{ "nvaluegeq", CT_GEQNVALUE, 2, { {read_list,read_var} } },
{ "occurrencegeq", CT_GEQ_OCCURRENCE, 3, { {read_list,read_constant,read_constant} } },
{ "occurrenceleq", CT_LEQ_OCCURRENCE, 3, { {read_list,read_constant,read_constant} } },
{ "occurrence", CT_OCCURRENCE, 3, { {read_list,read_constant,read_var} } },
{ "shortstr2", CT_SHORTSTR, 2, { {read_list,read_short_tuples} } },
{ "str2plus", CT_STR, 2, { {read_list,read_tuples} } },
{ "shortctuplestr2", CT_SHORTSTR_CTUPLE, 2, { {read_list,read_short_tuples} } },
{ "watchelement", CT_WATCHED_ELEMENT, 3, { {read_list,read_var,read_var} } },
{ "watchelement_one", CT_WATCHED_ELEMENT_ONE, 3, { {read_list,read_var,read_var} } },
{ "watchelement_undefzero", CT_WATCHED_ELEMENT_UNDEFZERO, 3, { {read_list,read_var,read_var} } },
{ "watchelement_one_undefzero", CT_WATCHED_ELEMENT_ONE_UNDEFZERO, 3, { {read_list,read_var,read_var} } },
{ "watchless", CT_WATCHED_LESS, 2, { {read_var,read_var} } },
{ "litsumgeq", CT_WATCHED_LITSUM, 3, { {read_list,read_constant_list,read_constant} } },
{ "watchneq", CT_WATCHED_NEQ, 2, { {read_var,read_var} } },
{ "watched-and", CT_WATCHED_NEW_AND, 1, { {read_constraint_list} } },
{ "watched-or", CT_WATCHED_NEW_OR, 1, { {read_constraint_list} } },
{ "lexleq[quick]", CT_QUICK_LEXLEQ, 2, { {read_list,read_list} } },
{ "lexless[quick]", CT_QUICK_LEXLESS, 2, { {read_list,read_list} } },
{ "watchsumgeq", CT_WATCHED_GEQSUM, 2, { {read_list,read_constant} } },
{ "watchsumleq", CT_WATCHED_LEQSUM, 2, { {read_list,read_constant} } },
{ "hamming", CT_WATCHED_HAMMING, 3, { {read_list,read_list,read_constant} } },
{ "not-hamming", CT_WATCHED_NOT_HAMMING, 3, { {read_list,read_list,read_constant} } },
{ "watchvecexists_less", CT_WATCHED_VEC_OR_LESS, 2, { {read_list,read_list} } },
{ "watchvecneq", CT_WATCHED_VECNEQ, 2, { {read_list,read_list} } },
{ "forwardchecking", CT_FORWARD_CHECKING, 1, { {read_constraint} } },
{ "reify", CT_REIFY, 2, { {read_constraint,read_var} } },
{ "reifyimply", CT_REIFYIMPLY, 2, { {read_constraint,read_var} } },
{ "reifyimply-quick", CT_REIFYIMPLY_QUICK, 2, { {read_constraint,read_var} } },
{ "div", CT_DIV, 2, { {read_2_vars,read_var} } },
{ "div_undefzero", CT_DIV_UNDEFZERO, 2, { {read_2_vars,read_var} } },
{ "modulo", CT_MODULO, 2, { {read_2_vars,read_var} } },
{ "modulo_undefzero", CT_MODULO_UNDEFZERO, 2, { {read_2_vars,read_var} } },
{ "pow", CT_POW, 2, { {read_2_vars,read_var} } },
{ "product", CT_PRODUCT2, 2, { {read_2_vars,read_var} } },
{ "sumleq", CT_LEQSUM, 2, { {read_list,read_var} } },
{ "sumgeq", CT_GEQSUM, 2, { {read_list,read_var} } },
{ "weightedsumgeq", CT_WEIGHTGEQSUM, 3, { {read_constant_list,read_list,read_var} } },
{ "weightedsumleq", CT_WEIGHTLEQSUM, 3, { {read_constant_list,read_list,read_var} } },
{ "w-inintervalset", CT_WATCHED_ININTERVALSET, 2, { {read_var,read_constant_list} } },
{ "w-inrange", CT_WATCHED_INRANGE, 2, { {read_var,read_constant_list} } },
{ "w-inset", CT_WATCHED_INSET, 2, { {read_var,read_constant_list} } },
{ "w-literal", CT_WATCHED_LIT, 2, { {read_var,read_constant} } },
{ "w-notinrange", CT_WATCHED_NOT_INRANGE, 2, { {read_var,read_constant_list} } },
{ "w-notinset", CT_WATCHED_NOT_INSET, 2, { {read_var,read_constant_list} } },
{ "w-notliteral", CT_WATCHED_NOTLIT, 2, { {read_var,read_constant} } },
};
