/*
 * Kernel assert debug tool
 *
 * (c) Jimmy Larsson 1999
 *
 */

#ifndef ASSERT_H
#define ASSERT_H

/**
 * To have asserts compile with -DASSERT
 *
 */

#ifdef ASSERT

void assert_failed (char *exp, char *file, char *baseFile, int line);

#define assert(exp)  if (exp) ; \
        else assert_failed ( #exp, __FILE__, __BASE_FILE__, __LINE__ )

#else

#define assert(exp)

#endif



#endif
