#pragma once

#ifdef DEBUG_MODE
#define DBG_EXPR(expr) expr
#else
#define DBG_EXPR(str)
#endif