#pragma once

//#define DEBUG_MODE

#ifdef DEBUG_MODE
#define DBG_EXPR(expr) expr
#else
#define DBG_EXPR(str)
#endif